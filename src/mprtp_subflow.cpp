#include "mprtp_subflow.h"
#include "mprtp_constants.h"

namespace mprtplib
{
    void subflow::process_rr_packet(rtcp_rr_packet::ptr& pkt)
    {
        if(m_last_receiver_report && m_packet_count_tt > 0)
        {
            // Calculate instantaneous bitrate
            double fract_loss = (double) pkt->get_fractionlost() / 256.0;
            double avg_packet_size = (double) m_octet_count_tt / (double) m_packet_count_tt;
            double time_diff = pkt->get_recvtime().get_double() - m_last_receiver_report->get_recvtime().get_double();
            double hsn_diff = (double) (pkt->get_exthighseqnr() - m_last_receiver_report->get_exthighseqnr());
            m_instant_bitrate = hsn_diff * (1.0 - fract_loss) * avg_packet_size / time_diff;

            // Calculate tested and congested bitrates
            if(pkt->get_fractionlost() == 0)
            {
                if(!m_TB_initialized)
                {
                    // Initialize tested bitrate
                    m_TB_initialized = true;
                    m_tested_bitrate = m_instant_bitrate * alpha;
                }
                else if(m_tested_bitrate < m_instant_bitrate)
                {
                    m_tested_bitrate = m_tested_bitrate * alpha + (1.0 - alpha) * m_instant_bitrate;
                }
                if(m_congestion_indicator > 0)
                {
                    if((m_congestion_time.get_double() + m_congestion_indicator * TO) < rtp_time::now().get_double())
                    {
                        m_congestion_indicator--;
                        m_congestion_time = rtp_time::now();
                    }
                    if(!m_CB_initialized)
                    {
                        // initialize congested bitrate
                        m_CB_initialized = true;
                        m_congested_bitrate = m_instant_bitrate * alpha;
                    }
                    else  if(m_congested_bitrate < m_instant_bitrate)
                    {
                        m_congested_bitrate = m_congested_bitrate * alpha + (1.0 - alpha) * m_instant_bitrate;
                    }
                }
            } else
            {
                //m_tested_bitrate = m_tested_bitrate * alpha + (1.0 - alpha) * m_instant_bitrate;
                // Reset tested bitrate
                m_tested_bitrate = m_instant_bitrate * alpha;
                m_TB_initialized = true;

                m_congested_bitrate = m_instant_bitrate * alpha;
                m_CB_initialized = true;
                m_congestion_time = rtp_time::now();
                if(m_congestion_indicator < CIMAX)
                {
                    m_congestion_indicator++;
                }
            }
            m_ready_for_distribution = true;
        }
        m_packet_count_tt = 0;
        m_octet_count_tt = 0;
        m_last_receiver_report = pkt;
    }

    bool subflow::is_congested()
    {
        if(m_congestion_indicator == CIMAX)
        {
            return true;
        }
        return false;
    }

}