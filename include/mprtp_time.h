#ifndef MPRTP_MPRTP_TIME_H
#define MPRTP_MPRTP_TIME_H

#include <cstdint>
#include <cerrno>
#include <time.h>

#define RTP_NTPTIMEOFFSET	2208988800UL

namespace mprtplib
{

/**
 * This is a simple wrapper for the most significant word (MSW) and least
 * significant word (LSW) of an NTP timestamp.
 */
    class rtp_ntp_time
    {
    public:
        /** This constructor creates and instance with MSW \c m and LSW \c l. */
        rtp_ntp_time(uint32_t m,uint32_t l)					{ m_msw = m ; m_lsw = l; }

        /** Returns the most significant word. */
        uint32_t get_msw() const							{ return m_msw; }

        /** Returns the least significant word. */
        uint32_t get_lsw() const							{ return m_lsw; }
    private:
        uint32_t m_msw;
        uint32_t m_lsw;
    };

/** This class is used to specify wallclock time, delay intervals etc.
 *  It stores a number of seconds and a number of microseconds.
 */
    class rtp_time
    {
    public:
        /** Returns an rtp_time instance representing the current wallclock time.
         *  as a number of seconds since 00:00:00 UTC, January 1, 1970.
         */
        static rtp_time now();

        /** This function waits the amount of time specified in \c delay. */
        static void wait(const rtp_time &delay);

        /** Creates an rtp_time instance representing \c t, which is expressed in units of seconds. */
        rtp_time(double t);

        /** Creates an instance that corresponds to \c ntptime.
         *  the conversion cannot be made, both the seconds and the
         *  microseconds are set to zero.
         */
        rtp_time(rtp_ntp_time ntptime);

        /** Creates an instance corresponding to \c seconds and \c microseconds. */
        rtp_time(int64_t seconds, uint32_t microseconds);

        /** Returns the number of seconds stored in this instance. */
        int64_t get_seconds() const;

        /** Returns the number of microseconds stored in this instance. */
        uint32_t get_microseconds() const;

        /** Returns the time stored in this instance, expressed in units of seconds. */
        double get_double() const 										{ return m_t; }

        /** Returns the NTP time corresponding to the time stored in this instance. */
        rtp_ntp_time get_ntp_time() const;

        rtp_time &operator-=(const rtp_time &t);
        rtp_time &operator+=(const rtp_time &t);
        bool operator<(const rtp_time &t) const;
        bool operator>(const rtp_time &t) const;
        bool operator<=(const rtp_time &t) const;
        bool operator>=(const rtp_time &t) const;

        bool is_zero() const { return m_t == 0; }
    private:

        double m_t;
    };

    inline rtp_time::rtp_time(double t)
    {
        m_t = t;
    }

    inline rtp_time::rtp_time(int64_t seconds, uint32_t microseconds)
    {
        if (seconds >= 0)
        {
            m_t = (double)seconds + 1e-6*(double)microseconds;
        }
        else
        {
            int64_t possec = -seconds;

            m_t = (double)possec + 1e-6*(double)microseconds;
            m_t = -m_t;
        }
    }

    inline rtp_time::rtp_time(rtp_ntp_time ntptime)
    {
        if (ntptime.get_msw() < RTP_NTPTIMEOFFSET)
        {
            m_t = 0;
        }
        else
        {
            uint32_t sec = ntptime.get_msw() - RTP_NTPTIMEOFFSET;

            double x = (double) ntptime.get_lsw();
            x /= (65536.0*65536.0);
            x *= 1000000.0;
            uint32_t microsec = (uint32_t)x;

            m_t = (double)sec + 1e-6*(double)microsec;
        }
    }

    inline int64_t rtp_time::get_seconds() const
    {
        return (int64_t)m_t;
    }

    inline uint32_t rtp_time::get_microseconds() const
    {
        uint32_t microsec;

        if (m_t >= 0)
        {
            int64_t sec = (int64_t)m_t;
            microsec = (uint32_t)(1e6*(m_t - (double)sec) + 0.5);
        }
        else // m_t < 0
        {
            int64_t sec = (int64_t)(-m_t);
            microsec = (uint32_t)(1e6*((-m_t) - (double)sec) + 0.5);
        }

        if (microsec >= 1000000)
            return 999999;
        // Unsigned, it can never be less than 0
        // if (microsec < 0)
        // 	return 0;
        return microsec;
    }

    inline double RTPTime_timespecToDouble(struct timespec &ts)
    {
        return (double)ts.tv_sec + 1e-9*(double)ts.tv_nsec;
    }

    inline rtp_time rtp_time::now()
    {
        static bool s_initialized = false;
        static double s_startOffet = 0;

        if (!s_initialized)
        {
            s_initialized = true;

            // Get the corresponding times in system time and monotonic time
            struct timespec tpSys, tpMono;

            clock_gettime(CLOCK_REALTIME, &tpSys);
            clock_gettime(CLOCK_MONOTONIC, &tpMono);

            double tSys = RTPTime_timespecToDouble(tpSys);
            double tMono = RTPTime_timespecToDouble(tpMono);

            s_startOffet = tSys - tMono;
            return tSys;
        }

        struct timespec tpMono;
        clock_gettime(CLOCK_MONOTONIC, &tpMono);

        double tMono0 = RTPTime_timespecToDouble(tpMono);
        return tMono0 + s_startOffet;
    }

    inline void rtp_time::wait(const rtp_time &delay)
    {
        if (delay.m_t <= 0)
            return;

        uint64_t sec = (uint64_t)delay.m_t;
        uint64_t nanosec = (uint32_t)(1e9*(delay.m_t-(double)sec));

        struct timespec req,rem;
        int ret;

        req.tv_sec = (time_t)sec;
        req.tv_nsec = ((long)nanosec);
        do
        {
            ret = nanosleep(&req,&rem);
            req = rem;
        } while (ret == -1 && errno == EINTR);
    }

    inline rtp_time &rtp_time::operator-=(const rtp_time &t)
    {
        m_t -= t.m_t;
        return *this;
    }

    inline rtp_time &rtp_time::operator+=(const rtp_time &t)
    {
        m_t += t.m_t;
        return *this;
    }

    inline rtp_ntp_time rtp_time::get_ntp_time() const
    {
        uint32_t sec = (uint32_t)m_t;
        uint32_t microsec = (uint32_t)((m_t - (double)sec)*1e6);

        uint32_t msw = sec + RTP_NTPTIMEOFFSET;
        uint32_t lsw;
        double x;

        x = microsec/1000000.0;
        x *= (65536.0*65536.0);
        lsw = (uint32_t)x;

        return rtp_ntp_time(msw,lsw);
    }

    inline bool rtp_time::operator<(const rtp_time &t) const
    {
        return m_t < t.m_t;
    }

    inline bool rtp_time::operator>(const rtp_time &t) const
    {
        return m_t > t.m_t;
    }

    inline bool rtp_time::operator<=(const rtp_time &t) const
    {
        return m_t <= t.m_t;
    }

    inline bool rtp_time::operator>=(const rtp_time &t) const
    {
        return m_t >= t.m_t;
    }

} // end namespace


#endif //MPRTP_MPRTP_TIME_H
