#ifndef SOCKS5_PEER_H_3894589325_8596472893475
#define SOCKS5_PEER_H_3894589325_8596472893475
#include "core/peer.h"
#include "core/stream.h"
#include <memory>
#include "privacy/privacy.h"
#include "core/address.h"

struct Socks5Peer : public Peer
{

    typedef std::vector<unsigned char > Bytes;

    Socks5Peer(const Bytes &k, const Bytes &u, const Bytes &p, const Address & remote, unsigned t);
    virtual ~Socks5Peer();

    virtual std::shared_ptr<Stream> build_stream(const std::shared_ptr<Address> &target);

    virtual std::vector<unsigned char> &get_key();
    virtual std::vector<unsigned char> &get_user();
    virtual std::vector<unsigned char> &get_pass();
    virtual std::shared_ptr<Privacy> &get_Method();

private:
    int sayHello(std::shared_ptr<Stream> &sp);
    int VerifyAuth(std::shared_ptr<Stream> &sp);
    std::shared_ptr<Address> doConnect(std::shared_ptr<Stream> &sp, const std::shared_ptr<Address> &target);

private:
    Bytes key;
    Bytes user;
    Bytes pass;
    Address proxy;
    std::shared_ptr<Privacy> method;
    unsigned int time;
};

#endif