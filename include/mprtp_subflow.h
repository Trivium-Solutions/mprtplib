#ifndef MPRTP_MPRTP_SUBFLOW_H
#define MPRTP_MPRTP_SUBFLOW_H

#include <cstdint>
#include <tuple>
#include "mprtp_defines.h"
#include "mprtp_interface.h"
#include "mprtp_stat.h"
#include "mprtp_sr_packet.h"

namespace mprtplib
{
    class subflow
    {
    public:

        typedef std::shared_ptr<subflow>    ptr;

        subflow(uint16_t id, const interface::shared_ptr& iface, const endpoint& remote) :
                m_subflow_id(id),
                m_remote(remote),
                m_iface(iface),
                m_packet_count(0),
                m_octet_count(0),
                m_packet_count_tt(0),
                m_octet_count_tt(0),
                m_tested_bitrate(0),
                m_congestion_indicator(0),
                m_congested_bitrate(0),
                m_congestion_time(0, 0),
                m_percentage(0.5),
                m_ready_for_distribution(false),
                m_instant_bitrate(0),
                m_TB_initialized(false),
                m_CB_initialized(false)
        {
            m_sequesnce_number = (uint16_t) (rand() % 0xFFFF);
        }

        const endpoint& get_remote_addr() const         { return m_remote;     }
        uint16_t get_id() const                         { return m_subflow_id; }
        uint16_t get_sequesnce_number() const           { return m_sequesnce_number; }
        const interface::shared_ptr& get_iface() const  { return m_iface; }
        uint32_t get_packet_count() const               { return m_packet_count; }
        uint32_t get_octet_count() const                { return m_octet_count; }
        const source_stats& get_recv_stat() const       { return m_recv_stat; }

        ssize_t sendto(const uint8_t* data, size_t len, uint32_t payload_length, bool rtcp = false)
        {
            ssize_t ret = m_iface->sendto(data, len, m_remote, rtcp);
            if(!rtcp)
            {
                if (ret == len)
                {
                    m_packet_count++;
                    m_octet_count += payload_length;
                    m_packet_count_tt++;
                    m_octet_count_tt += payload_length;
                }
                m_sequesnce_number++;
            }
            return ret;
        }

        void process_rtp_packet(rtp_packet* pack, double tsunit)
        {
            m_recv_stat.process_packet(pack, true, tsunit);
        }

        void process_sr_packet(rtcp_sr_packet::ptr& pkt)
        {
            m_last_sender_report = pkt;
        }

        void process_rr_packet(rtcp_rr_packet::ptr& pkt);

        bool is_congested();
        bool is_lossy()
        {
            return (m_congestion_indicator > 0 && !is_congested());
        }

        double get_percentage() const               { return m_percentage; }
        void   set_percentage(double percentage)    { m_percentage = percentage; }

        double get_tested_bitrate() const           { return m_tested_bitrate; }
        double get_congested_bitrate() const        { return m_congested_bitrate; }

        bool is_ready_for_distribution() const      { return m_ready_for_distribution; }
        int get_CI() const { return  m_congestion_indicator; }
        double get_B() const { return  m_instant_bitrate; }

    private:
        uint16_t                m_sequesnce_number;
        uint16_t                m_subflow_id;
        endpoint                m_remote;
        interface::shared_ptr   m_iface;
        rtcp_sr_packet::ptr     m_last_sender_report;
        rtcp_rr_packet::ptr     m_last_receiver_report;

        // sender stat
        uint32_t                m_packet_count;
        uint32_t                m_octet_count;

        uint32_t                m_packet_count_tt;
        uint32_t                m_octet_count_tt;
        double                  m_instant_bitrate;          /// B
        double                  m_tested_bitrate;           /// TB
        bool                    m_TB_initialized;
        double                  m_congested_bitrate;        /// CB
        bool                    m_CB_initialized;
        int                     m_congestion_indicator;     /// CI
        rtp_time                 m_congestion_time;          /// Ctime
        double                  m_percentage;
        bool                    m_ready_for_distribution;

        // receiver stat
        source_stats            m_recv_stat;
    };
}

#endif //MPRTP_MPRTP_SUBFLOW_H
