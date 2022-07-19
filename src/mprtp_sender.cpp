#include <memory>
#include <cmath>
#include <algorithm>
#include "mprtp_sender.h"
#include "mprtp_constants.h"

#define MAXPACKSIZE     65535

namespace mprtplib
{
    sender::sender(double timestamp_unit) :
            session(),
            m_ssrc(0),
            m_next_subflow_id(1),
            m_next_subflow(0),
            m_timestamp(0),
            m_packet_count(0),
            m_octet_count(0),
            m_timestamp_unit(timestamp_unit),
            m_last_packet_time(0, 0),
            m_last_sender_report_time(0, 0),
            m_stream_bitrate(1),
            m_sb_size(0),
            m_sb_time(0, 0),
            m_recalc_distribution(false)
    {
        srand((unsigned int) time(nullptr));
        m_sequesnce_number = (uint16_t) (rand() % 0xFFFF);
    }

    int sender::poll()
    {
        int ret = session::poll();
        rtp_time curtime = rtp_time::now();

        if(!m_last_sender_report_time.is_zero())
        {
            if (curtime.get_seconds() - m_last_sender_report_time.get_seconds() >= 5)
            {
                send_sender_report();
            }
        }
        if(!m_sb_time.is_zero())
        {
            if(m_sb_time.get_double() + 1 <= curtime.get_double())
            {
                m_stream_bitrate = (double) m_sb_size / (curtime.get_double() - m_sb_time.get_double());
                m_sb_size = 0;
                m_sb_time = curtime;
            }
        } else
        {
            m_sb_time = curtime;
        }
        if(m_recalc_distribution)
        {
            recalc_subflow_percentage();
            m_recalc_distribution = false;
        }
        return ret;
    }

    bool sender::connect(uint32_t ssrc, const address& destination)
    {
        if(m_interfaces.empty())
        {
            return false;
        }

        m_ssrc = ssrc;
        m_dest_addr = destination;

        for(const auto& iface : m_interfaces)
        {
            for(const auto& addr : m_dest_addr)
            {
                if(is_path_valid(iface->get_address(), addr))
                {
                    auto sf = std::make_shared<subflow>(m_next_subflow_id++, iface, addr);
                    m_subflows.emplace_back(sf);
                }
            }
        }

        return !m_subflows.empty();
    }

    subflow::ptr sender::select_subflow()
    {
        double rnd = (double) (rand() % 101) / 100;
        double p = 0;
        for(auto& sf : m_subflows)
        {
            p += sf->get_percentage();
            if(p > rnd)
            {
                return sf;
            }
        }
        return m_subflows.front();
    }


    bool sender::send_data(const uint8_t* data, int data_len)
    {
        auto sf = select_subflow();
#ifdef MPRTP_DEBUG
        printf("Sending packet via subflow #%hu\n", sf->get_id());
#endif

        int pkt_len = data_len + sizeof(rtp_header) + sizeof(rtp_extension_header) + sizeof(mprtp_ex);
        std::unique_ptr<uint8_t> pkt(new uint8_t[pkt_len]);
        memset(pkt.get(), 0, (size_t) pkt_len);

        auto* hdr = (rtp_header*) pkt.get();
        auto* ext_hdr =  (rtp_extension_header*) (pkt.get() + sizeof(rtp_header));
        auto* mprtp = (mprtp_ex*) (pkt.get() + sizeof(rtp_header) + sizeof(rtp_extension_header));
        uint8_t* payload = pkt.get() + sizeof(rtp_header) + sizeof(rtp_extension_header) + sizeof(mprtp_ex);

        // Copy payload data
        memcpy(payload, data, (size_t) data_len);

        // Fill RTP header
        hdr->version    = RTP_VERSION;
        hdr->extension  = 1;
        hdr->sequencenumber = htons(m_sequesnce_number++);
        hdr->timestamp = htonl(get_next_timestamp());
        hdr->ssrc = htonl(m_ssrc);

        // Fill extension header
        ext_hdr->extid = htons(MPRTP_EXTENSION_ID);
        ext_hdr->length = htons(2);

        // Fill MPRTP extension data
        mprtp->len1 = mprtp->len2 = 4;
        mprtp->subflow_id = htons(sf->get_id());
        mprtp->subflow_seq = htons(sf->get_sequesnce_number());

        if(sf->sendto(pkt.get(), (size_t) pkt_len, (uint32_t) data_len, false) == pkt_len)
        {
            if(!m_packet_count)
            {
                m_last_sender_report_time = rtp_time::now();
            }
            m_packet_count++;
            m_octet_count += data_len;
            update_stream_bitrate(pkt_len);
            return true;
        }

        return false;
    }

