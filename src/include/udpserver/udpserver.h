#ifndef UDP_SERVER_H_34834834734750_848574
#define UDP_SERVER_H_34834834734750_848574
#include "core/address.h"
#include "privacy/privacy.h"
#include <memory>
class UDPServer
{
private:
  Address local;
  Address remote;
  std::shared_ptr<Privacy> I;
  std::vector<unsigned char> key;

  
public:
    UDPServer(Address l, Address r, std::shared_ptr<Privacy> &i, std::vector<unsigned char> &v);
    ~UDPServer();

    int Run();
};

#endif