#include "rpc/callee.h"
#include <mutex>

callee::callee() : port(0)
{
}

static callee *g_instance = nullptr;
static std::mutex mx;
callee &callee::get_instance()
{
    if (g_instance == nullptr){
        std::lock_guard<std::mutex> lock(mx);
        if (g_instance == nullptr){
            g_instance = new callee();
        }
    }
    return *g_instance;
}

 void callee::set_callee(const std::string & host, unsigned short p)
 {
     this->port = p;
     this->callee_server = host;
 }

bool callee::is_valid()
{
    return (port != 0 && !callee_server.empty());
}

const std::string &callee::get_callee()
{
    return this->callee_server;
}
const unsigned short callee::get_port()
{
    return this->port;
}