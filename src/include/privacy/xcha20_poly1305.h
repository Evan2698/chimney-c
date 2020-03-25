/*******************************************************************************
 *
 * XChaCha20-Poly1305
 * XChaCha20-Poly1305 class for encryption nothing 
 *
 * Copyright 2020 Evan . All rights reserved.
 *
 ******************************************************************************/
#ifndef XCHACHA20_POLY_1305_H
#define XCHACHA20_POLY_1305_H

#include "privacy.h"

struct XChaCha20Poly1305 : public Privacy
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

    XChaCha20Poly1305() = default;
    ~XChaCha20Poly1305() = default;

private:
    std::vector<unsigned char> m_iv;
};

#endif