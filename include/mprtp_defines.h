#ifndef MPRTP_MPRT_DEFINES_H
#define MPRTP_MPRT_DEFINES_H

#include <vector>

#define RTP_VERSION                       2
#define RTP_MAXCSRCS                      15
#define RTP_MINPACKETSIZE                 600
#define RTP_DEFAULTPACKETSIZE             1400
#define RTP_PROBATIONCOUNT                2
#define RTP_MAXPRIVITEMS                  256
#define RTP_SENDERTIMEOUTMULTIPLIER       2
#define RTP_BYETIMEOUTMULTIPLIER          1
#define RTP_MEMBERTIMEOUTMULTIPLIER       5
#define RTP_COLLISIONTIMEOUTMULTIPLIER    10
#define RTP_NOTETTIMEOUTMULTIPLIER        25
#define RTP_DEFAULTSESSIONBANDWIDTH       10000.0

#define RTP_RTCPTYPE_SR                   200
#define RTP_RTCPTYPE_RR                   201
#define RTP_RTCPTYPE_SDES                 202
#define RTP_RTCPTYPE_BYE                  203
#define RTP_RTCPTYPE_APP                  204
#define RTP_RTCPTYPE_MPRTP                211

#define RTCP_SDES_ID_CNAME                1
#define RTCP_SDES_ID_NAME                 2
#define RTCP_SDES_ID_EMAIL                3
#define RTCP_SDES_ID_PHONE                4
#define RTCP_SDES_ID_LOCATION             5
#define RTCP_SDES_ID_TOOL                 6
#define RTCP_SDES_ID_NOTE                 7
#define RTCP_SDES_ID_PRIVATE              8
#define RTCP_SDES_NUMITEMS_NONPRIVATE     7
#define RTCP_SDES_MAXITEMLENGTH           255

#define RTCP_BYE_MAXREASONLENGTH          255
#define RTCP_DEFAULTMININTERVAL           5.0
#define RTCP_DEFAULTBANDWIDTHFRACTION     0.05
#define RTCP_DEFAULTSENDERFRACTION        0.25
#define RTCP_DEFAULTHALFATSTARTUP         true
#define RTCP_DEFAULTIMMEDIATEBYE          true
#define RTCP_DEFAULTSRBYE                 true

#define MPRTP_EXTENSION_ID                0xBEDE

#define MPRTCP_TYPE_SUBFLOW               0
#define MPRTCP_TYPE_IP4_IFACE_ADVERT      1
#define MPRTCP_TYPE_IP6_IFACE_ADVERT      2
#define MPRTCP_TYPE_DNS_IFACE_ADVERT      3

#endif //MPRTP_MPRT_DEFINES_H
