#include "rpc/callerpool.h"
#include <mutex>

static callerpool *g_instance = nullptr;
static std::mutex mx_caller_pool;

callerpool &callerpool::get_instance()
{
    if (g_instance == nullptr)
    {
        std::lock_guard<std::mutex> lock(mx_caller_pool);
        if (g_instance == nullptr)
        {
            g_instance = new callerpool();
        }
    }
    return *g_instance;
}

callerpool::callerpool() : count(0)
{
    for (int i = 0; i < 5; i++)
    {
        auto it = new client_caller();
        it->try_to_connect_callee();
        pool.push_back(it);
    }
}

client_caller &callerpool::get_caller()
{
    int v = count % 5;

    auto tmp = pool[v];

    count++;

    count = count % 5;

    return *tmp;
}