#include "core/socket.h"
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "core/g.h"
int SocketBuilder::create_socket(Address a, const std::string &network)
{
    LOG(INFO) << "create_socket " << a.toString();
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(a.port());
    auto err = inet_pton(AF_INET, a.host().c_str(), &address.sin_addr);
    if (err < 0)
    {
        LOG(ERROR) << "create socket failed: " << err << std::endl;
        return -1;
    }

    int sock = 0;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        LOG(ERROR) << "create socket failed: " << sock << std::endl;
        return -1;
    }    

    LOG(INFO) << "create_socket " << "will connect!!!" << address.sin_addr.s_addr << " X " << address.sin_port;
    if ((err = connect(sock, (struct sockaddr *)&address, sizeof(address))) < 0)
    {
        close(sock);
        LOG(ERROR) << "connect socket failed: " << a.toString() << err << std::endl;
        return -1;
    }

    LOG(INFO) << "Connect success~";   

    return sock;
}

Address SocketBuilder::get_socket_local_address(int fd)
{
    struct sockaddr_storage addr = {0};

    socklen_t len = sizeof(addr);

    if (::getsockname(fd, reinterpret_cast<struct sockaddr *>(&addr), &len) < 0)
    {
        return Address();
    }

    return Address(reinterpret_cast<struct sockaddr *>(&addr));
}

int SocketBuilder::create_listening_socket(Address a, const std::string &network)
{
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(a.port());
    auto err = inet_pton(AF_INET, a.host().c_str(), &address.sin_addr);
    if (err < 0)
    {
        LOG(ERROR) << "create socket failed: " << err << std::endl;
        return -1;
    }
    auto fd = socket(AF_INET, SOCK_STREAM, 0);
  /*  if (fd > 0)
    {
        fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
    }*/

    int reuse = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    err = bind(fd, (struct sockaddr *)&address, sizeof(address));
    if (err < 0)
    {
        close(fd);
        LOG(ERROR) << "bind socket address failed : " << err << std::endl;
        return -1;
    }
    return fd;
}