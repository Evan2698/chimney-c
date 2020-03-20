/*******************************************************************************
 *
 * ChaCha20
 * ChaCha20 class for encryption
 *
 * Copyright 2020 Evan . All rights reserved.
 *
 ******************************************************************************/
#ifndef CHACHA20_H_2020_03_16
#define CHACHA20_H_2020_03_16
#include "privacy.h"
struct ChaCha20 : public Privacy
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

    ChaCha20() = default;
    ~ChaCha20() = default;

private:
    std::vector<unsigned char> m_IV;
};

#endif