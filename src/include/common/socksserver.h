/*******************************************************************************
 *
 * socksserver
 * 
 * socks5 server 
 *
 * Copyright 2020 Evan . All rights reserved.
 *
 ******************************************************************************/

#ifndef SOCKS5SERVER_H
#define SOCKS5SERVER_H
#include "address.h"
#include <ev++.h>
#include "clientbuilder.h"

class Socks5Server
{
private:
    /* data */
    ev::default_loop loop;
    ev::io io;
    ev::sig sio;
    int s;
    ClientBuilder &cb;

public:
    Socks5Server(ClientBuilder & cbx);
    ~Socks5Server();

    void io_accept(ev::io &watcher, int revents);

    static void signal_cb(ev::sig &signal, int revents);
    

    int init(Address a, const std::string &network);
    void run();

    void stop();
};
#endif
