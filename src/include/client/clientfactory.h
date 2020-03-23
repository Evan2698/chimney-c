#ifndef CLIENT_FACTORY_H_3983827
#define CLIENT_FACTORY_H_3983827
#include "common/clientbuilder.h"
#include <vector>
#include "common/address.h"

struct ClientFactory : public ClientBuilder
{
    virtual ~ClientFactory();
    virtual std::shared_ptr<Protocol> build_client();

    void set_profile(std::vector<unsigned char> user, std::vector<unsigned char> pass);
    void set_key(std::vector<unsigned char> key);
    void set_remote(Address a);

    static ClientFactory &get_instance();

private:
    ClientFactory();

private:
    std::vector<unsigned char> usr;
    std::vector<unsigned char> pwd;
    std::vector<unsigned char> ky;
    Address addr;
};

#endif