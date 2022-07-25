//
// Created by tordex on 25.07.22.
//
#include <cstdio>
#include <arpa/inet.h>
#include <cstring>
#include <mprtp_receiver.h>
#include <signal.h>

#define PORT_SERVER_BASE       22000
#define PORT_CLIENT_BASE       23000

static volatile int stop_listen = 0;

static void signalHandler( int signum )
{
    stop_listen = 1;
}

static void usage(const char* pname)
{
    fprintf(stderr, "Usage: %s -i ipaddr1 [... -i ipaddrN] [-p baseport]\n",
            pname);
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[])
{
    signal(SIGINT, signalHandler);

    mprtplib::receiver server(1.0 / 1000.0, 0xBEAF);

    std::vector<std::string> ifaces;

    int opt;
    uint16_t port = PORT_SERVER_BASE;

    while ((opt = getopt(argc, argv, "p:i:")) != -1)
    {
        switch (opt)
        {
            case 'p':
                port = (uint16_t) atoi(optarg);
                break;
            case 'i':
                ifaces.emplace_back(optarg);
                break;
            default:
                usage(argv[0]);
                break;
        }
    }

    if(ifaces.empty())
    {
        usage(argv[0]);
    }

    for(const auto& address: ifaces)
    {
        server.add_interface(mprtplib::create_endpoint(address, port));
        printf("MPRTP: Listen on %s:%hu\n", address.c_str(), port);
        port += 2;
    }
    printf("Waiting for data...\nPress CTRL+C to stop server\n");

    while (!stop_listen)
    {
        auto pkt = server.fetch_packet();
        while (pkt)
        {
            auto data = pkt->get_payload_data();
            printf("packet: %u bytes\n", (uint32_t) pkt->get_payload_length());
            printf("data:   ");
            for(uint32_t i = 0; i < (uint32_t) pkt->get_payload_length(); i++)
            {
                printf("%02X ", data[i]);
            }
            printf("\n");
            pkt = server.fetch_packet();
        }
        server.poll();
        usleep(1000);
    }

    return 0;
}