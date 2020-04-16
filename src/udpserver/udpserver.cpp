#include "udpserver/udpserver.h"
#include "core/socketassistant.h"
#include "core/g.h"
#include "core/funs.h"
#include "core/func.hpp"
#include <unistd.h>

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

int UDPServer::Run()
{
    int fd = SocketAssistant::create_udp_listening_socket(this->local);
    if (fd < 0)
    {
        LOG(ERROR) << "create udp listening socket failed" << fd << std::endl;
        return -1;
    }

    struct timeval timeout = {10, 0}; //600ms
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(timeout));
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));

    auto r = SocketAssistant::create_udp_connect_socket(this->remote);
    if (r < 0)
    {
        LOG(ERROR) << "create proxy socket failed: " << r << std::endl;
        return -1;
    }

    setsockopt(r, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(timeout));
    setsockopt(r, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
    auto buffer = new unsigned char[BUFFER_UDP_SIZE];

    for (;;)
    {
        struct sockaddr_in client_address = {0};
        socklen_t client_address_len = sizeof(sockaddr_in);

        if (stop)
        {
            break;
        }

        auto pRead = buffer + 1024;
        ssize_t n = recvfrom(fd, pRead, UDP_READ_SIZE,
                             MSG_WAITALL, (struct sockaddr *)&client_address,
                             &client_address_len);
        if (n < 1)
        {
            LOG(ERROR) << "recv failed" << n << std::endl;
            continue;
        }
        std::vector<unsigned char> out;
        int zn = this->I->Compress(std::vector<unsigned char>(pRead, pRead + n), this->key, out);
        if (zn != 0)
        {
            LOG(ERROR) << "Compress failed" << n << std::endl;
            continue;
        }

        auto bytes = this->I->ToBytes();
        buffer[4] = bytes.size();
        std::copy(bytes.begin(), bytes.end(), buffer + 5);
        std::copy(out.begin(), out.end(), buffer + 5 + bytes.size());
        size_t total = 1 + bytes.size() + out.size();
        auto sz = Funcs::ToBytes(total);
        std::copy(sz.begin(), sz.end(), buffer);

        auto target = SocketAssistant::create_udp_connect_addr(this->remote);
        if (!target.has_value())
        {
            LOG(ERROR) << "remote address build failed!!" << n << std::endl;
            continue;
        }

        sockaddr_in v = target.value();
        socklen_t client_len = sizeof(v);

        n = sendto(r, buffer, total + 4, 0, (sockaddr *)&v, client_len);
        if (n < 1)
        {
            LOG(ERROR) << "send to failed" << n << std::endl;
            continue;
        }

        n = recvfrom(r, buffer, UDP_READ_SIZE, 0, (sockaddr *)&v, &client_len);
        if (n < 1)
        {
            LOG(ERROR) << "recvfrom to failed" << n << std::endl;
            continue;
        }

        auto ll = Funcs::ToInt(buffer);
        if (n - 4 != ll)
        {
            LOG(ERROR) << "LENGTH is incorrect." << n - 4 << "----" << ll << std::endl;
            continue;
        }
        LOG(INFO) << "content length: " << ll << " C: " << ToHexEX(buffer, buffer + ll + 4) << std::endl;
        auto kk = buffer[4];
        auto zip = PrivacyBase::build_privacy_method(std::vector<unsigned char>(buffer + 5, buffer + 5 + kk));
        if (!zip.has_value())
        {
            LOG(ERROR) << "I compress method failed" << std::endl;
            continue;
        }

        auto start = buffer + 5 + kk;
        ll = ll - kk - 1;

        std::vector<unsigned char> in = std::vector<unsigned char>(start, start + ll);

        auto o = zip.value()->UnCompress(in, this->key, out);
        if (o != 0)
        {
            LOG(ERROR) << "I compress method failed" << std::endl;
            continue;
        }
        LOG(INFO) << "Anwser " << ToHexEX(out.begin(), out.end()) << std::endl;
        n = sendto(fd, out.data(), out.size(), 0, (struct sockaddr *)&client_address, client_address_len);
        if (n < 0)
        {
            LOG(ERROR) << " send to failed : " << n << std::endl;
        }
    }

    delete[] buffer;
    close(fd);

    return 0;
}
