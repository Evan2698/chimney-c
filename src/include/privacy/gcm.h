/*******************************************************************************
 *
 * aes-gcm
 * encrypt method for aes-gcm
 *
 * Copyright 2020 Evan . All rights reserved.
 *
 ******************************************************************************/

#ifndef AES_GCM_2020_03_16_H
#define AES_GCM_2020_03_16_H
#include "privacy.h"

struct GCM : public Privacy
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

     GCM() = default;
     ~GCM() = default;

private:
     void aes_init();

private:
     std::vector<unsigned char> m_iv;
};

#endif