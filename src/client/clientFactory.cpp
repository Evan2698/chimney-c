#include "client/clientfactory.h"
#include "glog/logging.h"
#include "common/socks5protocol.h"
#include "common/func.hpp"


ClientFactory::ClientFactory()
{
    LOG(INFO) << "ClientFactory ---------------->" << std::endl;
}

ClientFactory::~ClientFactory()
{
    LOG(INFO) << "~~ClientFactory <----------------" << std::endl;
}

std::shared_ptr<Protocol> ClientFactory::build_client()
{
    LOG(INFO) << "RA " << this->proxy.toString() << "----"
              << ToHexEX(this->ky.begin(), this->ky.end()) << " ----"
              << ToHexEX(this->usr.begin(), this->usr.end()) << std::endl;
    auto sp = std::make_shared<Socks5Protocol>();
    sp->set_key(this->ky);
    sp->set_remote(this->proxy);
    sp->set_user_pass(this->usr, this->pwd);
    return sp;
}


std::mutex ClientFactory::_mutext_;
ClientFactory *ClientFactory::pinstance = nullptr;
ClientFactory *ClientFactory::get_instance()
{
    if (pinstance == nullptr)
    {
        std::lock_guard<std::mutex> lok(_mutext_);
        if (pinstance == nullptr)
        {
            pinstance = new ClientFactory();
        }
    }

    return pinstance;
}

void ClientFactory::set_profile(std::vector<unsigned char> user, std::vector<unsigned char> pass)
{
    usr = user;
    pwd = pass;
}
void ClientFactory::set_key(std::vector<unsigned char> key)
{
    ky = key;
}
void ClientFactory::set_remote(Address a)
{
    LOG(INFO) << "RRRRR-OMTE " << a.toString() << std::endl;
    proxy = a;
    LOG(INFO) << "RRRRR-ADDR " << proxy.toString() << std::endl;
}