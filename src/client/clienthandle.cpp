#include "client/clienthandle.h"
#include "common/socket.h"
#include <glog/logging.h>
#include "privacy/privacy.h"
#include <fcntl.h>
#include "common/func.hpp"

static const int stack_buffer_size = 512;
std::optional<std::shared_ptr<Target>> ClientHandle::do_connect(Address &dest)
{

    LOG(INFO) << "remote address: " << this->remote.toString() << std::endl;
    int s = SocketBuilder::create_socket(this->remote, "tcp");
    if (-1 == s)
    {
        LOG(ERROR) << "create socket failed: " << s << std::endl;
        return std::nullopt;
    }

    LOG(INFO) << "connect socket create successed!   " << s << std::endl;

    //say hello
    unsigned char szBuffer[stack_buffer_size] = {0x5, 0x1, 0x2};
    ssize_t written = write(s, szBuffer, 3);
    if (written < 0)
    {
        close(s);
        LOG(ERROR) << "say hello failed " << written << "--" << s << std::endl;
        return std::nullopt;
    }
    // recv hello
    memset(szBuffer, 0, sizeof szBuffer);
    ssize_t nread = recv(s, szBuffer, sizeof(szBuffer), 0);
    if (nread < 0)
    {
        close(s);
        LOG(ERROR) << "read hello response failed " << nread << "--" << s << std::endl;
        return std::nullopt;
    }

    if (szBuffer[0] != 0x5 || szBuffer[1] != 2 || szBuffer[2] > 250)
    {
        close(s);
        LOG(ERROR) << "server response format error " << szBuffer[0] << szBuffer[1] << szBuffer[2] << "--" << s << std::endl;
        return std::nullopt;
    }
    LOG(INFO) << "METHOD : " << ToHexEX(szBuffer, szBuffer + std::min<ssize_t>(nread, sizeof szBuffer));
    auto start = szBuffer + 3;
    auto end = start + szBuffer[2];
    std::vector<unsigned char> ls(start, end);
    auto sp = build_privacy_method(ls);
    if (!sp.has_value())
    {
        close(s);
        LOG(ERROR) << "parse method faield " << ToHexEX(ls.begin(), ls.end()) << s << std::endl;
        return std::nullopt;
    }
    this->method = sp.value();

    // user and pass
    std::vector<unsigned char> out;
    auto ret = this->method->Compress(this->pass, this->key, out);
    if (ret != 0)
    {
        close(s);
        LOG(ERROR) << "Compressed failed " << std::endl;
        return std::nullopt;
    }

    auto wcnt = 2 + this->user.size() + 1 + out.size();
    if (wcnt > sizeof(szBuffer))
    {
        close(s);
        LOG(ERROR) << "Buffer too small for user & pwd!!! " << std::endl;
        return std::nullopt;
    }

    szBuffer[0] = 0x5;
    szBuffer[1] = this->user.size();
    std::copy(this->user.begin(), this->user.end(), szBuffer + 2);
    szBuffer[2 + this->user.size()] = out.size();
    std::copy(out.begin(), out.end(), szBuffer + 2 + this->user.size() + 1);

    written = write(s, szBuffer, wcnt);
    if (written <= 0)
    {
        close(s);
        LOG(ERROR) << "user and pass " << written << std::endl;
        return std::nullopt;
    }

    memset(szBuffer, 0, sizeof szBuffer);
    nread = recv(s, szBuffer, sizeof(szBuffer), 0);
    if (nread <= 0)
    {
        close(s);
        LOG(ERROR) << "user and pass read response failed " << nread << std::endl;
        return std::nullopt;
    }

    if (szBuffer[0] != 5 || szBuffer[1] != 0)
    {
        close(s);
        LOG(ERROR) << "verify failed!! " << ToHexEX(szBuffer, szBuffer + std::min<ssize_t>(nread, 2)) << std::endl;
        return std::nullopt;
    }

    // connect dest
    //op.Write([]byte{socks5Version, socks5CMDConnect, 0x00, rawAddr.GetAddressType()})
    auto bytes = dest.PackSocks5Address();
    std::copy(bytes.begin(), bytes.begin() + 4, szBuffer);
    szBuffer[1] = 0x1; //Connect

    auto tmp = std::vector<unsigned char>(bytes.begin() + 4, bytes.end());
    ret = this->method->Compress(tmp, this->key, out);
    if (ret != 0)
    {
        close(s);
        LOG(ERROR) << "Compress destination address failed! " << std::endl;
        return std::nullopt;
    }
    wcnt = 4 + 1 + out.size();
    if (wcnt > sizeof szBuffer)
    {
        close(s);
        LOG(ERROR) << "Len of destination address too long than buffer. " << wcnt << std::endl;
        return std::nullopt;
    }

    szBuffer[4] = out.size();
    std::copy(out.begin(), out.end(), szBuffer + 5);

    written = write(s, szBuffer, wcnt);
    if (written <= 0)
    {
        close(s);
        LOG(ERROR) << "connect dest failed " << written << std::endl;
        return std::nullopt;
    }

    nread = recv(s, szBuffer, sizeof(szBuffer), 0);
    if (nread <= 0)
    {
        close(s);
        LOG(ERROR) << "connect response failed " << nread << std::endl;
        return nullptr;
    }

    if (szBuffer[0] != 0x5 || szBuffer[1] != 0 || nread < 10 || nread > (sizeof(szBuffer) - 10))
    {
        close(s);
        LOG(ERROR) << "connect failed " << szBuffer[0] << "--" << szBuffer[1] << std::endl;
        return nullptr;
    }

    std::vector<unsigned char> cmds(szBuffer + 3, szBuffer + nread);
    auto bound = Address::FromSocks5CommandStream(cmds);

    LOG(INFO) << "bound address: " << bound->toString() << std::endl;
    std::shared_ptr<Target> rsp(new Target(bound));

    this->init(s);
    LOG(INFO) << "do connect successed " << std::endl;

    return rsp;
}

