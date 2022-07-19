#include "mprtp_source.h"
#include "mprtp_compound_packet.h"

namespace mprtplib
{
    subflow::ptr source::find_subflow(uint16_t id)
    {
        for(auto& subfl : m_subflows)
        {
            if(subfl->get_id() == id)
            {
                return subfl;
            }
        }
        return nullptr;
    }

    subflow::ptr source::find_subflow(uint16_t id, const endpoint& addr, interface::shared_ptr& iface)
    {
        auto subfl = find_subflow(id);
        if(subfl == nullptr)
        {
            subfl = std::make_shared<subflow>(id, iface, addr);
            m_subflows.push_back(subfl);
        }
        return subfl;
    }

    void source::process_packet(rtp_packet* pack, double tsunit, interface::shared_ptr& iface)
    {
        m_stat.process_packet(pack, false, tsunit);
        auto  subfl = find_subflow(pack->get_subflow_id(), pack->get_from_address(), iface);
        subfl->process_rtp_packet(pack, tsunit);
        if(m_last_receiver_report_time.is_zero())
        {
            m_last_receiver_report_time = rtp_time::now();
        }
    }

    void source::process_sr_packet(rtcp_sr_packet::ptr& pkt, uint16_t subflow_id, interface::shared_ptr& iface)
    {
        if(!subflow_id)
        {
            m_last_sender_report = pkt;
            m_send_rr = true;
        } else
        {
            auto  subfl = find_subflow(subflow_id, pkt->get_recv_from(), iface);
            subfl->process_sr_packet(pkt);
        }
    }

    void source::create_rr_packet(compound_packet &comp_pkt, const rtp_time& curtime)
    {
        uint32_t pkt_len =  sizeof(rtcp_common_header) +
                            sizeof(rtp_source_identifier) +
                            sizeof(rtcp_receiver_report);
        std::unique_ptr<uint8_t> pkt(new uint8_t[pkt_len]);
        memset(pkt.get(), 0, pkt_len);

        make_rr_packet(pkt.get(), &m_stat, curtime);

        comp_pkt.add_packet(pkt.get(), pkt_len);
    }

    uint8_t* source::make_rr_packet(uint8_t* data, source_stats* stat, const rtp_time& curtime)
    {
        uint32_t pkt_len =  sizeof(rtcp_common_header) +
                            sizeof(rtp_source_identifier) +
                            sizeof(rtcp_receiver_report);

        rtcp_common_header* hdr = (rtcp_common_header*) data;
        data += sizeof(rtcp_common_header);

        hdr->version = RTP_VERSION;
        hdr->packettype = RTP_RTCPTYPE_RR;
        hdr->count = 1;
        hdr->length = htons((uint16_t) (pkt_len / 4 - 1));

        rtp_source_identifier* ssrc = (rtp_source_identifier*) data;
        data += sizeof(rtp_source_identifier);
        ssrc->ssrc = htonl(m_sender_ssrc);

        rtcp_receiver_report* rr = (rtcp_receiver_report*) data;
        data += sizeof(rtcp_receiver_report);

        stat->fill_receiver_report(rr, m_ssrc, m_last_sender_report, curtime);
        return data;
    }

    void source::create_subflows_rr_packet(compound_packet &comp_pkt, const rtp_time& curtime)
    {
        std::unique_ptr<uint8_t> pkt(new uint8_t[MAX_PACKET_SIZE]);
        uint8_t* data = pkt.get();
        memset(data, 0, MAX_PACKET_SIZE);

        rtcp_common_header* pkt_hdr = (rtcp_common_header*) data;
        data += sizeof(rtcp_common_header);
        pkt_hdr->version = RTP_VERSION;
        pkt_hdr->packettype = RTP_RTCPTYPE_MPRTP;

        rtp_source_identifier* ssrc = (rtp_source_identifier*) data;
        data += sizeof(rtp_source_identifier);
        ssrc->ssrc = htonl(m_sender_ssrc);

        ssrc = (rtp_source_identifier*) data;
        data += sizeof(rtp_source_identifier);
        ssrc->ssrc = htonl(m_ssrc);

        for(auto& sf : m_subflows)
        {
            uint8_t *mprtp_start = data;
            mprtp_rtcp *mprtcp_hdr = (mprtp_rtcp *) data;
            data += sizeof(mprtp_rtcp);

            uint16_t* sf_id = (uint16_t*) data;
            *sf_id = htons(sf->get_id());
            data += sizeof(uint16_t);

            data = make_rr_packet(data, (source_stats*) &(sf->get_recv_stat()), curtime);
            mprtcp_hdr->block_length = (uint8_t) ((data - mprtp_start) / 4);
        }
        pkt_hdr->length = htons((uint16_t) ((data - pkt.get()) / 4 - 1));

        comp_pkt.add_packet(pkt.get(), (uint32_t) (data - pkt.get()));
    }

    subflow::ptr source::select_subflow()
    {
        return m_subflows.front();
    }

    void source::poll()
    {
        if(m_send_rr)
        {
            rtp_time curtime = rtp_time::now();
            m_send_rr = false;
            compound_packet comp_pkt;
            create_rr_packet(comp_pkt, curtime);
            create_subflows_rr_packet(comp_pkt, curtime);
            if(comp_pkt.get_length())
            {
                auto sf = select_subflow();
                sf->sendto(comp_pkt.get_data(), comp_pkt.get_length(), 0, true);
            }
        }
    }
}
