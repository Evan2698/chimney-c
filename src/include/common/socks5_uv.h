/*******************************************************************************
 *
 * socks5 server 
 * implemented by uv.
 *
 * Copyright 2020 Evan . All rights reserved.
 *
 ******************************************************************************/
#ifndef SOCKS5_2020_EVAN_HELLO_H
#define SOCKS5_2020_EVAN_HELLO_H
#include "address.h"
#include "uv.h"

struct Socks5_uv
{
    Socks5_uv();
    ~Socks5_uv();

    int launch(Address local);
    int run();

private:
    static void on_connection(uv_stream_t *server, int status);

private:
    uv_tcp_t tcp_server;
};

#endif