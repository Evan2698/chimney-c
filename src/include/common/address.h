/*******************************************************************************
 *
 * socks5 address
 * A C++11 socks5 address
 *
 * 
 *
 ******************************************************************************/

#ifndef ADDRESS_H
#define ADDRESS_H

#include <netinet/in.h>
#include <stdint.h>

#include <array>
#include <iostream>
#include <string>
#include <vector>
#include <memory>

class Address
{
public:
    enum Type { ipv4, ipv6, domain, unknown };

    // Construct an invalid address
    Address();
    
    explicit Address(struct sockaddr *address);

    /**
       Constructor for IPv4 Raw Address,
       both host and port are in network byte orders
    **/
    Address(const std::array<unsigned char, 4> &host, unsigned short port);

    /**
       Constructor for IPv6 Raw Address,
       both host and port are in network byte orders
    **/
    Address(const std::array<unsigned char, 16> &host, unsigned short port);
    
    /** 
        Constructor for domain name,
        port in network byte orders
     **/
    Address(const std::string &domain, unsigned short port);



     Address(const std::string h, unsigned short port, Type t);

    /**
       Factory constructor,
       host can be ipv4 or ipv6 address, or domain name
       port in host byte orders
    **/
    static Address FromHostOrder(const std::string &host, unsigned short port);
    
    // Return ip address or domain name
    std::string host() const;

    // Return port in host byte order
    std::uint16_t port() const;

    std::string portString() const;
    
    // Return string representation of host and port
    std::string toString() const;

    // Return type of address
    Type type() const;

    // Whether the address is valid
    bool isValid() const;

    // Return bytes representation of IPv4 address
    std::array<unsigned char, 4> toRawIPv4() const;

    // Return bytes representation of IPv6 address    
    std::array<unsigned char, 16> toRawIPv6() const;

    // Return port in network byte order
    unsigned short portNetworkOrder() const;

    // Return bytes representation of port
    std::array<unsigned char, 2> rawPortNetworkOrder() const;

   // Return hole Address
    static std::shared_ptr<Address> FromSocks5CommandStream(const std::vector<unsigned char> &cmd);

    // Return bytes stream data
   std::vector<unsigned char> PackSocks5Address();
    
private:
    // Whether the address is IPv4 address
    static bool isIPv4(const std::string &host);

    // Whether the address is IPv6 address    
    static bool isIPv6(const std::string &host);
    
    Type          type_;
    std::string   host_;
    uint16_t      port_;
};

std::ostream &operator<<(std::ostream &os, const Address &addr);

#endif /* ADDRESS_H */