    uint32_t sender::get_next_timestamp()
    {
        if(!m_packet_count)
        {
            m_last_packet_time = rtp_time::now();
            m_timestamp = (uint32_t) (rand() % 0xFFFF);
            return m_timestamp;
        }

        rtp_time curtime = rtp_time::now();
        rtp_time diff = curtime;
        diff -= m_last_packet_time;
        uint32_t tsdiff = (uint32_t) ((diff.get_double() / m_timestamp_unit) + 0.5);
        m_last_packet_time = curtime;
        m_timestamp += tsdiff;
        return m_timestamp;
    }

    uint8_t* sender::make_sender_report(uint8_t *data,
                                        uint32_t packet_count,
                                        uint32_t octet_count,
                                        uint32_t timestamp,
                                        uint32_t ntptime_lsw,
                                        uint32_t ntptime_msw) const
    {
        auto* cmn_hdr = (rtcp_common_header*) data;
        cmn_hdr->version = RTP_VERSION;
        cmn_hdr->packettype = RTP_RTCPTYPE_SR;
        cmn_hdr->length = htons(6);
        data += sizeof(rtcp_common_header);

        auto* ssrc = (rtp_source_identifier*) data;
        ssrc->ssrc = htonl(m_ssrc);
        data += sizeof(rtp_source_identifier);

        auto* sr = (rtcp_sender_report*) data;
        sr->octetcount = htonl(octet_count);
        sr->packetcount = htonl(packet_count);
        sr->rtptimestamp = htonl(timestamp);
        sr->ntptime_lsw = htonl(ntptime_lsw);
        sr->ntptime_msw = htonl(ntptime_msw);
        data += sizeof(rtcp_sender_report);

        return data;
    }

    uint8_t* sender::make_subflow_sender_report(uint8_t *data,
                                                uint32_t ntptime_lsw,
                                                uint32_t ntptime_msw)
    {
        uint8_t* start = data;
        auto* mprtp_hdr = (rtcp_common_header*) data;
        mprtp_hdr->version = RTP_VERSION;
        mprtp_hdr->packettype = RTP_RTCPTYPE_MPRTP;
        data += sizeof(rtcp_common_header);

        //  SSRC of packet sender
        auto* ssrc = (rtp_source_identifier*) data;
        ssrc->ssrc = htonl(m_ssrc);
        data += sizeof(rtp_source_identifier);

        // SSRC_1 (SSRC of first source)
        ssrc = (rtp_source_identifier*) data;
        ssrc->ssrc = htonl(m_ssrc);
        data += sizeof(rtp_source_identifier);

        for(auto& subflow : m_subflows)
        {
            auto* ssr_hdr = (mprtp_rtcp_subflow*) data;
            uint8_t* start_ssr = data;
            ssr_hdr->subflow_id = htons(subflow->get_id());
            ssr_hdr->mprtcp_type = MPRTCP_TYPE_SUBFLOW;
            data += sizeof(mprtp_rtcp_subflow);
            data = make_sender_report(data,
                                      subflow->get_packet_count(),
                                      subflow->get_octet_count(),
                                      m_timestamp, ntptime_lsw, ntptime_msw);
            ssr_hdr->block_length = (uint8_t) ((data - start_ssr) / 4);
        }
        mprtp_hdr->length = htons((uint16_t) ((data - start - 1) / 4));
        return data;
    }

    void sender::send_sender_report()
    {
        std::unique_ptr<uint8_t> pkt(new uint8_t[MAXPACKSIZE]);
        memset(pkt.get(), 0, MAXPACKSIZE);

        get_next_timestamp();
        rtp_ntp_time ntptimestamp = m_last_packet_time.get_ntp_time();

        uint8_t* data_ptr = make_sender_report(pkt.get(),
                                               m_packet_count,
                                               m_octet_count,
                                               m_timestamp,
                                               ntptimestamp.get_lsw(),
                                               ntptimestamp.get_msw());
        data_ptr = make_subflow_sender_report(data_ptr, ntptimestamp.get_lsw(), ntptimestamp.get_msw());
        size_t pkt_len = data_ptr - pkt.get();

        auto sf = select_subflow();
        sf->sendto(pkt.get(), pkt_len, 0, true);
        update_stream_bitrate((uint32_t) pkt_len);
        m_last_sender_report_time = m_last_packet_time;
    }

