/*******************************************************************************
 *
 * RawEncrypt
 * RawEncrypt class for encryption nothing 
 *
 * Copyright 2020 Evan . All rights reserved.
 *
 ******************************************************************************/
#ifndef RAW_H_2020_03_16
#define RAW_H_2020_03_16

#include "privacy.h"

struct RawEncrypt : public Privacy
{
    virtual int Compress(const std::vector<unsigned char> &in,
                         const std::vector<unsigned char> &key,
                         std::vector<unsigned char> &out);

    virtual int UnCompress(const std::vector<unsigned char> &in,
                           const std::vector<unsigned char> &key,
                           std::vector<unsigned char> &out);

    virtual void SetIV(const std::vector<unsigned char> &iv);

    virtual std::vector<unsigned char> ToBytes();

    virtual int GetMask() const;

private:
    std::vector<unsigned char> m_IV;
};

#endif
