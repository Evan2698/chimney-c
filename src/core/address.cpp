/*******************************************************************************
 *
 * socks5
 * A C++11 socks5 proxy server based on Libevent 
 *
 * Copyright 2018 Senlin Zhan. All rights reserved.
 *
 ******************************************************************************/

#include "core/address.h"

#include <assert.h>
#include <arpa/inet.h>
#include <string.h>
#include <glog/logging.h>
#include "core/func.hpp"

Address::Address()
    : type_(Type::unknown)
{
}

Address::Address(struct sockaddr_in *address) : type_(Type::unknown)
{
    if (address->sin_family == AF_INET)
    {
        host_.resize(INET_ADDRSTRLEN, '\0');
        if (::inet_ntop(AF_INET, &address->sin_addr, &host_[0], host_.size()) != nullptr)
        {
            type_ = Type::ipv4;
            port_ = ntohs(address->sin_port);
        }
    }
    else if (address->sin_family == AF_INET6)
    {
        auto sin = reinterpret_cast<sockaddr_in6 *>(address);
        host_.resize(INET6_ADDRSTRLEN, '\0');
        if (::inet_ntop(AF_INET6, &sin->sin6_addr, &host_[0], host_.size()) != nullptr)
        {
            type_ = Type::ipv6;
            port_ = ntohs(sin->sin6_port);
        }
    }
}

Address::Address(struct sockaddr *address)
    : type_(Type::unknown)
{
    if (address->sa_family == AF_INET)
    {
        auto sin = reinterpret_cast<sockaddr_in *>(address);
        host_.resize(INET_ADDRSTRLEN, '\0');

        if (::inet_ntop(AF_INET, &sin->sin_addr, &host_[0], host_.size()) != nullptr)
        {
            type_ = Type::ipv4;
            port_ = ntohs(sin->sin_port);
        }
    }
    else if (address->sa_family == AF_INET6)
    {
        auto sin = reinterpret_cast<sockaddr_in6 *>(address);
        host_.resize(INET6_ADDRSTRLEN, '\0');

        if (::inet_ntop(AF_INET6, &sin->sin6_addr, &host_[0], host_.size()) != nullptr)
        {
            type_ = Type::ipv6;
            port_ = ntohs(sin->sin6_port);
        }
    }
}

Address::Address(const std::array<unsigned char, 4> &host, unsigned short port)
    : type_(Type::unknown),
      host_(INET_ADDRSTRLEN, '\0')
{
    if (::inet_ntop(AF_INET, host.data(), &host_[0], host_.size()) != nullptr)
    {
        type_ = Type::ipv4;
        port_ = ntohs(port);
    }
}

Address::Address(const Address &other)
{
    this->host_ = other.host_;
    this->port_ = other.port_;
    this->type_ = other.type_;
}

Address::Address(const std::array<unsigned char, 16> &host, unsigned short port)
    : type_(Type::unknown),
      host_(INET6_ADDRSTRLEN, '\0')
{
    if (::inet_ntop(AF_INET6, host.data(), &host_[0], host_.size()) != nullptr)
    {
        type_ = Type::ipv6;
        port_ = ntohs(port);
    }
}

Address::Address(const std::string &domain, unsigned short port)
    : type_(Type::domain),
      host_(domain)
{
    port_ = ntohs(port);
}

Address::Address(const std::string h, unsigned short port, Type t) : type_(t),
                                                                     host_(h),
                                                                     port_(port)
{
}

Address Address::FromHostOrder(const std::string &host, unsigned short port)
{
    // TODO: check validation of domain name

    Address address;

    if (isIPv4(host))
    {
        address.type_ = Type::ipv4;
    }
    else if (isIPv6(host))
    {
        address.type_ = Type::ipv6;
    }
    else
    {
        address.type_ = Type::domain;
    }

    address.host_ = host;
    address.port_ = port;

    return address;
}

std::string Address::host() const
{
    return host_;
}

uint16_t Address::port() const
{
    return port_;
}

std::string Address::portString() const
{
    return std::to_string(port_);
}

std::string Address::toString() const
{
    return host_ + ":" + std::to_string(port_);
}

Address::Type Address::type() const
{
    return type_;
}

