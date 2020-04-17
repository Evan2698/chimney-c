#include "core/socketassistant.h"
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "core/g.h"
#include "rpc/callerpool.h"

int SocketAssistant::create_socket(Address a, const std::string &network)
{
    return create_socket_raw(a, network, false);
}

int SocketAssistant::create_socket_raw(Address a, const std::string &network, bool raw)
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

    if (!raw)
    {
        callerpool::get_instance().get_caller().call_protect_socket(sock);
    }

    LOG(INFO) << "create_socket "
              << "will connect!!!" << address.sin_addr.s_addr << " X " << address.sin_port;
    if ((err = connect(sock, (struct sockaddr *)&address, sizeof(address))) < 0)
    {
        close(sock);
        LOG(ERROR) << "connect socket failed: " << a.toString() << err << std::endl;
        return -1;
    }

    LOG(INFO) << "Connect success~";

    return sock;
}

Address SocketAssistant::get_socket_local_address(int fd)
{
    struct sockaddr_storage addr = {0};

    socklen_t len = sizeof(addr);

    if (::getsockname(fd, reinterpret_cast<struct sockaddr *>(&addr), &len) < 0)
    {
        return Address();
    }

    return Address(reinterpret_cast<struct sockaddr *>(&addr));
}

int SocketAssistant::create_listening_socket(Address a, const std::string &network)
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

int SocketAssistant::set_socket_time(int fd, unsigned int time)
{
    struct timeval tv = {0};
    tv.tv_sec = time;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    return 0;
}

int SocketAssistant::create_udp_listening_socket(Address a)
{

    LOG(INFO) << a.host() << "--->" << a.port() << std::endl;

    struct sockaddr_in ser_addr = {0};
    ser_addr.sin_family = AF_INET;

    ser_addr.sin_port = htons(a.port());
    auto err = inet_pton(AF_INET, a.host().c_str(), &ser_addr.sin_addr);
    if (err < 0)
    {
        LOG(ERROR) << "create socket failed: " << err << std::endl;
        return -1;
    }

    int server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_fd < 0)
    {
        LOG(ERROR) << "create udp socket: " << server_fd << std::endl;
        return -1;
    }

    auto ret = bind(server_fd, (struct sockaddr *)&ser_addr, sizeof(ser_addr));
    if (ret < 0)
    {
        close(server_fd);
        LOG(ERROR) << "bind address  " << ret << std::endl;
        return -1;
    }

    return server_fd;
}

int SocketAssistant::create_udp_connect_socket(Address a)
{
    return create_udp_connect_socket_raw(a, false);
}

int SocketAssistant::create_udp_connect_socket_raw(Address a, bool raw)
{
    int server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (!raw)
    {
        callerpool::get_instance().get_caller().call_protect_socket(server_fd);
    }

    return server_fd;
}

std::optional<sockaddr_in> SocketAssistant::create_udp_connect_addr(Address a)
{
    struct sockaddr_in servaddr = {0};
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(a.port());
    auto err = inet_pton(AF_INET, a.host().c_str(), &servaddr.sin_addr);
    if (err < 0)
    {
        LOG(ERROR) << "parse host address failed: " << err << std::endl;
        return std::nullopt;
    }

    return servaddr;
}