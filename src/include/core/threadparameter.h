#ifndef THREAD_PARAMETER_H_32989423_48507823495
#define THREAD_PARAMETER_H_32989423_48507823495
#include "core/stream.h"
#include "privacy/privacy.h"
#include "core/g.h"

struct ThreadParameter
{
    std::shared_ptr<Stream> Src;
    std::shared_ptr<Stream> Dst;
    std::vector<unsigned char> Key;
    std::shared_ptr<Privacy> I;

    bool isValid()
    {
        if (!!Src && !!Dst && !Key.empty() && !!I)
        {
            return true;
        }
        return false;
    }
};

#endif