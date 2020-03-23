#include "client/clientfactory.h"
#include "glog/logging.h"
#include "common/socks5protocol.h"
#include <thread>

ClientFactory::ClientFactory()
{
}

ClientFactory::~ClientFactory()
{
}

std::shared_ptr<Protocol> ClientFactory::build_client()
{
    LOG(INFO) << "RA " << this->addr.toString() << std::endl; 
    auto sp = std::make_shared<Socks5Protocol>();
    sp->set_key(this->ky);
    sp->set_remote(this->addr);
    sp->set_user_pass(this->usr, this->pwd);
    return sp; 
}

ClientFactory &ClientFactory::get_instance()
{
    static ClientFactory fc;
    return fc;
}

void ClientFactory::set_profile(std::vector<unsigned char> user, std::vector<unsigned char> pass)
{
    this->usr = user;
    this->pwd = pass;
}
void ClientFactory::set_key(std::vector<unsigned char> key)
{
    this->ky = key;
}
void ClientFactory::set_remote(Address a)
{
    this->addr = a;
}