/*******************************************************************************
 *
 * socket h
 * create socket for all
 *
 * Copyright 2020 Evan . All rights reserved.
 *
 ******************************************************************************/
#ifndef CLIENT_BUILDER_H
#define CLIENT_BUILDER_H
#include "common/Protocol.h"

class ClientBuilder
{
private:
    /* data */
public:
    ClientBuilder(/* args */) = default;
    virtual ~ClientBuilder() = default;

    virtual std::shared_ptr<Protocol> build_client() = 0;
};

#endif 