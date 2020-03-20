#include "client/target.h"

std::shared_ptr<Address> Target::getBoundAddresss()
{
    return bound;
}

Target::Target(std::shared_ptr<Address> b) : bound(b)
{
}