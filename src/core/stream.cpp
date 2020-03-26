#include "core/stream.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include "core/g.h"
#include <unistd.h>
Stream::Stream(int h, Address l, Address r) : handle(h),
                                              local(l),
                                              remote(r)
{
}

Stream::Stream(int h) : handle(h)
{
}

Stream::~Stream()
{
    LOG(INFO) << "~Stream["
              << this->local.toString() << "------"
              << this->remote.toString() << "]"
              << std::endl;
    Close();
}

static int ReadXBytes(int socket, unsigned int x, void* buffer)
{
    unsigned bytesRead = 0;
    ssize_t result;
    while (bytesRead < x)
    {
        result = read(socket, buffer + bytesRead, x - bytesRead);
        if (result < 1 )
        {
            return -1;
        }
        bytesRead += result;
    }

    return x;
}

int Stream::Read(std::vector<unsigned char> &out, bool sync)
{
    if (out.empty())
        return -1;

    ssize_t ret  = 0;
    if (!sync)
    {
        ret = read(this->handle, out.data(), out.size());
        if (ret < 1)
        {
            return ret;
        }
    }
    else
    {
       ret = ReadXBytes(this->handle, out.size(),out.data());
       if (ret <=0 ){
           return ret;
       }
    }

    if (ret != out.size())
    {
        out.resize(ret);
    }  

    return ret;
}

int Stream::Write(const std::vector<unsigned char> &src)
{
    auto nbytes = send(this->handle, src.data(), src.size(), 0);
    if (nbytes <= 0)
    {
        return nbytes;
    }

    return nbytes;
}

int Stream::Close()
{
    close(this->handle);
    this->handle = 0;
}

Address Stream::get_local()
{
    return local;
}

Address Stream::get_remote()
{
    return remote;
}

void Stream::set_local(const Address &a)
{
    this->local = a;
}
void Stream::set_remote(const Address &a)
{
    this->remote = a;
}