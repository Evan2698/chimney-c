#include "rpc/callee.h"

callee::callee() : port(0)
{
}

callee &callee::get_instance()
{
    static callee lle;
    return lle;
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