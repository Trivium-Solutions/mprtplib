#ifndef MPRTP_MPRTP_COMPOUND_PACKET_H
#define MPRTP_MPRTP_COMPOUND_PACKET_H

#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace mprtplib
{
    class compound_packet
    {
    public:
        compound_packet() :
                m_data(nullptr),
                m_len(0)
        {}

        ~compound_packet()
        {
            if(m_data)
            {
                free(m_data);
            }
        }

        void add_packet(uint8_t* pack, uint32_t len)
        {
            if(!m_data)
            {
                m_data = (uint8_t*) malloc(len);
            } else
            {
                m_data = (uint8_t*) realloc(m_data, m_len + len);
            }
            memcpy(m_data + m_len, pack, len);
            m_len += len;
        }

        uint32_t get_length() const         { return m_len; }
        const uint8_t* get_data() const     { return m_data; }

    private:
        uint8_t*    m_data;
        uint32_t    m_len;
    };
}

#endif //MPRTP_MPRTP_COMPOUND_PACKET_H
