
#include "core/socket.h"
#include "core/address.h"
#include "core/stream.h"
#include <thread>
#include "core/func.hpp"
#include "core/peerfactory.h"
#include "core/g.h"
#include <unistd.h>
#include <netinet/in.h>
#include "core/socks5server.h"
#include "core/threadparameter.h"
#include "core/objectholder.hpp"

Socks5Server::Socks5Server(Address listening) : listen_address(listening), handle(0)
{
}
Socks5Server::~Socks5Server()
{
    shutdown();
}

static int sayHello(std::shared_ptr<Stream> &sp)
{
    std::vector<unsigned char> out(20, 0);
    int n = sp->Read(out);
    if (n <= 0)
    {
        LOG_S(ERROR) << "Read hello failed " << n << std::endl;
        return -1;
    }
    if (out.size() < 3)
    {
        LOG_S(ERROR) << "hello format is not correct!!" << n << std::endl;
        return -1;
    }

    LOG_S(INFO) << "client: >>> " << ToHexEX(out.begin(), out.end());

    if (out[0] != 0x5)
    {
        LOG_S(ERROR) << "socks5 is invalid" << n << std::endl;
        return -1;
    }
    out[0] = 0x5;
    out[1] = 0x00;
    out.resize(2);
    n = sp->Write(out);

    LOG_S(INFO) << "Write Hello " << n;

    return n > 0 ? 0 : -1;
}

static std::shared_ptr<ThreadParameter> doCommand(const std::vector<unsigned char> &input)
{

    auto target = Address::FromSocks5CommandStream(input);
    if (!target)
    {
        LOG_S(ERROR) << "destion address parse failed " << ToHexEX(input.begin(), input.end()) << std::endl;
        return nullptr;
    }

    auto peer = PeerFactory::get_instance().build_socks5_peer();
    if (!peer)
    {
        LOG_S(ERROR) << "Create Peer failed " << std::endl;
        return nullptr;
    }

    auto s = peer->build_stream(target);
    if (!s)
    {
        LOG_S(ERROR) << "create stream of peer failed " << std::endl;
        return nullptr;
    }

    ObjectHolder<ThreadParameter> holder(new ThreadParameter());
    holder->Dst = s;
    holder->I = peer->get_Method();
    holder->Key = peer->get_key();
    auto sp = std::shared_ptr<ThreadParameter>(holder.detach());
    return sp;
}

