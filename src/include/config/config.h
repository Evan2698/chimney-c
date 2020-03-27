#ifndef CONFIG_H_39984444444
#define CONFIG_H_39984444444
#include <string>
struct client
{
    std::string server;
    short  server_port;

    std::string local;
    short local_port;

    std::string network;   

    std::string pwd;

    unsigned int timeout;
};


#endif 