/*******************************************************************************
 *
* client handle for socket
 * 
 *
 * 
 *
 ******************************************************************************/
#ifndef SERVER_STREAM_39848383274_H
#define SERVER_STREAM_39848383274_H
#include <memory>
#include "common/buffer.h"
#include <ev++.h>
#include <atomic>
#include "target.h"
#include "common/iowriter.h"
#include "client/clienthandle.h"

struct ServerStream : public IOWriter
{
    ServerStream();
    virtual ~ServerStream();

    bool init(int fd);
    int Write(std::shared_ptr<std::vector<unsigned char>> src);
    void set_client(std::shared_ptr<ClientHandle> c);

private:
    bool hand_shake_end(int fd);
    void callback(ev::io &watcher, int revents);
    void write_cb(ev::io &watcher);
    void read_cb(ev::io &watcher);

private:
    std::shared_ptr<Target> handleCommand(const std::vector<unsigned char> &cmd);

private:
    std::shared_ptr<Buffer> wq;  
    int sfd;
    ev::io io;
    static std::atomic<long long> clientCount;
    std::mutex _mutex;
    std::vector<unsigned char> readBuffer;
    std::shared_ptr<ClientHandle> client;
};

#endif