static std::shared_ptr<ThreadParameter> doConnect(std::shared_ptr<Stream> &sp)
{
    std::vector<unsigned char> out(256, 0);
    int n = sp->Read(out);
    if (n <= 0)
    {
        LOG_S(ERROR) << "read connect command failed" << n << std::endl;
        return nullptr;
    }

    LOG_S(INFO) << "client connect : >>> " << ToHexEX(out.begin(), out.end());
    if (n < 4 || out[0] != 0x5 || out[1] != 1)
    {
        unsigned char rsu[10] = {0x05, 0x0A, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        sp->Write(std::vector<unsigned char>(std::begin(rsu), std::end(rsu)));
        LOG_S(ERROR) << "socks 5 command error" << std::endl;
        return nullptr;
    }

    std::shared_ptr<ThreadParameter> parameter = doCommand(std::vector<unsigned char>(out.begin() + 3, out.end()));
    if (!parameter)
    {
        unsigned char rsu[10] = {0x05, 0x0B, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        sp->Write(std::vector<unsigned char>(std::begin(rsu), std::end(rsu)));
        LOG_S(ERROR) << "handle connect command failed" << std::endl;
        return nullptr;
    }

    LOG_S(INFO) << "bound address" << parameter->Dst->get_local().toString() << std::endl;

    auto ss = parameter->Dst->get_local().PackSocks5Address();
    if (ss.empty())
    {
        unsigned char rsu[10] = {0x05, 0x0C, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        sp->Write(std::vector<unsigned char>(std::begin(rsu), std::end(rsu)));
        LOG_S(ERROR) << "bound address format error" << std::endl;
        return nullptr;
    }
    n = sp->Write(ss);
    LOG_S(INFO) << "write bound address Reason:" << n << std::endl;
    if (0 >= n)
    {
        LOG_S(ERROR) << "shake failed!!!!";
        return nullptr;
    }

    parameter->Src = sp;

    return parameter;
}

static unsigned int ToInt(unsigned char *sz)
{
    unsigned hi = sz[0];
    hi = hi << 24;
    hi = hi | (((unsigned)sz[1]) << 16);
    hi = hi | (((unsigned)sz[2]) << 8);
    hi = hi | sz[3];

    return hi;
}

static std::vector<unsigned char> ToBytes(unsigned int v)
{
    std::vector<unsigned char> nn(4, 0);
    nn[3] = v & 0xff;
    nn[2] = (v >> 8) & 0xff;
    nn[1] = (v >> 16) & 0xff;
    nn[0] = (v >> 24) & 0xff;
    return nn;
}

static void proxy_read(std::shared_ptr<ThreadParameter> & param)
{
    auto src = param->Src;
    auto key = param->Key;
    auto dst = param->Dst;
    auto I = param->I;

    while (true)
    {
        std::vector<unsigned char> out(BUFFER_SIZE_READ, 0);
        int n = src->Read(out);
        if (n <= 0)
        {
            LOG_S(ERROR) << "Read failed!" << src->get_local().toString()
                         << "<--->" << src->get_remote().toString() << " " << n;
            break;
        }

        std::vector<unsigned char> tmp;
        n = I->Compress(out, key, tmp);
        if (n != 0)
        {
            LOG_S(ERROR) << "zip failed" << std::endl;
            break;
        }
        out.resize(tmp.size() + 4);
        auto len = ToBytes(tmp.size());
        std::copy(len.begin(), len.end(), out.begin());
        std::copy(tmp.begin(), tmp.end(), out.begin() + 4);

        n = dst->Write(out);
        if (n <= 0)
        {
            LOG_S(ERROR) << "write failed:" << dst->get_local().toString()
                         << "<--->" << dst->get_remote().toString() << " " << n;
            break;
        }
    }
}

static void proxy_write(std::shared_ptr<ThreadParameter> & param)
{
    auto src = param->Dst;
    auto key = param->Key;
    auto dst = param->Src;
    auto I = param->I;

    while (true)
    {
        std::vector<unsigned char> out(4, 0);
        int n = src->Read(out);
        if (n <= 0)
        {
            LOG_S(ERROR) << "Read  LEN failed!" << src->get_local().toString()
                         << "<--->" << src->get_remote().toString() << " " << n;
            break;
        }

        auto next = ToInt(out.data());
        if (next > BUFFER_SIZE_READ + 512)
        {
            LOG_S(ERROR) << "NEXT READ LEN is too big. " << next << std::endl;
            break;
        }
        out.resize(next);
        n = src->Read(out);
        if (n <= 0)
        {
            LOG_S(ERROR) << "Read conntent failed" << src->get_local().toString()
                         << "<--->" << src->get_remote().toString() << " " << n;
            break;
        }
        std::vector<unsigned char> tmp;
        n = I->UnCompress(out, key, tmp);
        if (n != 0)
        {
            LOG_S(ERROR) << "unzip failed!" << std::endl;
            ;
            break;
        }

        n = dst->Write(tmp);
        if (n <= 0)
        {
            LOG_S(ERROR) << "Write failed" << dst->get_local().toString()
                         << "<--->" << dst->get_remote().toString() << "  " << n;
            break;
        }
    }
}



void Socks5Server::doServeOnOne(std::shared_ptr<Stream> src)
{
    if (sayHello(src) != 0)
        return;

    auto param = doConnect(src);
    if (!param )
    {
        LOG_S(ERROR) << "Prepare thread paramter failed!!" << std::endl;
        return;
    } 

    if (!param->isValid()){
        LOG_S(ERROR) << "thread paramter is invalid!" << std::endl;
        return;
    }
  
    std::thread xox([&param](){
         proxy_write(param);
    });

    proxy_read(param);
    xox.join();
    LOG_S(INFO) << "OVER<====================>OVER" << std::endl;
}

int Socks5Server::run()
{
    handle = SocketBuilder::create_listening_socket(this->listen_address, "tcp");
    if (handle == -1)
    {
        LOG_S(ERROR) << "create socket fd faield: " << handle << std::endl;
        return -1;
    }

    int listenR = listen(handle, 40);
    if (listenR == -1)
    {
        LOG_S(ERROR) << "listen fd faield: " << handle << std::endl;
        return -1;
    }

    sockaddr_storage their_addr;
    socklen_t client_addr_size = sizeof(their_addr);

    while (true)
    {
        int newFD = accept(handle, (sockaddr *)&their_addr, &client_addr_size);
        if (newFD == -1)
        {
            LOG_S(ERROR) << "Error while Accepting on socket\n";
            break;
        }
        Address R((struct sockaddr_in *)&their_addr);
        this->build_service_routine(newFD, R);
    }

    return 0;
}

void Socks5Server::build_service_routine(int fd, Address &R)
{
    auto out = std::make_shared<Stream>(fd, listen_address, R);
    std::thread wh([out]() {
        doServeOnOne(out);
    });
    wh.detach();
}

void Socks5Server::shutdown()
{
    close(handle);
}