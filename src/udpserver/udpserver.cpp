#include "udpserver/udpserver.h"
#include "core/socketassistant.h"
#include "core/g.h"
#include "core/funs.h"
#include "core/func.hpp"
#include <unistd.h>
#include <thread>

UDPServer::UDPServer(Address l, Address r, std::shared_ptr<Privacy> &i,
                     std::vector<unsigned char> &v) : local(l), remote(r), I(i), key(v), stop(false)
{
}

UDPServer::~UDPServer()
{
}

void UDPServer::stop_server()
{
    stop = true;
}

struct SocketHandle
{
    explicit SocketHandle(int f):fd(f)
    {        
    }

    operator int (){
        return fd;
    }

    ~SocketHandle()
    {
        close(fd);
    }

private:
    int fd;
};

static int udp_one_time_routine(std::shared_ptr<unsigned char> spRead,
                                ssize_t n,
                                const Address &target,
                                std::shared_ptr<sockaddr_in> src,
                                socklen_t src_len,
                                const std::shared_ptr<Privacy> &I,
                                const std::vector<unsigned char> &key,
                                int fd)
{
    SocketHandle r(SocketAssistant::create_udp_connect_socket(target));
    if (r < 0)
    {
        LOG(ERROR) << "create proxy socket failed: " << (int)(r) << std::endl;
        return -1;
    }

    struct timeval timeout2 = {60, 0}; //60s
    setsockopt(r, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout2, sizeof(timeout2));
    setsockopt(r, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout2, sizeof(timeout2));

    std::vector<unsigned char> out;
    int zn = I->Compress(std::vector<unsigned char>(spRead.get(), spRead.get() + n), key, out);
    if (zn != 0)
    {        
        LOG(ERROR) << "Compress failed" << n << std::endl;
        return -1;
    }

    auto buffer = spRead.get();
    auto bytes = I->ToBytes();
    buffer[4] = bytes.size();
    std::copy(bytes.begin(), bytes.end(), buffer + 5);
    std::copy(out.begin(), out.end(), buffer + 5 + bytes.size());
    size_t total = 1 + bytes.size() + out.size();
    auto sz = Funcs::ToBytes(total);
    std::copy(sz.begin(), sz.end(), buffer);

    auto dest = SocketAssistant::create_udp_connect_addr(target);
    if (!dest.has_value())
    {
        LOG(ERROR) << "remote address build failed!!" << n << std::endl;
        return -2;
    }

    sockaddr_in v = dest.value();
    socklen_t client_len = sizeof(v);

    n = sendto(r, buffer, total + 4, 0, (sockaddr *)&v, client_len);
    if (n < 1)
    {
        LOG(ERROR) << "send to failed" << n << std::endl;
        return -3;
    }

    LOG(INFO) << "SENT from remote server  " << n << " Bytes" << std::endl;

    n = recvfrom(r, buffer, UDP_READ_SIZE, 0, (sockaddr *)&v, &client_len);
    if (n < 1)
    {
        LOG(ERROR) << "recvfrom to failed from remote proxy :" << n << std::endl;
        return -4;
    }

    auto ll = Funcs::ToInt(buffer);
    if (n - 4 != ll)
    {
        LOG(ERROR) << "LENGTH is incorrect." << n - 4 << "----" << ll << std::endl;
        return -5;

    }
    LOG(INFO) << "content length: " << ll << " C: " << ToHexEX(buffer, buffer + ll + 4) << std::endl;
    auto kk = buffer[4];
    auto zip = PrivacyBase::build_privacy_method(std::vector<unsigned char>(buffer + 5, buffer + 5 + kk));
    if (!zip.has_value())
    {
        LOG(ERROR) << "I compress method failed" << std::endl;
        return -6;
    }

    auto start = buffer + 5 + kk;
    ll = ll - kk - 1;

    std::vector<unsigned char> in = std::vector<unsigned char>(start, start + ll);

    auto o = zip.value()->UnCompress(in, key, out);
    if (o != 0)
    {
        LOG(ERROR) << "I compress method failed" << std::endl;
        return -7;
    }

    LOG(INFO) << "Anwser " << out.size() << std::endl;
    LOG(INFO) << "socket is " <<  fd << std::endl;


    n = sendto(fd, out.data(), out.size(), 0, (struct sockaddr *)src.get(), src_len);
    if (n < 0)
    {
        LOG(ERROR) << " send to failed : " << n << " error: " << errno << std::endl;
    }

    return 0;
}

int UDPServer::Run()
{
    int fd = SocketAssistant::create_udp_listening_socket(this->local);
    if (fd < 0)
    {
        LOG(ERROR) << "create udp listening socket failed" << fd << std::endl;
        return -1;
    }
    LOG(INFO) << "socket handle is: " << fd << std::endl;

    struct timeval timeout = {60, 0}; //60s
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(timeout));
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));

    for (;;)
    {
        if (stop)
        {
            break;
        }

        std::shared_ptr<unsigned char> spRead(new unsigned char[BUFFER_UDP_SIZE], [](unsigned char *p) {
            delete[] p;
        });
        std::shared_ptr<sockaddr_in> src = std::make_shared<sockaddr_in>();
        memset(src.get(),0, sizeof(*src.get()));
        socklen_t client_address_len = sizeof(sockaddr_in);

        ssize_t n = recvfrom(fd, spRead.get(), UDP_READ_SIZE,
                             MSG_WAITALL, (struct sockaddr *)src.get(),
                             &client_address_len);
        if (n < 1)
        {
            LOG(ERROR) << "recv failed " << n << " Error: " << errno << std::endl;
            continue;
        }

        LOG(INFO) << "RECV From local user " << n << " Bytes" << std::endl;
        std::thread ro(udp_one_time_routine, spRead, n, this->remote,
                       src, client_address_len, this->I, this->key, fd);
        ro.detach();
    }

    close(fd);

    return 0;
}
