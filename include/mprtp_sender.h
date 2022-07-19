#ifndef MPRTP_MPRTP_SENDER_H
#define MPRTP_MPRTP_SENDER_H

#include "mprtp_session.h"
#include "mprtp_defines.h"
#include "mprtp_subflow.h"

namespace mprtplib
{
    class sender : public session
    {
    public:
        sender(double timestamp_unit);

        bool connect(uint32_t ssrc, const address& destination);
        bool send_data(const uint8_t* data, int data_len);

        virtual bool is_path_valid(const endpoint& local, const endpoint& remote) const = 0;
        bool on_rtcp_rr_packet(rtcp_rr_packet::ptr& pkt, uint16_t subflow_id, uint32_t sender_ssrc, interface::shared_ptr& iface) override ;

        int poll() override ;

    private:
        subflow::ptr select_subflow();
        uint32_t get_next_timestamp();
        void send_sender_report();
        uint8_t* make_sender_report(uint8_t *data,
                                    uint32_t packet_count,
                                    uint32_t octet_count,
                                    uint32_t timestamp,
                                    uint32_t ntptime_lsw,
                                    uint32_t ntptime_msw) const;
        uint8_t* make_subflow_sender_report(uint8_t *data,
                                            uint32_t ntptime_lsw,
                                            uint32_t ntptime_msw);

    private:
        void recalc_subflow_percentage();
        void update_stream_bitrate(uint32_t size)
        {
            m_sb_size += size;
            if(m_sb_time.is_zero())
            {
                m_sb_time = rtp_time::now();
            }
        }

        int                       m_next_subflow;
        uint16_t                  m_sequesnce_number;
        uint32_t                  m_timestamp;
        uint16_t                  m_next_subflow_id;
        uint32_t                  m_ssrc;
        address                   m_dest_addr;
        double                    m_timestamp_unit;
        std::vector<subflow::ptr> m_subflows;
        bool                      m_recalc_distribution;

        // sender stat
        uint32_t                  m_packet_count;
        uint32_t                  m_octet_count;
        rtp_time                   m_last_packet_time;
        rtp_time                   m_last_sender_report_time;
        double                    m_stream_bitrate;     /// SB - sending bitrate 𝑆𝐵𝑖,
                                                        /// which is the current average bitrate of the stream updated after
                                                        /// every second.
        uint32_t                  m_sb_size;            /// Bytes count since last SB calculation
        rtp_time                   m_sb_time;            /// Time of last calculation
    };
}

#endif //MPRTP_MPRTP_SENDER_H
