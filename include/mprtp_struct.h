#ifndef MPRTP_RTPSTRUCT_H
#define MPRTP_RTPSTRUCT_H

#include <cstdint>
#include <sys/param.h>

namespace mprtplib
{

#pragma pack(push, 1)

    /* RTP structures*/

    struct rtp_header
    {
#if __BYTE_ORDER == __BIG_ENDIAN
        uint8_t version:2;
	    uint8_t padding:1;
	    uint8_t extension:1;
	    uint8_t csrccount:4;

	    uint8_t marker:1;
	    uint8_t payloadtype:7;
#else // little endian
        uint8_t csrccount:4;
        uint8_t extension:1;
        uint8_t padding:1;
        uint8_t version:2;

        uint8_t payloadtype:7;
        uint8_t marker:1;
#endif // __BYTE_ORDER == __BIG_ENDIAN

        uint16_t sequencenumber;
        uint32_t timestamp;
        uint32_t ssrc;
    };

    struct rtcp_common_header
    {
#if __BYTE_ORDER == __BIG_ENDIAN
        uint8_t version:2;
	    uint8_t padding:1;
	    uint8_t count:5;
#else // little endian
        uint8_t count:5;
        uint8_t padding:1;
        uint8_t version:2;
#endif // __BYTE_ORDER == __BIG_ENDIAN

        uint8_t  packettype;
        uint16_t length;
    };

    struct rtp_extension_header
    {
        uint16_t extid;
        uint16_t length;
    };

    struct rtp_source_identifier
    {
        uint32_t ssrc;
    };

    /* RTCP structures*/

    struct rtcp_sender_report
    {
        uint32_t ntptime_msw;
        uint32_t ntptime_lsw;
        uint32_t rtptimestamp;
        uint32_t packetcount;
        uint32_t octetcount;
    };

    struct rtcp_receiver_report
    {
        uint32_t ssrc; // Identifies about which SSRC's data this report is...
        uint8_t  fractionlost;
        uint8_t  packetslost[3];
        uint32_t exthighseqnr;
        uint32_t jitter;
        uint32_t lsr;
        uint32_t dlsr;
    };

    /* MPRTP structures*/

    struct mprtp_rtcp
    {
        uint8_t  mprtcp_type;
        uint8_t  block_length;
    };

    struct mprtp_rtcp_subflow
    {
        uint8_t  mprtcp_type;
        uint8_t  block_length;
        uint16_t subflow_id;
    };

    struct mprtp_ex
    {
#if __BYTE_ORDER == __BIG_ENDIAN
        uint8_t ID:4;
        uint8_t len1:4;

        uint8_t pad1:4;
        uint8_t len2:4;

        uint16_t subflow_id;
        uint16_t flow_seq;
        uint16_t pad2;
#else // little endian
        uint8_t len1:4;
        uint8_t ID:4;

        uint8_t len2:4;
        uint8_t pad:4;

        uint16_t subflow_id;
        uint16_t subflow_seq;
        uint16_t pad2;
#endif // __BYTE_ORDER == __BIG_ENDIAN
    };

#pragma pack(pop)

}

#endif //MPRTP_RTPSTRUCT_H
