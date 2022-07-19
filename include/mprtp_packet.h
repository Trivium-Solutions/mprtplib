#ifndef MPRTP_MPRTP_PACKET_H
#define MPRTP_MPRTP_PACKET_H

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <memory>
#include <chrono>
#include "mprtp_time.h"
#include "mprtp_address.h"

namespace mprtplib
{
    class raw_packet
    {
    private:
        uint8_t*    m_data;
        size_t      m_len;
        endpoint    m_from;
        bool        m_isrtcp;
        rtp_time     m_recvtime;
    public:
        typedef std::unique_ptr<raw_packet>  unique_ptr;
        typedef std::shared_ptr<raw_packet>  shared_ptr;

        raw_packet(const uint8_t* data, size_t data_len, uint32_t fromip, uint16_t fromport, bool isrtcp, const rtp_time& recvtime) :
                m_data(new uint8_t[data_len]),
                m_len(data_len),
                m_from(fromip, fromport),
                m_isrtcp(isrtcp),
                m_recvtime(recvtime)
        {
            if(m_data)
                memcpy(m_data, data, data_len);
        }

        ~raw_packet()
        {
            delete m_data;
        }

        const uint8_t* data() const                 { return m_data; }
        size_t data_len() const                     { return m_len; }
        const endpoint& get_from_address() const    { return m_from; }
        bool is_rtcp() const                        { return m_isrtcp; }
        const rtp_time& recvtime() const             { return m_recvtime; }
    };

    class rtp_packet
    {
    private:
        raw_packet::unique_ptr m_rawpack;
        int             m_error;
        bool            m_hasextension;
        bool            m_hasmarker;
        int             m_numcsrcs;
        uint8_t         m_payloadtype;
        uint32_t        m_extseqnr;
        uint32_t        m_subflow_extseqnr;
        uint16_t        m_subflow_id;
        uint32_t        m_timestamp;
        uint32_t        m_ssrc;
        const uint8_t*  m_payload;
        size_t          m_payloadlength;
        uint16_t        m_extid;
        const uint8_t*  m_extension;
        size_t          m_extensionlength;
    public:
        typedef std::unique_ptr<rtp_packet> ptr;

        rtp_packet(raw_packet::unique_ptr& rawpack) :
                m_rawpack(std::move(rawpack)),
                m_error(0),
                m_hasmarker(0),
                m_hasextension(0),
                m_numcsrcs(0),
                m_payloadtype(0),
                m_extseqnr(0),
                m_timestamp(0),
                m_ssrc(0),
                m_payload(nullptr),
                m_payloadlength(0),
                m_extid(0),
                m_extension(nullptr),
                m_extensionlength(0),
                m_subflow_extseqnr(0),
                m_subflow_id(0)
        {
            m_error = parse_packet();
        }

        int get_error() const                                   { return m_error; }
        bool has_extension() const					            { return m_hasextension; }
        bool has_marker() const	    				            { return m_hasmarker; }
        int get_csrc_count() const					            { return m_numcsrcs; }
        uint32_t get_csrc(int num) const;
        uint8_t get_payload_type() const		                { return m_payloadtype; }
        uint32_t get_extended_sequencenumber() const	        { return m_extseqnr; }
        uint16_t get_sequence_number() const			        { return (uint16_t)(m_extseqnr & 0x0000FFFF); }
        void set_extended_sequence_number(uint32_t seq)	        { m_extseqnr = seq; }
        uint32_t get_timestamp() const					        { return m_timestamp; }
        uint32_t get_ssrc() const						        { return m_ssrc; }
        const uint8_t *get_packet_data() const			        { return m_rawpack->data(); }
        const uint8_t *get_payload_data() const			        { return m_payload; }
        size_t get_packet_length() const				        { return m_rawpack->data_len(); }
        size_t get_payload_length() const				        { return m_payloadlength; }
        uint16_t get_extension_id() const				        { return m_extid; }
        const uint8_t *get_extension_data() const		        { return m_extension; }
        size_t get_extension_length() const				        { return m_extensionlength; }
        bool is_mprtp_packet() const                            { return (m_hasextension && m_extid == 0xBEDE); }
        uint32_t get_subflow_extended_sequencenumber() const	{ return m_subflow_extseqnr; }
        uint16_t get_subflow_sequence_number() const			{ return (uint16_t)(m_subflow_extseqnr & 0x0000FFFF); }
        void set_subflow_extended_sequence_number(uint32_t seq)	{ m_subflow_extseqnr = seq; }
        uint16_t get_subflow_id() const	                        { return m_subflow_id; }
        const rtp_time& recvtime() const                        { return m_rawpack->recvtime(); }
        const endpoint& get_from_address() const                { return m_rawpack->get_from_address(); }

    private:
        int parse_packet();
    };
}

#endif //MPRTP_MPRTP_PACKET_H
