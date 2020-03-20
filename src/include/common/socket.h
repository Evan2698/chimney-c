/*******************************************************************************
 *
 * socket h
 * create socket for all
 *
 * Copyright 2020 Evan . All rights reserved.
 *
 ******************************************************************************/
#ifndef SOCKET_H_EVAN_321033_H
#define SOCKET_H_EVAN_321033_H
#include "address.h"
#include <string>
struct SocketBuilder
{

    static int create_socket(Address a, const std::string& network);

    static Address get_socket_local_address(int fd);

    static int create_listening_socket(Address a, const std::string &network);
};
#endif