#include <cstdio>
#include <arpa/inet.h>
#include <string>
#include <mprtp_sender.h>
#include <mprtp_receiver.h>
#include <thread>
#include <mutex>
#include <iostream>

#define PORT_SERVER_BASE       22000
#define PORT_CLIENT_BASE       23000

static volatile int stop_client = 0;
static std::thread poll_thread;

static void signalHandler( int signum )
{
    stop_client = 1;
    poll_thread.join();
    exit(0);
}

class my_sender : public mprtplib::sender
{
public:
    my_sender(double ts_unit) : mprtplib::sender(ts_unit)
    {}

    bool is_path_valid(const mprtplib::endpoint& local, const mprtplib::endpoint& remote) const override
    {
        return mprtplib::in_same_subnet(local, remote, 24);
    }
};

static void usage(const char* pname)
{
    fprintf(stderr, "Usage: %s -i local_ipaddr_1 [... -i local_ipaddr_N] [-p local_port] -r remote_ipaddr_1 [... -r remote_ipaddr_N] [-s remote_port]\n",
            pname);
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[])
{
    signal(SIGINT, signalHandler);

    std::vector<std::string> local_ifaces;
    std::vector<std::string> remote_ifaces;
    uint16_t local_port = PORT_CLIENT_BASE;
    uint16_t remote_port = PORT_SERVER_BASE;

    int opt;

    while ((opt = getopt(argc, argv, "p:i:r:s:")) != -1)
    {
        switch (opt)
        {
            case 'p':
                local_port = (uint16_t) atoi(optarg);
                break;
            case 's':
                remote_port = (uint16_t) atoi(optarg);
                break;
            case 'i':
                local_ifaces.emplace_back(optarg);
                break;
            case 'r':
                remote_ifaces.emplace_back(optarg);
                break;
            default:
                usage(argv[0]);
                break;
        }
    }

    if(local_ifaces.empty() || remote_ifaces.empty())
    {
        usage(argv[0]);
    }

    my_sender cli(1.0 / 1000.0);

    for( const auto& addr : local_ifaces)
    {
        cli.add_interface(mprtplib::create_endpoint(addr, local_port));
        printf("MPRTP: Listen on %s:%hu\n", addr.c_str(), local_port);
        local_port += 2;
    }

    mprtplib::address dest;
    for( const auto& addr : remote_ifaces)
    {
        dest.push_back(mprtplib::create_endpoint(addr, remote_port));
        printf("MPRTP: connect to %s:%hu\n", addr.c_str(), remote_port);
        remote_port += 2;
    }

    if(cli.connect(0xDEAD, dest))
    {
        printf("Enter text and press ENTER\n");
        printf("Press CTRL+C to exit\n");

        std::mutex sync;
        std::list<std::string> queue;
        poll_thread = std::thread(
                [&](){
                    while (!stop_client)
                    {
                        sync.lock();
                        cli.poll();
                        sync.unlock();
                        usleep(1000);
                    }
                });
        std::string buff;
        while (std::getline(std::cin, buff))
        {
            if(buff.empty()) break;

            sync.lock();
            cli.send_data((uint8_t*) buff.c_str(), (int) buff.length());
            sync.unlock();
        }
        poll_thread.join();
    }

    return 0;
}