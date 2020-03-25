#ifndef PEER_HOLDE_SOCKET_H
#define PEER_HOLDE_SOCKET_H
#include <memory>
#include "core/stream.h"
#include <vector>
#include "privacy/privacy.h"
struct Peer
{

    Peer() = default;
    virtual ~Peer() = default;

    virtual std::shared_ptr<Stream> build_stream(const std::shared_ptr<Address> &target) = 0;

    virtual std::vector<unsigned char> &get_key() = 0;
    virtual std::vector<unsigned char> &get_user() = 0;
    virtual std::vector<unsigned char> &get_pass() = 0;
    virtual std::shared_ptr<Privacy> &get_Method() = 0;
};

#endif