#ifndef STREAM_H_394857
#define STREAM_H_394857
#include "core/readwritecloser.h"
#include "core/address.h"

struct Stream : public ReadWriteCloser
{
    Stream(int h, Address l, Address r);
    Stream(int h);
    ~Stream();

    virtual int Read(std::vector<unsigned char> &out);

    virtual int Write(const std::vector<unsigned char> &src);

    virtual int Close();

    Address get_local();

    Address get_remote();

    void set_local(const Address & a);
    void set_remote(const Address & a);

private:
    int handle;
    Address local;
    Address remote;
};

struct StreamHodler
{
    StreamHodler(Stream *p) : mp(p)
    {
    }

    StreamHodler():mp(nullptr)
    {

    }

    ~StreamHodler()
    {
        if (mp != nullptr)
        {
            delete mp;
        }
    }

    void attach(Stream *p)
    {
        mp = p;
    }

    Stream * detach()
    {
        auto tmp = mp;
        mp = nullptr;
        return tmp;
    }

    Stream *operator->()
    {
        return mp;
    }

private:
    Stream *mp;
};

#endif