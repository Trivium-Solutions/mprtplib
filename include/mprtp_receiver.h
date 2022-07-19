#ifndef MPRTP_MPRTP_RECEIVER_H
#define MPRTP_MPRTP_RECEIVER_H

#include <map>
#include "mprtp_session.h"
#include "mprtp_source.h"

namespace mprtplib
{
    class receiver : public session
    {
    public:
        receiver(double timestamp_unit, uint32_t ssrc) :
                session(),
                m_timestamp_unit(timestamp_unit)
        {
            if(ssrc == 0)
            {
                m_ssrc = 0;
                while (!m_ssrc)
                {
                    m_ssrc = (uint32_t) rand();
                }
            } else
            {
                m_ssrc = ssrc;
            }
        }
        ~receiver() {}

        bool on_rtp_packet(rtp_packet* pkt, interface::shared_ptr& iface) override;
        bool on_rtcp_sr_packet(rtcp_sr_packet::ptr& pkt, uint16_t subflow_id, interface::shared_ptr& iface) override;

        int poll() override;
    private:
        source::ptr get_source(uint32_t ssrc);
        std::map<uint32_t, source::ptr>     m_sources;
        double                              m_timestamp_unit;
        uint32_t                            m_ssrc;
    };
}

#endif //MPRTP_MPRTP_RECEIVER_H
