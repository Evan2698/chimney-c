/*******************************************************************************
 *
 * mini pool
 * 
 *
 * 
 *
 ******************************************************************************/

#ifndef MINI_POOL_H_9848237887_3845897485783
#define MINI_POOL_H_9848237887_3845897485783
#include <memory>
#include <mutex>
#include <unordered_map>
#include <queue>

struct MiniPool
{

    unsigned char *Alloc(); 
    void Free(unsigned char *p);

    MiniPool();
    virtual ~MiniPool();

    static MiniPool & get_instance();
    void init(int n);

    unsigned size();


private:
    std::mutex _mutex;
    std::unordered_map<unsigned char*, unsigned char *> using_buffer;
    std::queue<unsigned char *> unuse_buffer;

    friend class BufferHolder;
};

struct BufferHolder
{
    BufferHolder();
    BufferHolder(bool empty);
    virtual ~BufferHolder();
    operator unsigned char * ();
    unsigned size();
    unsigned char * detach();
    void attach(void *p);

private:
    unsigned char *p;

};


#endif