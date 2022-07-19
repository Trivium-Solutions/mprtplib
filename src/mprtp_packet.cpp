#include <netinet/in.h>
#include "mprtp_packet.h"
#include "mprtp_struct.h"
#include "mprtp_defines.h"

namespace mprtplib
{
    int rtp_packet::parse_packet()
    {
        if(!m_rawpack)
        {
            return -1;
        }
        if(m_rawpack->is_rtcp())
        {
            return -2;
        }
        if(m_rawpack->data_len() < sizeof(rtp_header))
        {
            return -3;
        }

        int numpadbytes = 0;
        int payloadoffset = 0;
        int payloadlength = 0;
        const uint8_t* packet_bytes = m_rawpack->data();
        const rtp_header* rtpheader = (const rtp_header*) packet_bytes;

        if(rtpheader->version != 2)
        {
            return -4;
        }

        m_hasmarker = (rtpheader->marker == 0) ? false : true;
        m_payloadtype = rtpheader->payloadtype;
        if (m_hasmarker)
        {
            if (m_payloadtype == (RTP_RTCPTYPE_SR & 127))
            { // don't check high bit (this was the marker!!)
                return -5;
            }
            if (m_payloadtype == (RTP_RTCPTYPE_RR & 127))
            {
                return -5;
            }
        }

        m_numcsrcs = rtpheader->csrccount;
        payloadoffset = sizeof(rtp_header) + (int)(m_numcsrcs * sizeof(uint32_t));

        if (rtpheader->padding) // adjust payload length to take padding into account
        {
            numpadbytes = (int)packet_bytes[m_rawpack->data_len() - 1]; // last byte contains number of padding bytes
            if (numpadbytes <= 0)
            {
                return -6;
            }
        }
        m_hasextension = (rtpheader->extension == 0) ? false : true;
        if (m_hasextension) // got header extension
        {
            rtp_extension_header* rtpextheader = (rtp_extension_header *)(packet_bytes + payloadoffset);
            payloadoffset += sizeof(rtp_extension_header);

            uint16_t exthdrlen = ntohs(rtpextheader->length);
            payloadoffset += ((int)exthdrlen) * sizeof(uint32_t);

            m_extid = ntohs(rtpextheader->extid);
            m_extensionlength = ((int)ntohs(rtpextheader->length)) * sizeof(uint32_t);
            m_extension = ((uint8_t *)rtpextheader) + sizeof(rtp_extension_header);

            if(m_extid == 0xBEDE)
            {
                // This is MPRTP packet;
                mprtp_ex* mprtp_hdr = (mprtp_ex*) m_extension;

                m_subflow_extseqnr = ntohs(mprtp_hdr->subflow_seq);
                m_subflow_id = ntohs(mprtp_hdr->subflow_id);
            }
        }
        payloadlength = (int) m_rawpack->data_len() - numpadbytes - payloadoffset;
        if (payloadlength < 0)
        {
            return -7;
        }

        m_extseqnr = (uint32_t)ntohs(rtpheader->sequencenumber);
        m_timestamp = ntohl(rtpheader->timestamp);
        m_ssrc = ntohl(rtpheader->ssrc);
        m_payload = packet_bytes + payloadoffset;
        m_payloadlength = (size_t) payloadlength;

        return 0;
    }

    uint32_t rtp_packet::get_csrc(int num) const
    {
        const uint8_t *csrcpos;
        uint32_t *csrcval_nbo;
        uint32_t csrcval_hbo;

        csrcpos = m_rawpack->data() + sizeof(rtp_header) + num * sizeof(uint32_t);
        csrcval_nbo = (uint32_t *)csrcpos;
        csrcval_hbo = ntohl(*csrcval_nbo);
        return csrcval_hbo;
    }
}