#ifndef READ_WRITE_CLOSER_H
#define READ_WRITE_CLOSER_H
#include <vector>

struct ReadWriteCloser
{
    virtual int Read(std::vector<unsigned char> &out, bool sync = false) = 0;

    virtual int Write(const std::vector<unsigned char> &src) = 0;

    virtual int Close() = 0;

    virtual ~ReadWriteCloser() = default;
};

#endif