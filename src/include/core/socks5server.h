#ifndef SOCKET_SOCKS5_SERVER_H
#define SOCKET_SOCKS5_SERVER_H
#include "core/address.h"
#include "core/stream.h"
#include <memory>
struct Socks5Server
{
    Socks5Server(Address listening);
    ~Socks5Server();

    int run();

    void shutdown();

private:
    static void doServeOnOne(std::shared_ptr<Stream> sp);
  

    void build_service_routine(int df, Address & a);

private:
    Address listen_address;
    int handle;
};

#endif