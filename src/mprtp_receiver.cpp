#include "mprtp_receiver.h"

namespace mprtplib
{
    bool receiver::on_rtp_packet(rtp_packet* pkt, interface::shared_ptr& iface)
    {
        auto src = get_source(pkt->get_ssrc());
        src->process_packet(pkt, m_timestamp_unit, iface);
        return true;
    }

    bool receiver::on_rtcp_sr_packet(rtcp_sr_packet::ptr& pkt, uint16_t subflow_id, interface::shared_ptr& iface)
    {
        auto src = get_source(pkt->get_sender_ssrc());
        src->process_sr_packet(pkt, subflow_id, iface);
#ifdef MPRTP_DEBUG
        if(!subflow_id)
        {
            printf("go SR\n");
            pkt->print();
        } else
        {
            printf("go SR subflow %d\n", (int) subflow_id);
            pkt->print();
        }
#endif
        return true;
    }

    source::ptr receiver::get_source(uint32_t ssrc)
    {
        auto src_it = m_sources.find(ssrc);
        if(src_it == m_sources.end())
        {
            auto src = std::make_shared<source>(ssrc, m_ssrc);
            m_sources[ssrc] = src;
            return src;
        }
        return src_it->second;
    }

    int receiver::poll()
    {
        int ret = session::poll();

        for(auto& src : m_sources)
        {
            src.second->poll();
        }

        return ret;
    }
}
