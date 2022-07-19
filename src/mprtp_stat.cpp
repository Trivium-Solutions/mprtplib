#include "mprtp_stat.h"

namespace mprtplib
{
    void source_stats::process_packet(rtp_packet* pack, bool subflow, double tsunit)
    {
        uint32_t pack_sequence_number = subflow ? pack->get_subflow_extended_sequencenumber() : pack->get_extended_sequencenumber();

        if (!m_received_data) // This is the first received packet
        {
            m_received_data = true;
            m_packets_received++;
            m_num_new_packets++;

            if (pack_sequence_number == 0)
            {
                m_baseseqnr = 0x0000FFFF;
                m_num_cycles = 0x00010000;
            }
            else
            {
                m_baseseqnr = pack_sequence_number - 1;
            }

            m_exthighseqnr = m_baseseqnr + 1;
            m_prev_packtime = pack->recvtime();
            m_prev_ext_high_seqnr = m_baseseqnr;
            m_saved_ext_seqnr = m_baseseqnr;

            pack->set_extended_sequence_number(m_exthighseqnr);

            m_prev_timestamp = pack->get_timestamp();
            m_last_msg_time = m_prev_packtime;
            //if (!ownpacket) /* for own packet, this value is set on an outgoing packet */
            m_last_rtp_time = m_prev_packtime;
        }
        else // already got packets
        {
            uint16_t maxseq16;
            uint32_t extseqnr;

            // Adjust max extended sequence number and set extende seq nr of packet

            m_packets_received++;
            m_num_new_packets++;

            maxseq16 = (uint16_t)(m_exthighseqnr & 0x0000FFFF);
            if (pack_sequence_number >= maxseq16)
            {
                extseqnr = m_num_cycles + pack_sequence_number;
                m_exthighseqnr = extseqnr;
            }
            else
            {
                uint16_t dif1, dif2;

                dif1 = (uint16_t) pack_sequence_number;
                dif1 -= maxseq16;
                dif2 = maxseq16;
                dif2 -= (uint16_t) pack_sequence_number;
                if (dif1 < dif2)
                {
                    m_num_cycles += 0x00010000;
                    extseqnr = m_num_cycles + pack_sequence_number;
                    m_exthighseqnr = extseqnr;
                }
                else
                {
                    extseqnr = m_num_cycles + pack_sequence_number;
                }
            }

            if(subflow)
            {
                pack->set_subflow_extended_sequence_number(extseqnr);
            } else
            {
                pack->set_extended_sequence_number(extseqnr);
            }

            // Calculate jitter

            if (tsunit > 0)
            {
                rtp_time curtime = pack->recvtime();
                double ts1;
                double ts2;
                double diff;
                uint32_t curts = pack->get_timestamp();

                curtime -= m_prev_packtime;
                ts1 = curtime.get_double()/tsunit;

                if (curts > m_prev_timestamp)
                {
                    uint32_t unsigneddiff = curts - m_prev_timestamp;

                    if (unsigneddiff < 0x10000000) // okay, curts realy is larger than prevtimestamp
                        ts2 = (double)unsigneddiff;
                    else
                    {
                        // wraparound occurred and curts is actually smaller than prevtimestamp

                        unsigneddiff = -unsigneddiff; // to get the actual difference (in absolute value)
                        ts2 = -((double)unsigneddiff);
                    }
                }
                else if (curts < m_prev_timestamp)
                {
                    uint32_t unsigneddiff = m_prev_timestamp - curts;

                    if (unsigneddiff < 0x10000000)
                    {
                        // okay, curts really is smaller than prevtimestamp
                        ts2 = -((double)unsigneddiff); // negative since we actually need curts-prevtimestamp
                    } else
                    {
                        // wraparound occurred and curts is actually larger than prevtimestamp

                        unsigneddiff = -unsigneddiff; // to get the actual difference (in absolute value)
                        ts2 = (double)unsigneddiff;
                    }
                }
                else
                {
                    ts2 = 0;
                }

                diff = ts1 - ts2;
                if (diff < 0)
                {
                    diff = -diff;
                }
                diff -= m_djitter;
                diff /= 16.0;
                m_djitter += diff;
                m_jitter = (uint32_t)m_djitter;
            }
            else
            {
                m_djitter = 0;
                m_jitter = 0;
            }

            m_prev_packtime = pack->recvtime();
            m_prev_timestamp = pack->get_timestamp();
            m_last_msg_time = m_prev_packtime;
            m_last_rtp_time = m_prev_packtime;
        }
    }
}
