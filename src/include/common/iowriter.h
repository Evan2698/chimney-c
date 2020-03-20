#ifndef IWRITE_H_394848374
#define IWRITE_H_394848374
#include <vector>
#include <memory>

struct IOWriter
{
    virtual int Write(std::shared_ptr<std::vector<unsigned char>> src) = 0;
};

#endif 