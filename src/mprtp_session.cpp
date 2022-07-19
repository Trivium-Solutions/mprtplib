#include "mprtp_session.h"

namespace mprtplib
{
    int session::poll()
    {
        // Poll data
        for(auto& iface : m_interfaces)
        {
            int status = iface->poll(false);
            if(status < 0)
            {
                return status;
            }
            status = iface->poll(true);
            if(status < 0)
            {
                return status;
            }
        }

        // Process received packets
        for(auto& iface : m_interfaces)
        {
            auto pkt = iface->fetch_raw_packet();
            while (pkt)
            {
                if(pkt->is_rtcp())
                {
                    process_rtcp_packet(pkt, iface);
                } else
                {
                    process_rtp_packet(pkt, iface);
                }
                pkt = iface->fetch_raw_packet();
            }
        }
        return 0;
    }

    void session::process_rtcp_packet(raw_packet::unique_ptr &pkt, interface::shared_ptr& iface)
    {
        const uint8_t* packet_data = pkt->data();
        const rtcp_common_header* rtcp_hdr = (const rtcp_common_header*) packet_data;
        while ((packet_data - pkt->data()) < pkt->data_len())
        {
            switch (rtcp_hdr->packettype)
            {
                case RTP_RTCPTYPE_SR:
                    process_sr_packet(packet_data, 0, pkt->get_from_address(), pkt->recvtime(), iface);
                    break;
                case RTP_RTCPTYPE_RR:
                    process_rr_packet(packet_data, 0, pkt->get_from_address(), pkt->recvtime(), iface);
                    break;
                case RTP_RTCPTYPE_SDES:
                    process_sdes_packet(packet_data, pkt->get_from_address(), pkt->recvtime(), iface);
                    break;
                case RTP_RTCPTYPE_BYE:
                    process_bye_packet(packet_data, pkt->get_from_address(), pkt->recvtime(), iface);
                    break;
                case RTP_RTCPTYPE_APP:
                    process_app_packet(packet_data, pkt->get_from_address(), pkt->recvtime(), iface);
                    break;
                case RTP_RTCPTYPE_MPRTP:
                    process_mprtcp_packet(packet_data, pkt->get_from_address(), pkt->recvtime(), iface);
                    break;
                default:
                    process_unknown_packet(packet_data, pkt->get_from_address(), pkt->recvtime(), iface);
                    break;
            }
            packet_data += (ntohs(rtcp_hdr->length) + 1) * 4;
            rtcp_hdr = (const rtcp_common_header*) packet_data;
        }
    }

    void session::process_rtp_packet(raw_packet::unique_ptr &pkt, interface::shared_ptr& iface)
    {
        rtp_packet::ptr rtppkt(new rtp_packet(pkt));

        if(!rtppkt->get_error())
        {
            if(on_rtp_packet(rtppkt.get(), iface))
            {
                m_packets.emplace_back(std::move(rtppkt));
            }
        }
    }

    bool session::on_rtp_packet(rtp_packet* pkt, interface::shared_ptr& iface)                  { return true; }
    bool session::on_rtcp_sr_packet(rtcp_sr_packet::ptr& pkt, uint16_t subflow_id, interface::shared_ptr& iface)     { return true; }
    bool session::on_rtcp_rr_packet(rtcp_rr_packet::ptr& pkt, uint16_t subflow_id, uint32_t sender_ssrc, interface::shared_ptr& iface)     { return true; }

    void session::process_sr_packet(const uint8_t* pkt, uint16_t subflow_id, const endpoint& recv_from, const rtp_time& recvtime, interface::shared_ptr& iface)
    {
        auto sr_pkt = std::make_shared<rtcp_sr_packet>(pkt, recv_from, recvtime);
        if(sr_pkt->is_loaded())
        {
            on_rtcp_sr_packet(sr_pkt, subflow_id, iface);
        }
    }

    void session::process_rr_packet(const uint8_t* pkt, uint16_t subflow_id, const endpoint& recv_from, const rtp_time& recvtime, interface::shared_ptr& iface)
    {
        const rtcp_common_header* rtcp_hdr = (const rtcp_common_header*) pkt;
        pkt += sizeof(rtcp_common_header);

        rtp_source_identifier* ssrc_hdr = (rtp_source_identifier*) pkt;
        pkt += sizeof(rtp_source_identifier);

        rtcp_rr_packet::ptr rr_pkt;
        for(int i = 0; i < rtcp_hdr->count; i++)
        {
            rr_pkt = std::make_shared<rtcp_rr_packet>(pkt, recv_from, recvtime);
            on_rtcp_rr_packet(rr_pkt, subflow_id, ntohl(ssrc_hdr->ssrc), iface);
        }
    }

    void session::process_sdes_packet(const uint8_t* pkt, const endpoint& recv_from, const rtp_time& recvtime, interface::shared_ptr& iface)
    {

    }

    void session::process_bye_packet(const uint8_t* pkt, const endpoint& recv_from, const rtp_time& recvtime, interface::shared_ptr& iface)
    {

    }

    void session::process_app_packet(const uint8_t* pkt, const endpoint& recv_from, const rtp_time& recvtime, interface::shared_ptr& iface)
    {

    }

    void session::process_mprtcp_packet(const uint8_t* pkt, const endpoint& recv_from, const rtp_time& recvtime, interface::shared_ptr& iface)
    {
        const uint8_t* data = pkt;
        const rtcp_common_header* hdr = (const rtcp_common_header*) data;
        data += sizeof(rtcp_common_header);

        size_t data_len = (size_t) ((ntohs(hdr->length) + 1) * 4);

        rtp_source_identifier* ssrc_hdr = (rtp_source_identifier*) data;
        data += sizeof(rtp_source_identifier);

        rtp_source_identifier* ssrc_1_hdr = (rtp_source_identifier*) data;
        data += sizeof(rtp_source_identifier);

        while (data - pkt < data_len)
        {
            const uint8_t* start_mprtp = data;
            mprtp_rtcp* mprtp_hdr = (mprtp_rtcp*) data;
            data += sizeof(mprtp_rtcp);

            switch(mprtp_hdr->mprtcp_type)
            {
                case MPRTCP_TYPE_SUBFLOW:
                    {
                        uint16_t* subflow_id = (uint16_t*) data;
                        data += sizeof(uint16_t);

                        const rtcp_common_header* mp_hdr = (const rtcp_common_header*) data;
                        switch (mp_hdr->packettype)
                        {
                            case RTP_RTCPTYPE_SR:
                                {
                                    process_sr_packet(data, ntohs(*subflow_id), recv_from, recvtime, iface);
                                }
                                break;
                            case RTP_RTCPTYPE_RR:
                                {
                                    process_rr_packet(data, ntohs(*subflow_id), recv_from, recvtime, iface);
                                }
                                break;
                            default:break;
                        }
                    }
                    break;
                case MPRTCP_TYPE_IP4_IFACE_ADVERT:
                    // TODO: Process Interface Advertisement (IPv4 Address)
                    break;
                case MPRTCP_TYPE_IP6_IFACE_ADVERT:
                    // TODO: Interface Advertisement (IPv4 Address)
                    break;
                case MPRTCP_TYPE_DNS_IFACE_ADVERT:
                    // TODO: Interface Advertisement (DNS Address)
                    break;
                default:break;
            }
            data = start_mprtp + (mprtp_hdr->block_length * 4);
        }
    }

    void session::process_unknown_packet(const uint8_t* pkt, const endpoint& recv_from, const rtp_time& recvtime, interface::shared_ptr& iface)
    {

    }
}
