#ifndef MPRTP_MPRTP_STAT_H
#define MPRTP_MPRTP_STAT_H

#include <cstdint>
#include "mprtp_time.h"
#include "mprtp_packet.h"
#include "mprtp_struct.h"
#include "mprtp_sr_packet.h"

namespace mprtplib
{
    class source_stats
    {
    public:
        source_stats():
                m_received_data(false),
                m_packets_received(0),
                m_num_cycles(0), // shifted left 16 bits
                m_baseseqnr(0),
                m_exthighseqnr(0),
                m_prev_ext_high_seqnr(0),
                m_jitter(0),
                m_prev_timestamp(0),
                m_djitter(0),
                m_prev_packtime(0, 0),
                m_last_msg_time(0, 0),
                m_last_rtp_time(0, 0),
                m_last_note_time(0, 0),
                m_num_new_packets(0),
                m_saved_ext_seqnr(0)
        {}


        void process_packet(rtp_packet* pack, bool subflow, double tsunit);

        bool has_sent_data() const						            { return m_received_data; }
        uint32_t get_num_packets_received() const					{ return m_packets_received; }
        uint32_t get_base_sequence_number() const					{ return m_baseseqnr; }
        uint32_t get_extended_highest_sequence_number() const		{ return m_exthighseqnr; }
        uint32_t get_jitter() const						            { return m_jitter; }

        uint32_t get_num_packets_received_in_interval() const		{ return m_num_new_packets; }
        uint32_t get_saved_extended_sequence_number() const			{ return m_saved_ext_seqnr; }
        void start_new_interval()							        { m_num_new_packets = 0; m_saved_ext_seqnr = m_exthighseqnr; }

        void set_last_message_time(const rtp_time &t)				{ m_last_msg_time = t; }
        rtp_time get_last_message_time() const					    { return m_last_msg_time; }
        void set_last_rtp_packet_time(const rtp_time &t)				{ m_last_rtp_time = t; }
        rtp_time get_last_rtp_packet_time() const					{ return m_last_rtp_time; }

        void set_last_note_time(const rtp_time &t)					{ m_last_note_time = t; }
        rtp_time get_last_note_time() const						    { return m_last_note_time; }

        void fill_receiver_report(rtcp_receiver_report* rr, uint32_t ssrc, rtcp_sr_packet::ptr& sr, const rtp_time& currtime)
        {
            rtp_time curtime = rtp_time::now();
            uint32_t num = get_num_packets_received_in_interval();
            uint32_t prevseq = get_saved_extended_sequence_number();
            uint32_t curseq = get_extended_highest_sequence_number();
            uint32_t expected = curseq - prevseq;
            uint8_t fraclost;

            if (expected < num)
            {
                fraclost = 0;
            }
            else
            {
                double lost = (double)(expected-num);
                double frac = lost/((double)expected);
                fraclost = (uint8_t)(frac*256.0);
            }

            expected = curseq - get_base_sequence_number();
            num = get_num_packets_received();

            uint32_t packlost = expected - num;

            uint32_t jitter = get_jitter();
            uint32_t lsr;
            uint32_t dlsr;

            if (!sr)
            {
                lsr = 0;
                dlsr = 0;
            }
            else
            {
                rtp_ntp_time srtime = sr->get_ntp_time();
                uint32_t msw = (srtime.get_msw() & 0xFFFF);
                uint32_t lsw = ((srtime.get_lsw() >> 16) & 0xFFFF);
                lsr = ((msw << 16) | lsw);

                rtp_time diff = curtime;
                diff -= sr->get_recv_time();
                double diff2 = diff.get_double();
                diff2 *= 65536.0;
                dlsr = (uint32_t)diff2;
            }

            rr->ssrc = htonl(ssrc);
            rr->fractionlost = fraclost;
            rr->packetslost[2] = (uint8_t)(packlost & 0xFF);
            rr->packetslost[1] = (uint8_t)((packlost >> 8) & 0xFF);
            rr->packetslost[0] = (uint8_t)((packlost >> 16) & 0xFF);
            rr->exthighseqnr = htonl(curseq);
            rr->jitter = htonl(jitter);
            rr->lsr = htonl(lsr);
            rr->dlsr = htonl(dlsr);
            start_new_interval();
        }

    private:
        bool        m_received_data;
        uint32_t    m_packets_received;
        uint32_t    m_num_cycles; // shifted left 16 bits
        uint32_t    m_baseseqnr;
        uint32_t    m_exthighseqnr;
        uint32_t    m_prev_ext_high_seqnr;
        uint32_t    m_jitter;
        uint32_t    m_prev_timestamp;
        double      m_djitter;
        rtp_time    m_prev_packtime;
        rtp_time    m_last_msg_time;
        rtp_time    m_last_rtp_time;
        rtp_time    m_last_note_time;
        uint32_t    m_num_new_packets;
        uint32_t    m_saved_ext_seqnr;
    };
}

#endif //MPRTP_MPRTP_STAT_H
