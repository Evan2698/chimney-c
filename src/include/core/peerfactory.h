#ifndef PEER_FACTORY_H
#define PEER_FACTORY_H
#include "core/stream.h"
#include "core/address.h"
#include "privacy/privacy.h"
struct PeerFactory
{

    static PeerFactory *get_instance();

    Stream * build_peer_with_target(const std::shared_ptr<Address> &target);


    void set_proxy(const Address & r);

    void set_key(const std::vector<unsigned char> & k);

    void set_user_pass(const std::vector<unsigned char> & u, const std::vector<unsigned char> & p);

    std::shared_ptr<Privacy> & getMethod();

    std::vector<unsigned char > & get_key();


private:
    int sayHello(Stream *sp);
    int VerifyAuth(Stream *sp);
    std::shared_ptr<Address> doConnect(Stream *sp, const Address &target);

private:
    PeerFactory();

private:
    std::vector<unsigned char > key;
    std::vector<unsigned char > user;
    std::vector<unsigned char > pass;
    Address proxy;
    std::shared_ptr<Privacy> method;

};

#endif