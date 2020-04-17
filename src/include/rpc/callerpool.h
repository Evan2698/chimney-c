#ifndef CALLER_POOL_H_K_XXX_2982615495051_3
#define CALLER_POOL_H_K_XXX_2982615495051_3
#include "rpc/clientcaller.h"
#include <vector>

struct callerpool
{
    ~callerpool() = default;
    callerpool(const callerpool &) = delete;
    callerpool(const callerpool &&) = delete;

    static callerpool &get_instance();

    client_caller &get_caller();

private:
    callerpool();

private:
    unsigned int count;
    std::vector<client_caller *> pool;
};

#endif