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

class ClientBuilder
{
private:
    /* data */
public:
    ClientBuilder(/* args */) = default;
    virtual ~ClientBuilder() = default;

    virtual int delegate_to_client(int fd) = 0;
};

#endif 