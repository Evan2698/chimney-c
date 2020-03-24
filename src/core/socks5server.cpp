#include "core/socks5server.h"
#include "core/socket.h"
#include "glog/logging.h"
#include "core/address.h"
#include "core/stream.h"
#include <thread>
#include "core/func.hpp"
#include "core/peerfactory.h"
#include "core/g.h"
Socks5Server::Socks5Server(Address listening) : listen_address(listening), handle(0)
{
}
Socks5Server::~Socks5Server()
{
    shutdown();
}



static int sayHello(StreamHodler &sp)
{
    std::vector<unsigned char> out(20, 0);
    int n = sp->Read(out);
    if (n <= 0)
    {
        LOG(ERROR) << "Read hello failed " << n << std::endl;
        return -1;
    }
    if (out.size() < 3)
    {
        LOG(ERROR) << "hello format is not correct!!" << n << std::endl;
        return -1;
    }

    LOG(INFO) << "client: >>> " << ToHexEX(out.begin(), out.end());

    if (out[0] != 0x5)
    {
        LOG(ERROR) << "socks5 is invalid" << n << std::endl;
        return -1;
    }
    out[0] = 0x5;
    out[1] = 0x00;
    out.resize(2);
    n = sp->Write(out);

    LOG(INFO) << "Write Hello " << n;

    return n > 0 ? 0 : -1;
}

static Stream * doCommand(const std::vector<unsigned char> &input)
{
  
    auto dest = Address::FromSocks5CommandStream(input);
    if (!dest)
    {
        LOG(INFO) << "destion address parse failed " << ToHexEX(input.begin(), input.end()) << std::endl;
        return nullptr;
    }
    return PeerFactory::get_instance()->build_peer_with_target(dest);
}

