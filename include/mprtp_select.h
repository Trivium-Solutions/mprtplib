#ifndef MPRTP_MPRTP_SELECT_H
#define MPRTP_MPRTP_SELECT_H

#include <cstdio>
#include <cstdint>
#include <vector>
#include <poll.h>
#include <errno.h>

namespace mprtplib
{
    inline int rtp_select(const int* sockets, int8_t *readflags, size_t numsocks)
    {
        using namespace std;

        vector<struct pollfd> fds(numsocks);

        for (size_t i = 0 ; i < numsocks ; i++)
        {
            fds[i].fd = sockets[i];
            fds[i].events = POLLIN;
            fds[i].revents = 0;
            readflags[i] = 0;
        }

        int timeoutmsec = 0;

        int status = poll(&(fds[0]), numsocks, timeoutmsec);
        if (status < 0)
        {
            // We're just going to ignore an EINTR
            if (errno == EINTR)
                return 0;
            return status;
        }

        if (status > 0)
        {
            for (size_t i = 0 ; i < numsocks ; i++)
            {
                if (fds[i].revents)
                    readflags[i] = 1;
            }
        }
        return status;
    }

}

#endif //MPRTP_MPRTP_SELECT_H
