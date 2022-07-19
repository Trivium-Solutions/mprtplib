#ifndef MPRTP_MPRTP_SOURCE_H
#define MPRTP_MPRTP_SOURCE_H

#include <cstdint>
#include <memory>
#include "mprtp_packet.h"
#include "mprtp_time.h"
#include "mprtp_subflow.h"
#include "mprtp_stat.h"
#include "mprtp_sr_packet.h"
#include "mprtp_compound_packet.h"

namespace mprtplib
{
    class source
    {
    public:
        typedef std::shared_ptr<source>     ptr;

        source(uint32_t ssrc, uint32_t sender_ssrc) :
                m_ssrc(ssrc),
                m_sender_ssrc(sender_ssrc),
                m_last_receiver_report_time(0, 0),
                m_send_rr(false)
        {}
        ~source() {}

        uint32_t get_ssrc() const           { return m_ssrc; }
        void poll();

        void process_packet(rtp_packet* pack, double tsunit, interface::shared_ptr& iface);
        void process_sr_packet(rtcp_sr_packet::ptr& pkt, uint16_t subflow_id, interface::shared_ptr& iface);

    private:
        uint32_t                    m_ssrc;
        uint32_t                    m_sender_ssrc;
        source_stats                m_stat;
        std::vector<subflow::ptr>   m_subflows;
        rtcp_sr_packet::ptr         m_last_sender_report;
        rtp_time                     m_last_receiver_report_time;
        bool                        m_send_rr;

        subflow::ptr find_subflow(uint16_t id, const endpoint& addr, interface::shared_ptr& iface);
        subflow::ptr find_subflow(uint16_t id);

        void create_rr_packet(compound_packet &comp_pkt, const rtp_time& curtime);
        void create_subflows_rr_packet(compound_packet &comp_pkt, const rtp_time& curtime);
        uint8_t* make_rr_packet(uint8_t* data, source_stats* stat, const rtp_time& curtime);
        subflow::ptr select_subflow();
    };
}

#endif //MPRTP_MPRTP_SOURCE_H