bool ClientHandle::init(int s)
{
    int n = fcntl(s, F_SETFL, fcntl(s, F_GETFL, 0) | O_NONBLOCK);
    if (n == -1)
    {
        close(s);
        LOG(ERROR) << "set non-block failed" << s << std::endl;
        return false;
    }

    this->sfd = s;

    LOG(INFO) << "one socket recv!" << s << std::endl;

    io.set<ClientHandle, &ClientHandle::callback>(this);

    io.start(s, ev::READ);
    LOG(INFO) << "client loop is running!!!" << s << std::endl;

    return true;
}

void ClientHandle::callback(ev::io &watcher, int revents)
{
    LOG(INFO) << "ClientHandle::callback"
              << " event=" << revents << std::endl;
    if (EV_ERROR & revents)
    {
        LOG(ERROR) << "got a invalid event in ClientHandle" << revents << std::endl;
        return;
    }

    if (revents & EV_READ)
        read_cb(watcher);

    if (revents & EV_WRITE)
        write_cb(watcher);

    if (this->wq->empty())
    {
        io.set(ev::READ);
    }
    else
    {
        io.set(ev::READ | ev::WRITE);
    }
}
void ClientHandle::write_cb(ev::io &watcher)
{
    LOG(INFO) << "write_cb" << std::endl;
    auto sp = this->wq;
    if (sp->empty())
    {
        LOG(ERROR) << "write buffer is empty!!!" << std::endl;
        io.set(ev::READ);
        return;
    }

    auto item = sp->pop();

    LOG(INFO) << "write_cb  " << item->size() << std::endl;

    ssize_t written = write(watcher.fd, item->data(), item->size());
    if (written <= 0)
    {
        // destroy object in read method!!!
        LOG(ERROR) << "Write to Socket(" << watcher.fd
                   << ") failed, Reason:" << written << std::endl;
        return;
    }
}

unsigned int ClientHandle::ToInt(unsigned char *sz)
{
    unsigned hi = sz[0];
    hi = hi << 24;
    hi = hi | (((unsigned)sz[1]) << 16);
    hi = hi | (((unsigned)sz[2]) << 8);
    hi = hi | sz[3];

    return hi;
}

