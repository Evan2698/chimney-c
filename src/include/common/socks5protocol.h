/*******************************************************************************
 *
 * socks5 protocol 
 * implemented with uv.
 *
 * Copyright 2020 Evan . All rights reserved.
 *
 ******************************************************************************/

#ifndef SOCKS5_PROTOCOL_H_239948844
#define SOCKS5_PROTOCOL_H_239948844
#include <mutex>
#include "common/Protocol.h"
#include <memory>
#include <uv.h>
#include "common/address.h"
#include "privacy/privacy.h"
#include <vector>
#include <string>

struct Socks5Protocol : public Protocol
{
    enum State
    {
        enum_Fresh = 0,
        enum_Hello,
        enum_Connect,
        enum_Auth,
        enum_Normal
    };

    Socks5Protocol();
    virtual ~Socks5Protocol();

    virtual int routing_and_answser(char *income, unsigned int len);

    virtual void register_write_stream(std::weak_ptr<WriteStream> wp);

    static unsigned int ToInt(const char *p);
    static std::string ToBytes(unsigned int n);
    virtual int query_status();

    void set_user_pass(const std::vector<unsigned char> &u, const std::vector<unsigned char>& p){
        this->user = u;
        this->pass = p;
    }
    void set_key(const std::vector<unsigned char> &ky){
        this->key = ky;
    }

    void set_remote(Address a){
        this->remote = a;
    }

private:
    int init_client();
    static void on_connect(uv_connect_t *req, int status);
    static void after_write_once(uv_write_t *req, int status);
    static void alloc_cb(uv_handle_t *handle,
                         size_t suggested_size,
                         uv_buf_t *buf);
    static void after_read(uv_stream_t *handle,
                           ssize_t nread,
                           const uv_buf_t *buf);
    static void after_shutdown(uv_shutdown_t *req, int status);
    static void afterr_write(uv_write_t *req, int status);

    void start_shutdown();
    static void on_close(uv_handle_t *handle);
    
    void send_user_pass();
    void send_target_to_remote();

    // Callback
    void BackError(unsigned char erorr_code);
    void Back_Local_Bound_Address(std::shared_ptr<Address> bound);
    void Back_Write(std::vector<unsigned char> &out);

private:
    State state;
    State peer_state;
    std::mutex _mutex;
    std::mutex _peer;
    uv_tcp_t client;
    uv_connect_t connect_req;
    std::weak_ptr<WriteStream> writer;
    std::shared_ptr<Address> target;
    Address remote;
    std::shared_ptr<Privacy> method;
    std::vector<unsigned char> key;
    std::vector<unsigned char> pass;
    std::vector<unsigned char> user;
    std::string incoming;
    int can_close;
};

#endif