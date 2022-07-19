#ifndef MPRTP_MPRTP_SR_PACKET_H
#define MPRTP_MPRTP_SR_PACKET_H

#include "mprtp_packet.h"
#include "mprtp_rr_packet.h"

namespace mprtplib
{
    class rtcp_sr_packet
    {
    public:
        typedef std::shared_ptr<rtcp_sr_packet> ptr;

        rtcp_sr_packet(const uint8_t* data, const endpoint& recv_from, const rtp_time& recvtime) :
                m_recv_time(recvtime),
                m_recv_from(recv_from),
                m_sender_ssrc(0),
                m_ntp_time(0, 0),
                m_rtp_timestamp(0),
                m_packet_count(0),
                m_octet_count(0)
        {
            m_is_loaded = parse(data);
        }

        bool is_loaded() const                                  { return m_is_loaded; }
        uint32_t get_sender_ssrc() const                        { return m_sender_ssrc; }
        uint32_t get_rtp_timestamp() const                      { return m_rtp_timestamp; }
        uint32_t get_packet_count() const                       { return m_packet_count; }
        uint32_t get_octet_count() const                        { return m_octet_count; }
        const rtp_time& get_recv_time() const                    { return m_recv_time; }
        const rtp_ntp_time& get_ntp_time() const                  { return m_ntp_time; }
        int get_report_blocks_count() const                     { return (int) m_rr_reports.size(); }
        rtcp_rr_packet::ptr get_report_block(int idx) const     { return m_rr_reports[idx]; }
        const endpoint& get_recv_from() const                   { return m_recv_from; }
        void print();

    private:
        bool parse(const uint8_t* data);

        bool                                m_is_loaded;
        rtp_time                             m_recv_time;
        endpoint                            m_recv_from;

        uint32_t                            m_sender_ssrc;
        rtp_ntp_time                          m_ntp_time;
        uint32_t                            m_rtp_timestamp;
        uint32_t                            m_packet_count;
        uint32_t                            m_octet_count;
        std::vector<rtcp_rr_packet::ptr>    m_rr_reports;
    };
}

#endif //MPRTP_MPRTP_SR_PACKET_H
