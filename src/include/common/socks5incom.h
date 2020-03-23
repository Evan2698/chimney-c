/*******************************************************************************
 *
 * socks income stream 
 * implemented with uv.
 *
 * Copyright 2020 Evan . All rights reserved.
 *
 ******************************************************************************/
#ifndef SOCKS5_INCOME_UV_H
#define SOCKS5_INCOME_UV_H
#include "uv.h"
#include <atomic>
#include <string>
#include <memory>
#include <common/Protocol.h>

struct Socks5Income;

struct WriteStreamHub : public WriteStream
{

    WriteStreamHub(Socks5Income *p);
    ~WriteStreamHub();
    virtual int write_to_stream(char *out, unsigned int len);

private:
    Socks5Income * pwriter;
};

struct Socks5Income
{
    Socks5Income();
    ~Socks5Income();

    int on_connect(uv_stream_t *server);
    virtual int write_to_stream(char *out, unsigned int len);

private:
    static void alloc_cb(uv_handle_t *handle,
                         size_t suggested_size,
                         uv_buf_t *buf);

    static void after_read(uv_stream_t *handle,
                           ssize_t nread,
                           const uv_buf_t *buf);

    static void after_shutdown(uv_shutdown_t *req, int status);

    static void on_close(uv_handle_t *handle);

    static void after_write(uv_write_t *req, int status);

private:
    void start_shutdown();

   

private:
    uv_stream_t *parent;
    uv_tcp_t stream;
    static std::atomic_uint object_counter;
    std::string name;
    std::shared_ptr<Protocol> router;
    std::shared_ptr<WriteStream> hub;
};

#endif;