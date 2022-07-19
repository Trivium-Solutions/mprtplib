#include "mprtp_rr_packet.h"

namespace mprtplib
{
    bool rtcp_rr_packet::parse(const uint8_t* data)
    {
        rtcp_receiver_report* hdr = (rtcp_receiver_report*) data;

        m_ssrc = ntohl(hdr->ssrc);
        m_fractionlost = hdr->fractionlost;
        m_packetslost = ((uint32_t)hdr->packetslost[2])|(((uint32_t)hdr->packetslost[1])<<8)|(((uint32_t)hdr->packetslost[0])<<16);
        m_exthighseqnr = ntohl(hdr->exthighseqnr);
        m_jitter = ntohl(hdr->jitter);
        m_lsr = ntohl(hdr->lsr);
        m_dlsr = ntohl(hdr->dlsr);

        return true;
    }

    void rtcp_rr_packet::print()
    {
        printf("ssrc:%X flost:%u lost:%u hiseq:%u jit:%u lsr:%u, dls:%u\n",
               m_ssrc, m_fractionlost, m_packetslost, m_exthighseqnr, m_jitter, m_lsr, m_dlsr);
    }
}