std::ostream &operator<<(std::ostream &os, const Address &addr)
{
    os << addr.toString();
    return os;
}

bool Address::isValid() const
{
    return type_ != Type::unknown;
}

std::array<unsigned char, 4> Address::toRawIPv4() const
{
    assert(type_ == Type::ipv4);

    std::array<unsigned char, 4> address;
    ::inet_pton(AF_INET, host_.c_str(), address.data());

    return address;
}

std::array<unsigned char, 16> Address::toRawIPv6() const
{
    assert(type_ == Type::ipv6);

    std::array<unsigned char, 16> address;
    ::inet_pton(AF_INET6, host_.c_str(), address.data());

    return address;
}

unsigned short Address::portNetworkOrder() const
{
    return htons(port_);
}

std::array<unsigned char, 2> Address::rawPortNetworkOrder() const
{
    std::array<unsigned char, 2> result;

    auto networkOrder = portNetworkOrder();
    memcpy(result.data(), &networkOrder, 2);

    return result;
}

bool Address::isIPv4(const std::string &host)
{
    std::array<unsigned char, 4> address;
    return ::inet_pton(AF_INET, host.c_str(), address.data()) == 1;
}

bool Address::isIPv6(const std::string &host)
{
    std::array<unsigned char, 16> address;
    return ::inet_pton(AF_INET6, host.c_str(), address.data()) == 1;
}

//	socks5AddressIPV4   uint8 = 0x1
//	socks5AddressIPV6   uint8 = 0x4
//	socks5AddressDomain uint8 = 0x3
std::shared_ptr<Address> Address::FromSocks5CommandStream(const std::vector<unsigned char> &cmd)
{
    LOG(INFO) << "FromSocks5CommandStream  " << ToHexEX(cmd.begin(), cmd.end());
    auto address = std::make_shared<Address>();
    short a = cmd[cmd.size() - 2];
    a = a << 8 | cmd[cmd.size() - 1];

    if (0x1 == cmd[0])
    {
        //ipv4
        std::vector<unsigned char> host(cmd.begin() + 1, cmd.begin() + 5);
        address->host_.resize(INET_ADDRSTRLEN, '\0');
        if (::inet_ntop(AF_INET, host.data(), &address->host_[0], INET_ADDRSTRLEN) != nullptr)
        {
            address->type_ = Type::ipv4;
            address->port_ = a;
        }
    }
    else if (0x3 == cmd[0])
    {
        // domain
        address->type_ = Type::domain;
        address->port_ = a;
        if (cmd.begin() + cmd[1] < cmd.end())
        {
            address->host_ = std::string(cmd.begin() + 2, cmd.begin() + cmd[1]);
        }
    }
    else if (0x4 == cmd[0])
    {
        std::vector<unsigned char> host(cmd.begin() + 1, cmd.begin() + cmd.size() - 2);
        address->host_.resize(INET6_ADDRSTRLEN, '\0');
        if (::inet_ntop(AF_INET, host.data(), &address->host_[0], INET_ADDRSTRLEN) != nullptr)
        {
            address->type_ = Type::ipv6;
            address->port_ = a;
        }
    }

    return address;
}

std::vector<unsigned char> Address::PackSocks5Address() const
{
    std::vector<unsigned char> out;

    if (!isValid())
    {
        return std::move(out);
    }

    out.push_back(0x5);
    out.push_back(0x00); //successed
    out.push_back(0x00); // reserved

    unsigned char low = (port_ & 0xff);
    unsigned char high = (port_ >> 8) & 0xff;

    char o = 0x1;
    if (type() == Address::Type::ipv6)
    {
        o = 0x4;
        out.push_back(o);
        auto ip = toRawIPv6();
        for (auto a : ip)
        {
            out.push_back(a);
        }
    }
    else if (type() == Address::Type::domain)
    {
        o = 0x3;
        out.push_back(o);
        unsigned char l = host_.size();
        out.push_back(l);
        for (auto a : host_)
        {
            out.push_back(a);
        }
    }
    else if (type() == Address::Type::ipv4)
    {
        o = 0x1;
        out.push_back(o);
        auto ip = toRawIPv4();
        for (auto a : ip)
        {
            out.push_back(a);
        }
    }
    out.push_back(high);
    out.push_back(low);

    return std::move(out);
}
