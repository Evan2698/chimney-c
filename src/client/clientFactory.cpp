#include "client/clientfactory.h"
#include "client/serverstream.h"
#include "client/clienthandle.h"
#include "glog/logging.h"
#include <thread>

ClientFactory::ClientFactory()
{
}

ClientFactory::~ClientFactory()
{
}

int ClientFactory::delegate_to_client(int fd)
{
    LOG(INFO) << "RA " << this->addr.toString() << std::endl; 
    std::thread t([this](int s) {
        LOG(INFO) << "serve on " << s << "   ra: " << this->addr.toString() << std::endl;
        auto handle = std::shared_ptr<ClientHandle>(new ClientHandle(
            this->addr,
            this->usr,
            this->pwd,
            this->ky));

        ServerStream *p = new ServerStream();
        p->set_client(handle);
        handle->register_io(p);
        auto ret = p->init(s);
        if (!ret)
        {
            LOG(INFO) << "object ServerStream will be destory!!!";
            delete p;
        }
    }, fd);

    t.detach();
    // need not destroy it.
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