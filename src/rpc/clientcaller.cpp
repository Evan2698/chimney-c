#include "rpc/clientcaller.h"
#include <array>
#include "unistd.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "core/g.h"
#include "rpc/callee.h"
#include "core/socketassistant.h"

static int convert_result(const std::array<unsigned char, 8> &res)
{
    unsigned int c = res[0];
    c = (c << 8) | res[1];
    c = (c << 8) | res[2];
    c = (c << 8) | res[3];
    return (int)c;
}

static std::array<unsigned char, 4> convert_param(int fd)
{
    unsigned int k = fd;
    std::array<unsigned char, 4> out;
    out[3] = k & 0xff;
    out[2] = (k >> 8) & 0xff;
    out[1] = (k >> 16) & 0xff;
    out[0] = (k >> 24) & 0xff;
    return std::move(out);
}

client_caller::client_caller() : callee(0)
{
}

client_caller::~client_caller()
{
    close(callee);
}

int client_caller::try_to_connect_callee()
{
    auto it = callee::get_instance().is_valid();
    if (!it)
    {
        return -1;
    }

    Address a(callee::get_instance().get_instance().get_callee(),
              callee::get_instance().get_instance().get_port(), Address::ipv4);
    for (int i = 0; i < 3; i++)
    {
        auto fd = SocketAssistant::create_socket(a, "tcp");
        if (fd > 0)
        {
            callee = fd;
            break;
        }
    }

    return (callee > 0);
}

int client_caller::call_protect_socket(int fd)
{
    if (!callee::get_instance().is_valid())
    {
        return -1;
    }

    std::lock_guard<std::mutex> lock(this->bequeue);

    if (this->callee <= 0)
    {
        this->callee = try_to_connect_callee();
    }

    if (this->callee <= 0)
    {
        return -1;
    }

    auto param = convert_param(fd);
    auto r = send(callee, param.data(), param.size(), 0);
    if (r <= 0)
    {
        close(callee);
        try_to_connect_callee();
        LOG(ERROR) << "call callee server failed!!! " << r << std::endl;
        return -1;
    }

    std::array<unsigned char, 8> result;

    r = read(callee, result.data(), result.size());
    if (r <= 0)
    {
        close(callee);
        try_to_connect_callee();
        LOG(ERROR) << "read callee server failed!!! " << r << std::endl;
        return -2;
    }
    r = convert_result(result);

    LOG(INFO) << "Success Value" << r << std::endl;

    return r;
}