    bool sender::on_rtcp_rr_packet(rtcp_rr_packet::ptr& pkt, uint16_t subflow_id, uint32_t sender_ssrc, interface::shared_ptr& iface)
    {
#ifdef MPRTP_DEBUG
        printf("got RR sfid:%u, sender_ssrc:%X => ", subflow_id, sender_ssrc);
#endif
        if(subflow_id != 0)
        {
            for(auto& sf : m_subflows)
            {
                if(sf->get_id() == subflow_id)
                {
                    sf->process_rr_packet(pkt);
                    m_recalc_distribution = true;
                    break;
                }
            }
        }
#ifdef MPRTP_DEBUG
        pkt->print();
#endif
        return true;
    }

    void sender::recalc_subflow_percentage()
    {
        int congested_count = 0;
        int lossy_count = 0;
        for(auto& sf : m_subflows)
        {
            if(!sf->is_ready_for_distribution())
                return;

            if(sf->is_congested())
            {
                congested_count++;
            } else if(sf->is_lossy())
            {
                lossy_count++;
            }
        }
        if(congested_count == 0)
        {
            double sum_TB = 0;
            for(auto& sf : m_subflows)
            {
                sum_TB += sf->get_tested_bitrate();
            }
            for(auto& sf : m_subflows)
            {
                sf->set_percentage(sf->get_tested_bitrate() / sum_TB);
            }
        } if(congested_count == (int) m_subflows.size())
        {
            double sum_CB = 0;
            for(auto& sf : m_subflows)
            {
                sum_CB += sf->get_congested_bitrate();
            }
            for(auto& sf : m_subflows)
            {
                sf->set_percentage(sf->get_congested_bitrate() / sum_CB);
            }
        } else
        {
            // If some paths are congested, while others are not i.e c < n, we use a stepwise
            // approach.
            double sum_TB = 0;
            for(auto& sf : m_subflows)
            {
                sum_TB += sf->get_tested_bitrate();
            }

            double AP = 0;
            std::vector<subflow::ptr> rest;
            for(auto& sf : m_subflows)
            {
                if(sf->is_congested())
                {
                    double p = std::min(sf->get_tested_bitrate() / sum_TB,
                                        beta * sf->get_congested_bitrate() / m_stream_bitrate);
                    sf->set_percentage(p);
                    AP += p;
                } else if(sf->is_lossy())
                {
                    double p = std::min(sf->get_tested_bitrate() / sum_TB,
                                        gamma * sf->get_congested_bitrate() / m_stream_bitrate);
                    sf->set_percentage(p);
                    AP += p;
                } else
                {
                    rest.push_back(sf);
                }
            }

            if(!rest.empty())
            {
                double sum_unass_TB = 0;
                for (auto &sf : rest)
                {
                    sum_unass_TB += sf->get_tested_bitrate();
                }
                if(sum_unass_TB != 0)
                {
                    for (auto &sf : rest)
                    {
                        double p = (sf->get_tested_bitrate() / sum_unass_TB) * (1.0 - AP);
                        sf->set_percentage(p);
                    }
                }
            }
        }
#ifdef MPRTP_DEBUG
        for(auto& sf : m_subflows)
        {
            printf("subflow #%hu: %0.1f%%\n", sf->get_id(), sf->get_percentage() * 100.0);
            printf("           B: %0.3f\n", sf->get_B());
            printf("          TB: %0.3f\n", sf->get_tested_bitrate());
            printf("          CB: %0.3f\n", sf->get_congested_bitrate());
            printf("          CI: %d\n", sf->get_CI());
            printf("   congested: %d\n", sf->is_congested() ? 1 : 0);
            printf("       lossy: %d\n", sf->is_lossy() ? 1 : 0);
        }
        printf("          SB: %0.3f\n", m_stream_bitrate);
#endif
    }
}
