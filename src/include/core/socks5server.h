#ifndef SOCKET_SOCKS5_SERVER_H
#define SOCKET_SOCKS5_SERVER_H
#include "core/address.h"
#include "core/stream.h"
struct Socks5Server
{
    Socks5Server(Address listening);
    ~Socks5Server();

    int run();

    void shutdown();

private:
    static void doServeOnOne(Stream * sp);
    static void serve_on(Stream * sp);

private:
    Address listen_address;
    int handle;

};

#endif