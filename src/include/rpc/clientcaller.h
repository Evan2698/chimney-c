#ifndef CLIENT_CALLER_H_3984837_2384874
#define CLIENT_CALLER_H_3984837_2384874

#include <mutex>

struct callerpool;
struct client_caller
{
    client_caller();
    ~client_caller();

    int call_protect_socket(int fd);

private:
    int try_to_connect_callee();
    friend struct callerpool;
private:
    int callee;
    std::mutex  bequeue;
};

#endif