#include "mprtp_sr_packet.h"
#include "mprtp_defines.h"

namespace mprtplib
{
    bool rtcp_sr_packet::parse(const uint8_t* data)
    {
        rtcp_common_header* hdr = (rtcp_common_header*) data;
        data += sizeof(rtcp_common_header);

        if(hdr->packettype != RTP_RTCPTYPE_SR)
            return false;

        rtp_source_identifier* ssrc = (rtp_source_identifier*) data;
        data += sizeof(rtp_source_identifier);
        m_sender_ssrc = ntohl(ssrc->ssrc);

        rtcp_sender_report* sr = (rtcp_sender_report*) data;
        data += sizeof(rtcp_sender_report);
        m_ntp_time = rtp_ntp_time(ntohl(sr->ntptime_msw), ntohl(sr->ntptime_lsw));
        m_rtp_timestamp = ntohl(sr->rtptimestamp);
        m_packet_count = ntohl(sr->packetcount);
        m_octet_count = ntohl(sr->octetcount);

        for(int i = 0; i < hdr->count; i++)
        {
            auto rr_pack = std::make_shared<rtcp_rr_packet>(data, m_recv_from, m_recv_time);
            if(rr_pack->is_loaded())
            {
                m_rr_reports.emplace_back(std::move(rr_pack));
            }
            data += sizeof(rtcp_receiver_report);
        }

        return true;
    }

    void rtcp_sr_packet::print()
    {
        printf("Sender Report:\n");
        printf("    sender ssrc: %X\n", m_sender_ssrc);
        printf("  rtp timestamp: %u\n", m_rtp_timestamp);
        printf("   packet count: %u\n", m_packet_count);
        printf("    octet count: %u\n", m_octet_count);
        printf("        ntp msw: %u\n", m_ntp_time.get_msw());
        printf("        ntp lsw: %u\n", m_ntp_time.get_lsw());
    }
}