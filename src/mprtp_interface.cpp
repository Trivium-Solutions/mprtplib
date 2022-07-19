#include <netinet/in.h>
#include <cstring>
#include <sys/ioctl.h>
#include "mprtp_interface.h"
#include "mprtp_select.h"

namespace mprtplib
{
    bool interface::open()
    {
        struct sockaddr_in si_me;

        m_rtp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        m_rtcp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        if(m_rtp_sock < 0 || m_rtcp_sock < 0)
            goto clean;

        // Bind RTP socket
        memset((char *) &si_me, 0, sizeof(si_me));
        si_me.sin_family = AF_INET;
        si_me.sin_port = htons(m_rtp_port);
        si_me.sin_addr.s_addr = htonl(m_ip);
        if (bind(m_rtp_sock, (struct sockaddr*) &si_me, sizeof(si_me)) == -1)
            goto clean;

        // Bind RTCP socket
        memset((char *) &si_me, 0, sizeof(si_me));
        si_me.sin_family = AF_INET;
        si_me.sin_port = htons(m_rtcp_port);
        si_me.sin_addr.s_addr = htonl(m_ip);
        if (bind(m_rtcp_sock, (struct sockaddr*) &si_me, sizeof(si_me)) == -1)
        {
            goto clean;
        }

        return true;
    clean:
        if(m_rtp_sock >= 0)
        {
            close(m_rtp_sock);
        }
        if(m_rtcp_sock >= 0)
        {
            close(m_rtcp_sock);
        }
        return false;
    }

    ssize_t interface::sendto(const uint8_t* data, size_t len, const endpoint& addr, bool rtcp)
    {
        struct sockaddr_in server;
        server.sin_addr.s_addr = htonl(std::get<0>(addr));
        server.sin_family      = AF_INET;
        if(rtcp)
        {
            server.sin_port        = htons(std::get<1>(addr) + (uint16_t) 1);
        } else
        {
            server.sin_port        = htons(std::get<1>(addr));
        }
        int sock = rtcp ? m_rtcp_sock : m_rtp_sock;
        return ::sendto(sock, data, len, MSG_NOSIGNAL, (struct sockaddr*) &server, sizeof(struct sockaddr_in));
    }

    int interface::poll(bool rtcp)
    {
        int sock = rtcp ? m_rtcp_sock : m_rtp_sock;
        size_t len;
        uint8_t  packet_buffer[MAX_PACKET_SIZE];

        if(sock < 0)
        {
            return -1;
        }

        bool dataavailable = false;
        struct sockaddr_in srcaddr;

        do
        {
            len = 0;
            ioctl(sock, FIONREAD, &len);
            if(len <= 0)
            {
                int8_t isset = 0;
                int status = rtp_select(&sock, &isset, 1);
                if (status < 0)
                    return status;

                if (isset)
                {
                    dataavailable = true;
                } else
                {
                    dataavailable = false;
                }
            } else
            {
                dataavailable = true;
            }

            if(dataavailable)
            {
                rtp_time recvtime = rtp_time::now();
                socklen_t fromlen = sizeof(struct sockaddr_in);
                ssize_t recvlen = recvfrom(sock, packet_buffer, MAX_PACKET_SIZE, 0, (struct sockaddr *)&srcaddr, &fromlen);
                if(recvlen > 0)
                {
                    auto * pkt = new raw_packet(packet_buffer,
                                                (size_t) recvlen,
                                                ntohl(srcaddr.sin_addr.s_addr),
                                                ntohs(srcaddr.sin_port), rtcp, recvtime);
                    m_packets.emplace_back(pkt);
                }
            }
        } while (dataavailable);

        return 0;
    }
}