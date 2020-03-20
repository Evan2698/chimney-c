/*******************************************************************************
 *
 * client handle for socket
 * 
 *
 * 
 *
 ******************************************************************************/
#ifndef CLIENT_HANDLE_3049238948_H
#define CLIENT_HANDLE_3049238948_H
#include <memory>
#include "common/buffer.h"
#include "common/address.h"
#include <ev++.h>
#include "client/target.h"
#include "common/iowriter.h"
#include "privacy/privacy.h"
#include <optional>

struct ClientHandle : public IOWriter
{
    ClientHandle(Address a,
                 std::vector<unsigned char> usr,
                 std::vector<unsigned char> pwd,
                 std::vector<unsigned char> ky);
    ~ClientHandle();

    std::optional<std::shared_ptr<Target>> do_connect(Address &dest);

    static unsigned int ToInt(unsigned char *sz);
    static std::vector<unsigned char> ToBytes(unsigned int v);
    virtual int Write(std::shared_ptr<std::vector<unsigned char>> src);

    void register_io(IOWriter *p);

private:
    bool init(int fd);

    void callback(ev::io &watcher, int revents);
    void write_cb(ev::io &watcher);
    void read_cb(ev::io &watcher);

private:
    std::shared_ptr<Buffer> wq;
    std::vector<unsigned char> user;
    std::vector<unsigned char> pass;
    std::vector<unsigned char> key;
    int sfd;
    ev::io io;
    Address remote;
    std::shared_ptr<Privacy> method;
    IOWriter *writer;
};

#endif