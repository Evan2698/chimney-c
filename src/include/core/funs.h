#ifndef FUNCS_H_3848837748484
#define FUNCS_H_3848837748484
#include <vector>

struct Funcs
{
    static unsigned int ToInt(unsigned char *sz);
    static std::vector<unsigned char> ToBytes(unsigned int v);
    static std::vector<unsigned char> U16ToBytes(unsigned short v);
    static unsigned short ToShort(unsigned char *sz);
};


#endif 