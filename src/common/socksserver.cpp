#include "common/socksserver.h"
#include <unistd.h>
#include <fcntl.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <resolv.h>
#include "common/socket.h"
#include <glog/logging.h>

Socks5Server::Socks5Server(ClientBuilder & cbx) : s(0), cb(cbx)
{
}
Socks5Server::~Socks5Server()
{
    shutdown(s, SHUT_RDWR);
    close(s);
}

void Socks5Server::stop()
{
    this->loop.break_loop();

}

int Socks5Server::init(Address a, const std::string &network)
{
    LOG(INFO) << "listen server on: " << a.toString() << std::endl;
    s = SocketBuilder::create_listening_socket(a, network);
    if (s < 0)
    {
        LOG(ERROR) << "create listening socket failed!   " << s << std::endl;
        return -1;
    }

    auto r = listen(s, 10);
    if (r != 0)
    {
        close(s);
        s = 0;
        LOG(ERROR) << "create listening socket failed!  " << s << std::endl;
        return -1;
    }

    io.set<Socks5Server, &Socks5Server::io_accept>(this);
    io.start(s, ev::READ);

    sio.set<&Socks5Server::signal_cb>();
    sio.start(SIGINT);

    LOG(INFO) << "listen server on (success). " << a.toString() << std::endl;

    return 0;
}

void Socks5Server::run()
{
    this->loop.run();
}

void Socks5Server::signal_cb(ev::sig &signal, int revents)
{
    signal.loop.break_loop();
}

void Socks5Server::io_accept(ev::io &watcher, int revents)
{
    LOG(INFO) << "event=" << revents << std::endl;

    if (EV_ERROR & revents)
    {  
        LOG(ERROR) << "got invalid event" << revents;
        return;
    }

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int client_sd = accept(watcher.fd, (struct sockaddr *)&client_addr, &client_len);

    if (client_sd < 0)
    {
        LOG(ERROR) << "accept error" << client_sd;
        return;
    }

    cb.delegate_to_client(client_sd);
}