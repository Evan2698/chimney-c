#include "rpc/callerpool.h"

callerpool &callerpool::get_instance()
{
    static callerpool instance;

    return instance;

}

callerpool::callerpool():count(0)
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
    int v = count  % 5 ;

    auto tmp = pool[v];

    count++;

    count = count % 5;

    return *tmp;
}