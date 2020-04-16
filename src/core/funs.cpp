#include "core/funs.h"

unsigned int Funcs::ToInt(unsigned char *sz)
{
    unsigned hi = sz[0];
    hi = hi << 24;
    hi = hi | (((unsigned)sz[1]) << 16);
    hi = hi | (((unsigned)sz[2]) << 8);
    hi = hi | sz[3];

    return hi;
}

std::vector<unsigned char> Funcs::ToBytes(unsigned int v)
{
    std::vector<unsigned char> nn(4, 0);
    nn[3] = v & 0xff;
    nn[2] = (v >> 8) & 0xff;
    nn[1] = (v >> 16) & 0xff;
    nn[0] = (v >> 24) & 0xff;
    return std::move(nn);
}

std::vector<unsigned char> Funcs::U16ToBytes(unsigned short v)
{
    std::vector<unsigned char> nn(2, 0);
    nn[1] = v & 0xff;
    nn[0] = (v >> 8) & 0xff;

    return std::move(nn);
}

unsigned short Funcs::ToShort(unsigned char *sz)
{
    unsigned short v = sz[0];
    v = (v << 8) | sz[1];

    return v;
}