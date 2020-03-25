#include "core/minipool.h"
#include "core/g.h"
#include <logfault/logfault.h>

static const unsigned int offset = 32; // offset for start | align
static const int capacity = 50;        // full size
static const int tail = 256;           //reserved
unsigned char *MiniPool::Alloc()
{
    std::lock_guard<std::mutex> guard(_mutex);
    if (unuse_buffer.size() > 0)
    {
        LFLOG_INFO << "object in cache!!!" << std::endl;
        auto p = unuse_buffer.front();
        unuse_buffer.pop();
        using_buffer.insert(std::pair<unsigned char *, unsigned char *>(p, p));
        return p;
    }
    else
    {
        LFLOG_INFO << "there is no more object in cache.!!" << std::endl;
        auto p = new unsigned char[BUFFER_SIZE_MAX];
        using_buffer.insert(std::pair<unsigned char *, unsigned char *>(p, p));
        return p;
    }
    LFLOG_INFO << "usage: " << using_buffer.size() << "  unused: "
              << unuse_buffer.size() << std::endl;
}

void MiniPool::Free(unsigned char *p)
{
    std::lock_guard<std::mutex> guard(_mutex);
    if (p != nullptr)
    {
        auto got = this->using_buffer.find(p);
        if (got != using_buffer.end())
        {
            using_buffer.erase(got);
            if (unuse_buffer.size() < capacity)
            {
                unuse_buffer.push(p);
            }
            else
            {
                delete[] p;
                LFLOG_INFO << "destory Mini Buffer immediately" << std::endl;
            }
        }
        else
        {
            LFLOG_ERROR << "CAN NOT RUN HERE!!!!!!!" << std::endl;
        }
    }

    LFLOG_INFO << "usage: " << using_buffer.size() << "  unused: "
              << unuse_buffer.size() << "FREE BUFFER!!" << std::endl;
}

MiniPool::MiniPool()
{
}

unsigned MiniPool::size()
{
    return BUFFER_SIZE_MAX - tail;
}

MiniPool::~MiniPool()
{
    LFLOG_INFO << "Need not Destory all object!" << std::endl;
}

void MiniPool::init(int n)
{
    static int flag = 0;
    if (flag == 0)
    {
        std::lock_guard<std::mutex> guard(_mutex);
        if (flag == 0)
        {
            for (int i = 0; i < 5; ++i)
            {
                auto p = new unsigned char[BUFFER_SIZE_MAX];
                using_buffer.insert(std::pair<unsigned char *, unsigned char *>(p, p));
            }
            flag = 1;
        }
    }
}

MiniPool &MiniPool::get_instance()
{
    static MiniPool pool;
    pool.init(5);
    return pool;
}

BufferHolder::BufferHolder() : p(nullptr)
{
    p = MiniPool::get_instance().Alloc();
}

BufferHolder::BufferHolder(bool empty) : p(nullptr)
{
   UNREFERENCED_PARAMETER(empty);
}


BufferHolder::~BufferHolder()
{
    if (p != nullptr)
    {
        MiniPool::get_instance().Free(p);
        p = nullptr;
    }
}

BufferHolder:: operator unsigned char *()
{
    return p;
}

void BufferHolder::attach(void *o)
{
     p = (unsigned char *) o;
}

unsigned BufferHolder::size()
{
    return MiniPool::get_instance().size();
}

unsigned char *BufferHolder::detach()
{
    auto h = p;
    p = nullptr;
    return h;
}