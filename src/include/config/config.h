#ifndef CONFIG_H_39984444444
#define CONFIG_H_39984444444
#include <string>
struct client
{
    std::string server;
    unsigned short  server_port;

    std::string local;
    unsigned short local_port;

    unsigned short udp_port;

    unsigned short local_udp;

    std::string methodName;

    std::string network;   

    std::string pwd;

    unsigned int timeout;
};


#endif 