static int doConnect(StreamHodler &sp, StreamHodler &peer)
{
    std::vector<unsigned char> out(256, 0);
    int n = sp->Read(out);
    if (n <= 0)
    {
        LOG(ERROR) << "read connect command failed" << n << std::endl;
        return -1;
    }

    LOG(INFO) << "client connect : >>> " << ToHexEX(out.begin(), out.end());
    if (n < 4 || out[0] != 0x5 || out[1] != 1)
    {
        unsigned char rsu[10] = {0x05, 0x0A, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        sp->Write(std::vector<unsigned char>(std::begin(rsu), std::end(rsu)));
        LOG(ERROR) << "socks 5 command error" << std::endl;
        return -1;
    }

    auto target = doCommand(std::vector<unsigned char>(out.begin() + 3, out.end()));
    if (target == nullptr)
    {
        unsigned char rsu[10] = {0x05, 0x0B, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        sp->Write(std::vector<unsigned char>(std::begin(rsu), std::end(rsu)));
        LOG(ERROR) << "handle connect command failed" << std::endl;
        return -1;
    }

    LOG(INFO) << "bound address" << target->get_local().toString() << std::endl;

    auto ss = target->get_local().PackSocks5Address();
    if (ss.empty())
    {
        delete target;
        unsigned char rsu[10] = {0x05, 0x0C, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        sp->Write(std::vector<unsigned char>(std::begin(rsu), std::end(rsu)));
        LOG(ERROR) << "bound address format error" << std::endl;
        return -1;
    }
    n = sp->Write(ss);
    LOG(INFO) << "write bound address Reason:" << n << std::endl;
    if (0 >= n)
    {
        delete target;
        LOG(ERROR) << "shake failed!!!!";
        return -1;
    }

    peer.attach(target);

    return 0;
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

static void proxy_read(StreamHodler &src,
                       StreamHodler &dst,
                       std::shared_ptr<Privacy> &I,
                       const std::vector<unsigned char> &key)
{

    while (true)
    {
        std::vector<unsigned char> out(BUFFER_SIZE_READ, 0);
        int n = src->Read(out);
        if (n <= 0)
        {
            LOG(ERROR) << "Read failed!" << src->get_local().toString()
                       << "<--->" << src->get_remote().toString() << " " << n;
            break;
        }

        std::vector<unsigned char> tmp;
        n = I->Compress(out, key, tmp);
        if (n != 0)
        {
            LOG(ERROR) << "zip failed" << std::endl;
            break;
        }
        out.resize(tmp.size() + 4);
        auto len = ToBytes(tmp.size());
        std::copy(len.begin(), len.end(), out.begin());
        std::copy(tmp.begin(), tmp.end(), out.begin() + 4);

        n = dst->Write(out);
        if (n <= 0)
        {
            LOG(ERROR) << "write failed:" << dst->get_local().toString()
                       << "<--->" << dst->get_remote().toString() << " " << n;
            break;
        }
    }
}

static void proxy_write(StreamHodler  &src,
                        StreamHodler &dst,
                        std::shared_ptr<Privacy> &I,
                        const std::vector<unsigned char> &key)
{

    while (true)
    {
        std::vector<unsigned char> out(4, 0);
        int n = src->Read(out);
        if (n <= 0)
        {
            LOG(ERROR) << "Read  LEN failed!" << src->get_local().toString()
                       << "<--->" << src->get_remote().toString() << " " << n;
            break;
        }

        auto next = ToInt(out.data());
        if (next > BUFFER_SIZE_READ + 512)
        {
            LOG(ERROR) << "NEXT READ LEN is too big. " << next << std::endl;
            break;
        }
        out.resize(next);
        n = src->Read(out);
        if (n <= 0)
        {
            LOG(ERROR) << "Read conntent failed" << src->get_local().toString()
                       << "<--->" << src->get_remote().toString() << " " << n;
            break;
        }
        std::vector<unsigned char> tmp;
        n = I->UnCompress(out, key, tmp);
        if (n != 0)
        {
            LOG(ERROR) << "unzip failed!" << std::endl;
            ;
            break;
        }

        n = dst->Write(tmp);
        if (n <= 0)
        {
            LOG(ERROR) << "Write failed" << dst->get_local().toString()
                       << "<--->" << dst->get_remote().toString() << "  " << n;
            break;
        }
    }
}

struct write_param
{
    StreamHodler &src;
    StreamHodler &dst;
    write_param(StreamHodler &s, StreamHodler &d):
    src(s),
    dst(d)
    {

    }
};

static void help_func(write_param *p)
{
    if (p != nullptr)
    {
        proxy_write(p->src, p->dst,
                    PeerFactory::get_instance()->getMethod(),
                    PeerFactory::get_instance()->get_key());

        delete p;
    }
}

void Socks5Server::doServeOnOne(Stream *psrc)
{
    StreamHodler sp(psrc);
    StreamHodler peer;

    if (sayHello(sp) != 0)
        return;

    if (doConnect(sp, peer) != 0)
        return;

    // for
    auto pa = new write_param(peer, sp);
    std::thread rt(help_func, pa);

    proxy_read(sp, peer,
               PeerFactory::get_instance()->getMethod(),
               PeerFactory::get_instance()->get_key());
    rt.join();
    LOG(INFO) << "OVER<====================>OVER" << std::endl;
}

void Socks5Server::serve_on(Stream *sp)
{
    doServeOnOne(sp);
}

int Socks5Server::run()
{
    handle = SocketBuilder::create_listening_socket(this->listen_address, "tcp");
    if (handle == -1)
    {
        LOG(ERROR) << "create socket fd faield: " << handle << std::endl;
        return -1;
    }

    int listenR = listen(handle, 40);
    if (listenR == -1)
    {
        LOG(ERROR) << "listen fd faield: " << handle << std::endl;
        return -1;
    }

    sockaddr_storage their_addr;
    socklen_t client_addr_size = sizeof(their_addr);

    while (true)
    {
        int newFD = accept(handle, (sockaddr *)&their_addr, &client_addr_size);
        if (newFD == -1)
        {
            LOG(ERROR) << "Error while Accepting on socket\n";
            continue;
        }
        struct sockaddr_in *sin = (struct sockaddr_in *)&their_addr;
        Address a(sin);
        auto p = new Stream(newFD, a, this->listen_address);
        std::thread server(Socks5Server::serve_on, p);
        server.detach();
    }
}

void Socks5Server::shutdown()
{
    close(handle);
}