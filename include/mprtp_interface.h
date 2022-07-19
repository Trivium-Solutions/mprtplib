#ifndef MPRTP_MPRTP_INTERFACE_H
#define MPRTP_MPRTP_INTERFACE_H

#include <unistd.h>
#include <memory>
#include <list>
#include "mprtp_packet.h"
#include "mprtp_defines.h"
#include "mprtp_address.h"

namespace mprtplib
{
    const int MAX_PACKET_SIZE = 65535;

    class interface
    {
    public:
        typedef std::unique_ptr<interface>  unique_ptr;
        typedef std::shared_ptr<interface>  shared_ptr;
    private:
        uint32_t                            m_ip;
        uint16_t                            m_rtp_port;
        uint16_t                            m_rtcp_port;
        int                                 m_rtp_sock;
        int                                 m_rtcp_sock;
        bool                                m_suspended;
        std::list<raw_packet::unique_ptr>   m_packets;
    public:
        interface(const endpoint& ep) :
                m_ip(std::get<0>(ep)),
                m_rtp_port(std::get<1>(ep)),
                m_rtcp_port(std::get<1>(ep) + (uint16_t) 1),
                m_rtcp_sock(-1),
                m_rtp_sock(-1),
                m_suspended(false)
        {}

        ~interface()
        {
            if(m_rtp_sock >= 0)
            {
                close(m_rtp_sock);
            }
            if(m_rtcp_sock >= 0)
            {
                close(m_rtcp_sock);
            }
        }

        endpoint get_address()
        {
            return std::make_tuple(m_ip, m_rtp_port);
        }

        void suspend(bool val) { m_suspended = val; }
        bool is_same(const endpoint& addr)
        {
            return (addr == get_address());
        }

        bool open();

        ssize_t sendto(const uint8_t* data, size_t len, const endpoint& addr, bool rtcp = false);

        int poll(bool rtcp = false);

        raw_packet::unique_ptr fetch_raw_packet()
        {
            if(!m_packets.empty())
            {
                raw_packet::unique_ptr pkt = std::move(m_packets.front());
                m_packets.pop_front();
                return std::move(pkt);
            }
            return nullptr;
        }
    };
}

#endif //MPRTP_MPRTP_INTERFACE_H
