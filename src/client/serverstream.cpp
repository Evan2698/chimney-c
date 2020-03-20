#include "client/serverstream.h"
#include <fcntl.h>
#include <glog/logging.h>
#include <sys/socket.h>
#include "common/address.h"
#include "common/func.hpp"
#include "common/g.h"
std::atomic<long long> ServerStream::clientCount;


ServerStream::ServerStream() : readBuffer(BUFFER_SIZE_MAX, 0),
                               wq(new Buffer()),
                               sfd(0)
{
    ++clientCount;
    LOG(INFO) << clientCount << "client(s) created." << std::endl;
}

ServerStream::~ServerStream()
{
    io.stop();

    close(sfd);

    --clientCount;

    LOG(INFO) << clientCount << "client(s) connected." << std::endl;
    LOG(INFO) << "~ServerStream()" << std::endl;
}

bool ServerStream::hand_shake_end(int s)
{
    int n = fcntl(s, F_SETFL, fcntl(s, F_GETFL, 0) | O_NONBLOCK);
    if (n == -1)
    {
        close(s);
        LOG(ERROR) << "set non-block failed" << s << std::endl;
        return false;
    }

    this->sfd = s;

    LOG(INFO) << "one socket recv!  socket=" << s << std::endl;

    io.set<ServerStream, &ServerStream::callback>(this);

    io.start(s, ev::READ);

    return true;
}

