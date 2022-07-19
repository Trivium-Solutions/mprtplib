#ifndef MPRTP_MPRTP_RR_PACKET_H
#define MPRTP_MPRTP_RR_PACKET_H

#include <memory>
#include "mprtp_time.h"
#include "mprtp_struct.h"
#include "mprtp_address.h"

namespace mprtplib
{
    class rtcp_rr_packet
    {
    public:
        typedef std::shared_ptr<rtcp_rr_packet> ptr;

        /// data must point to report block
        rtcp_rr_packet(const uint8_t* data, const endpoint& recv_from, const rtp_time& recvtime) :
                m_recv_time(recvtime),
                m_recv_from(recv_from)
        {
            m_is_loaded = parse(data);
        }

        bool is_loaded() const              { return m_is_loaded; }

        uint32_t  get_ssrc() const          { return  m_ssrc; }
        uint8_t   get_fractionlost() const  { return  m_fractionlost; }
        uint32_t  get_packetslost() const   { return  m_packetslost; }
        uint32_t  get_exthighseqnr() const  { return  m_exthighseqnr; }
        uint32_t  get_jitter() const        { return  m_jitter; }
        uint32_t  get_lsr() const           { return  m_lsr; }
        uint32_t  get_dlsr() const          { return  m_dlsr; }

        const rtp_time&  get_recvtime() const  { return m_recv_time; }
        const endpoint& get_recv_from() const { return m_recv_from; }

        void print();

    private:
        bool                m_is_loaded;
        rtp_time             m_recv_time;
        endpoint            m_recv_from;

        uint32_t m_ssrc; // Identifies about which SSRC's data this report is...
        uint8_t  m_fractionlost;
        uint32_t m_packetslost;
        uint32_t m_exthighseqnr;
        uint32_t m_jitter;
        uint32_t m_lsr;
        uint32_t m_dlsr;

        bool parse(const uint8_t* data);

    };
}

#endif //MPRTP_MPRTP_RR_PACKET_H
