#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#ifndef MPRTP_MPRTP_SESSION_H
#define MPRTP_MPRTP_SESSION_H

#include <cstdint>
#include <memory>
#include <vector>
#include "mprtp_interface.h"
#include "mprtp_sr_packet.h"
#include "mprtp_rr_packet.h"

namespace mprtplib
{
    class session
    {
    protected:
        std::vector<interface::shared_ptr>  m_interfaces;
        std::list<rtp_packet::ptr>          m_packets;
    public:
        session() {}
        virtual ~session() {}

        bool add_interface(const endpoint& addr)
        {
            auto iface = std::make_shared<interface>(addr);
            if(iface->open())
            {
                m_interfaces.push_back(iface);
                return true;
            }
            return false;
        }
        bool suspend_interface(const endpoint& addr, bool suspend)
        {
            for(auto& iface : m_interfaces)
            {
                if(iface->is_same(addr))
                {
                    iface->suspend(suspend);
                    return true;
                }
            }
            return false;
        }

        virtual int poll();

        rtp_packet::ptr fetch_packet()
        {
            if(m_packets.empty())
            {
                return nullptr;
            }
            auto pkt = std::move(m_packets.front());
            m_packets.pop_front();
            return std::move(pkt);
        }

        virtual bool on_rtp_packet(rtp_packet* pkt, interface::shared_ptr& iface);
        virtual bool on_rtcp_sr_packet(rtcp_sr_packet::ptr& pkt, uint16_t subflow_id, interface::shared_ptr& iface);
        virtual bool on_rtcp_rr_packet(rtcp_rr_packet::ptr& pkt, uint16_t subflow_id, uint32_t sender_ssrc, interface::shared_ptr& iface);
//        virtual bool on_rtcp_sdes_packet(rtcp_sr_packet::ptr& pkt, interface::shared_ptr& iface);
//        virtual bool on_rtcp_bye_packet(rtcp_sr_packet::ptr& pkt, interface::shared_ptr& iface);
//        virtual bool on_rtcp_app_packet(rtcp_sr_packet::ptr& pkt, interface::shared_ptr& iface);
//        virtual bool on_rtcp_mprtcp_packet(rtcp_sr_packet::ptr& pkt, interface::shared_ptr& iface);
//        virtual bool on_rtcp_unknown_packet(rtcp_sr_packet::ptr& pkt, interface::shared_ptr& iface);

    private:
        void process_rtcp_packet(raw_packet::unique_ptr& pkt, interface::shared_ptr& iface);
        void process_rtp_packet(raw_packet::unique_ptr& pkt, interface::shared_ptr& iface);

        void process_sr_packet(const uint8_t* pkt, uint16_t subflow_id, const endpoint& recv_from, const rtp_time& recvtime, interface::shared_ptr& iface);
        void process_rr_packet(const uint8_t* pkt, uint16_t subflow_id, const endpoint& recv_from, const rtp_time& recvtime, interface::shared_ptr& iface);
        void process_sdes_packet(const uint8_t* pkt, const endpoint& recv_from, const rtp_time& recvtime, interface::shared_ptr& iface);
        void process_bye_packet(const uint8_t* pkt, const endpoint& recv_from, const rtp_time& recvtime, interface::shared_ptr& iface);
        void process_app_packet(const uint8_t* pkt, const endpoint& recv_from, const rtp_time& recvtime, interface::shared_ptr& iface);
        void process_mprtcp_packet(const uint8_t* pkt, const endpoint& recv_from, const rtp_time& recvtime, interface::shared_ptr& iface);
        void process_unknown_packet(const uint8_t* pkt, const endpoint& recv_from, const rtp_time& recvtime, interface::shared_ptr& iface);
    };
}

#endif //MPRTP_MPRTP_SESSION_H

#pragma clang diagnostic pop