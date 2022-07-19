#ifndef MPRTP_MPRTP_ADDRESS_H
#define MPRTP_MPRTP_ADDRESS_H

#include <cstdint>
#include <tuple>
#include <vector>
#include <arpa/inet.h>
#include <string>

namespace mprtplib
{
    typedef std::tuple<uint32_t, uint16_t > endpoint;
    typedef std::vector<endpoint>           address;

    inline endpoint create_endpoint(const std::string& addr, uint16_t port)
    {
        uint32_t ip = ntohl(inet_addr(addr.c_str()));
        return std::make_tuple(ip, port);
    }

    inline uint32_t endpoint_ip(const endpoint& ep)
    {
        return std::get<0>(ep);
    }

    inline uint16_t endpoint_port(const endpoint& ep)
    {
        return std::get<1>(ep);
    }

    inline bool in_same_subnet(const endpoint &addr1, const endpoint &addr2, uint32_t prefix)
    {
        if((endpoint_ip(addr1) & (0xFFFFFFFF << (32 - prefix))) == (endpoint_ip(addr2) & (0xFFFFFFFF << (32 - prefix))))
        {
            return true;
        }
        return false;
    }
}

#endif //MPRTP_MPRTP_ADDRESS_H
