#ifndef PEER_FACTORY_H
#define PEER_FACTORY_H
#include "core/stream.h"
#include "core/address.h"
#include "privacy/privacy.h"
#include "core/peer.h"
struct PeerFactory
{

    static PeerFactory &get_instance();

    std::shared_ptr<Peer> build_socks5_peer();

    void set_proxy(const Address &r);

    void set_key(const std::vector<unsigned char> &k);

    void set_user_pass(const std::vector<unsigned char> &u, const std::vector<unsigned char> &p);

    void set_time_out(unsigned int t);

private:
    PeerFactory();

private:
    std::vector<unsigned char> key;
    std::vector<unsigned char> user;
    std::vector<unsigned char> pass;
    Address proxy;
    unsigned int time;
};

#endif