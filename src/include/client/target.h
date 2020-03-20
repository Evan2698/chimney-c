/*******************************************************************************
 *
* target handle for socket
 * 
 *
 * 
 *
 ******************************************************************************/
#ifndef TARGET_3938238732732648_H
#define TARGET_3938238732732648_H
#include "common/address.h"

struct Target
{
    Target(std::shared_ptr<Address> b);

    std::shared_ptr<Address> getBoundAddresss();

private:
    std::shared_ptr<Address> bound;
};

#endif