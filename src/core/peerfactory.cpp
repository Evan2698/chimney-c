#include "core/peerfactory.h"
#include "core/socket.h"
#include "core/func.hpp"
#include <optional>
#include "core/g.h"
#include "core/socks5peer.h"
PeerFactory::PeerFactory()
{
}

PeerFactory &PeerFactory::get_instance()
{
    static PeerFactory one;
    return one;
}

 std::shared_ptr<Peer> PeerFactory::build_socks5_peer()
 {
     //Socks5Peer(const Bytes &k, const Bytes &u, const Bytes &p, const Address & remote);
     auto sp = std::make_shared<Socks5Peer>(
         this->key,
         this->user,
         this->pass,
         this->proxy);

    return sp;

 }


void PeerFactory::set_proxy(const Address &r)
{
    this->proxy = r;
}

void PeerFactory::set_key(const std::vector<unsigned char> &k)
{
    this->key.resize(k.size());
    std::copy(k.begin(), k.end(), key.begin());
}

void PeerFactory::set_user_pass(const std::vector<unsigned char> &u, const std::vector<unsigned char> &p)
{
    this->user.resize(u.size());
    std::copy(u.begin(), u.end(), user.begin());

    this->pass.resize(p.size());
    std::copy(p.begin(), p.end(), pass.begin());
}