bool ServerStream::init(int s)
{
    unsigned char szBuffer[512] = {0};

    // step 1: echo hello
    ssize_t nread = recv(s, szBuffer, sizeof(szBuffer), 0);
    LOG(INFO) << "First: " << nread  << " ==>" << ToHexEX(szBuffer, szBuffer + std::min<ssize_t>(nread, 512))
              << std::endl;
    if (nread <= 0)
    {
        close(s);
        LOG(INFO) << "Read to Socket(" << s
                  << ") failed, Reason:" << nread << std::endl;

        return false;
    }
    if (szBuffer[0] != 0x5 || szBuffer[1] == 0 || nread > 20)
    {
        szBuffer[0] = 0x5;
        szBuffer[1] = 0xff;
        write(s, szBuffer, 2);
        close(s);
        LOG(ERROR) << "Hello format is invalid." << ToHexEX(szBuffer, szBuffer + std::min((ssize_t)20, nread)) << std::endl;
        return false;
    }
    szBuffer[0] = 0x5;
    szBuffer[1] = 0x00;
    auto n = write(s, szBuffer, 2);
    LOG(INFO) << "Hello response :" << n << " bytes writen." << std::endl;

    // step 2: connect
    memset(szBuffer, 0, sizeof szBuffer);
    nread = recv(s, szBuffer, sizeof(szBuffer), 0);
    LOG(INFO) << "second: " << nread  << " ==>" << ToHexEX(szBuffer, szBuffer + std::min<ssize_t>(nread, 512))
              << std::endl;
    if (nread <= 0)
    {
        close(s);
        LOG(INFO) << "Read socks5 command (" << s
                  << ") failed, Reason:" << nread << std::endl;

        return false;
    }

    if (nread < 4 || szBuffer[0] != 0x5 || szBuffer[1] != 1)
    {
        unsigned char rsu[10] = {0x05, 0x0A, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        write(s, rsu, sizeof rsu);
        close(s);
        auto offset = std::min<ssize_t>(sizeof szBuffer, nread);
        LOG(INFO) << "socks 5 command error (" << s
                  << ") failed, Reason:" << ToHexEX(szBuffer, szBuffer + offset) << std::endl;

        return false;
    }
     LOG(INFO) << "will connect!! "          << std::endl;

    auto target = handleCommand(std::vector<unsigned char>(szBuffer + 3, szBuffer + nread));
    auto addr = target->getBoundAddresss();
    if (!addr->isValid())
    {
        std::shared_ptr<Buffer::BUFFERTYPE> r = std::make_shared<Buffer::BUFFERTYPE>(10, 0);
        unsigned char rsu[10] = {0x05, 0x0B, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        write(s, rsu, sizeof rsu);
        LOG(ERROR) << "bound address got failed (" << s
                   << ") failed, Reason:" << nread << std::endl;
        close(s);
        return false;
    }

    auto ss = addr->PackSocks5Address();
    if (ss.empty())
    {
        std::shared_ptr<Buffer::BUFFERTYPE> r = std::make_shared<Buffer::BUFFERTYPE>(10, 0);
        unsigned char rsu[10] = {0x05, 0x0C, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        write(s, rsu, sizeof rsu);
        close(s);
        LOG(ERROR) << "bound address got failed (" << s
                   << ") failed, Reason:" << nread << std::endl;
        return false;
    }
    n = write(s, ss.data(), ss.size());
    LOG(INFO) << "write bound address Reason:" << n  << "X= " << ToHexEX(ss.begin(), ss.end())<< std::endl;
    if (0 >= n)
    {
        close(s);
        LOG(ERROR) << "shake failed!!!!";
        return false;
    }

    this->hand_shake_end(s);

    return true;
}

void ServerStream::callback(ev::io &watcher, int revents)
{
    LOG(INFO) << "event = " << revents << std::endl;
    if (EV_ERROR & revents)
    {
        LOG(ERROR) << "got a invalid event in ServerStream" << revents << std::endl;
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

void ServerStream::write_cb(ev::io &watcher)
{

    auto sp = this->wq;
    if (sp->empty())
    {
        LOG(INFO) << "write buffer is empty!!!" << clientCount << std::endl;
        io.set(ev::READ);
        return;
    }

    auto item = sp->pop();

    LOG(INFO) << "write_cb byte: " << ToHexEX(item->begin(), item->end()) << std::endl;

    ssize_t written = write(watcher.fd, item->data(), item->size());
    if (written < 0)
    {
        // destroy object in read method!!!
        LOG(INFO) << "Write to Socket(" << watcher.fd
                  << ") failed, Reason:" << written << std::endl;
        return;
    }
    LOG(INFO) << "write_cb byte:  result: " << written << std::endl;
}

void ServerStream::read_cb(ev::io &watcher)
{
    LOG(INFO) << "read_cb : ----- " << watcher.fd<< std::endl;
    auto buffer = this->readBuffer.data();
    memset(buffer, 0, this->readBuffer.size());
    ssize_t nread = recv(watcher.fd,
                         buffer,
                         this->readBuffer.size(), 0);

    LOG(INFO) << "READ RAW: " << nread << std::endl;
    std::lock_guard<std::mutex> lck(this->_mutex);
    if (nread < 0)
    {
        LOG(INFO) << "Read to Socket(" << watcher.fd
                  << ") failed, Reason:" << nread << std::endl;
        return;
    }

    if (nread == 0)
    {
        // Gack - we're deleting ourself inside of ourself!
        LOG(INFO) << "Destory Socket object (" << watcher.fd
                  << ") failed, Reason:" << nread << std::endl;
        delete this;
    }
    else
    {
        LOG(INFO) << "READ: " << ToHexEX(buffer, buffer + std::min<ssize_t>(nread,BUFFER_SIZE_MAX)) << " Count: " << nread << std::endl;
        if (!!this->client)
        {
            std::shared_ptr<Buffer::BUFFERTYPE> r = std::make_shared<Buffer::BUFFERTYPE>(nread, 0);
            std::copy(this->readBuffer.begin(), this->readBuffer.begin() + nread, r->begin());
            client->Write(r);
        }
    }
}

std::shared_ptr<Target> ServerStream::handleCommand(const std::vector<unsigned char> &cmd)
{
    auto addr = Address::FromSocks5CommandStream(cmd);
    if (!!this->client)
    {
        LOG(INFO) << "destion address: " << addr->toString() << std::endl;
        auto target = this->client->do_connect(*addr.get());
        if (target.has_value())
        {
            LOG(INFO) << "target  GOT!" << std::endl;
            return target.value();
        }
    }
    return std::shared_ptr<Target>();
}

int ServerStream::Write(std::shared_ptr<std::vector<unsigned char>> src)
{
    if (!!wq)
    {
        LOG(INFO) << "ServerStream::Write" << std::endl;
        this->wq->push(src);
        io.set(ev::WRITE);
    }
}

void ServerStream::set_client(std::shared_ptr<ClientHandle> c)
{
    this->client = c;
}