std::vector<unsigned char> ClientHandle::ToBytes(unsigned int v)
{
    std::vector<unsigned char> nn(4, 0);
    nn[3] = v & 0xff;
    nn[2] = (v >> 8) & 0xff;
    nn[1] = (v >> 16) & 0xff;
    nn[0] = (v >> 24) & 0xff;
    return nn;
}

int readbytesfromraw(int fd, unsigned bytes, std::vector<unsigned char> &out)
{

    if (bytes == 0 || bytes > 10000)
    {
        LOG(ERROR) << "readbytesfromraw bytes is zero: " << bytes << std::endl;
        return 0;
    }
    auto buffer = out.data();

    unsigned index = 0;
    unsigned rest = bytes;
    ssize_t n = 0;
    for (;;)
    {
        n = recv(fd, buffer + index, rest, 0);
        LOG(INFO) << "read bytes count: " << n << std::endl;
        if (n <= 0)
        {
            LOG(ERROR) << "read bytes count: " << n << std::endl;
            break;
        }
        index += n;
        rest = bytes - index;
        if (index >= bytes)
        {
            break;
        }
    }

    if (rest == 0 || bytes == index)
    {
        n = 0;
    }
    return n;
}

void ClientHandle::read_cb(ev::io &watcher)
{
    unsigned char szbuffer[4] = {0};
    ssize_t nread = recv(watcher.fd, szbuffer, 4, 0);
    if (nread <= 0)
    {
        LOG(ERROR) << "Read LEN (" << watcher.fd
                   << ") failed, Reason:" << nread << std::endl;
        return;
    }

    auto len = ToInt(szbuffer);
    if (len == 0 || len > 10000)
    {
        LOG(ERROR) << "LEN too big!!!" << std::endl;
        return;
    }
    LOG(INFO) << "read len: " << len << std::endl;

    std::vector<unsigned char> zipBuffer(len, 0);

    auto n = readbytesfromraw(watcher.fd, len, zipBuffer);
    if (n != 0)
    {
        LOG(ERROR) << "read error!!" << std::endl;
        return;
    }

    auto pout = new std::vector<unsigned char>();

    if (this->method->UnCompress(zipBuffer, this->key, *pout) != 0)
    {
        LOG(ERROR) << "UnCompress" << std::endl;
        return;
    }

    if (pout->size() > 0)
    {
        auto sp = std::shared_ptr<std::vector<unsigned char>>(pout);
        if (writer != nullptr)
        {
            writer->Write(sp);
        }
    }
    else
    {
        delete pout;
        pout = nullptr;
    }
}

int ClientHandle::Write(std::shared_ptr<std::vector<unsigned char>> src)
{
    auto out = std::shared_ptr<std::vector<unsigned char>>(new std::vector<unsigned char>());
    if (!!this->method)
    {
        std::vector<unsigned char> kk;
        if (this->method->Compress(*src, this->key, kk) == 0)
        {
            out->resize(kk.size() + 4);
            auto len = ToBytes(kk.size());
            std::copy(len.begin(), len.end(), out->begin());
            std::copy(kk.begin(), kk.end(), out->begin() + 4);
        }
    }
    else
    {
        LOG(ERROR) << "Write position 1" << std::endl;
        return -1;
    }

    if (!!wq && !out->empty())
    {
        LOG(INFO) << "write to remote: " << out->size() << std::endl;
        this->wq->push(out);
        io.set(ev::WRITE);
    }
    else
    {
        LOG(ERROR) << "Write position 2" << std::endl;
        return -1;
    }

    return 0;
}

void ClientHandle::register_io(IOWriter *p)
{
    this->writer = p;
}

ClientHandle::ClientHandle(Address a,
                           std::vector<unsigned char> usr,
                           std::vector<unsigned char> pwd,
                           std::vector<unsigned char> ky) : user(usr),
                                                            pass(pwd),
                                                            key(ky),
                                                            remote(a),
                                                            sfd(0),
                                                            wq(new Buffer())

{
}

ClientHandle::~ClientHandle()
{
    LOG(INFO) << "~ClientHandle()" << std::endl;
    io.stop();
    close(sfd);
}
