/*******************************************************************************
 *
 * aes-gcm
 * encrypt method for aes-gcm
 *
 * Copyright 2020 Evan . All rights reserved.
 *
 ******************************************************************************/

#include <openssl/aes.h>
#include <openssl/evp.h>
#include <iostream>
#include "privacy/gcm.h"

inline std::size_t alloSize(size_t o)
{
    return (((o >> 5) + 2) << 5) + 16;
}

int GCM::Compress(const std::vector<unsigned char> &in,
                  const std::vector<unsigned char> &key,
                  std::vector<unsigned char> &out)
{

    if (key.size() != 32)
        return -1;
    if (m_iv.size() != 12)
        return -2;
    if (in.empty())
        return -3;

    aes_init();
    out.resize(alloSize(in.size()));
    unsigned char tag[AES_BLOCK_SIZE];
    int actual_size = 0, final_size = 0;
    EVP_CIPHER_CTX *e_ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit(e_ctx, EVP_aes_256_gcm(), (const unsigned char *)key.data(), m_iv.data());
    EVP_EncryptUpdate(e_ctx, &out[0], &actual_size, (const unsigned char *)in.data(), in.size());
    EVP_EncryptFinal(e_ctx, &out[actual_size], &final_size);
    EVP_CIPHER_CTX_ctrl(e_ctx, EVP_CTRL_GCM_GET_TAG, 16, tag);
    std::copy(tag, tag + 16, &out[actual_size + final_size]);
    out.resize(16 + actual_size + final_size);
    EVP_CIPHER_CTX_free(e_ctx);
    return 0;
}

int GCM::UnCompress(const std::vector<unsigned char> &in,
                    const std::vector<unsigned char> &key,
                    std::vector<unsigned char> &out)
{
    if (key.size() != 32)
        return -1;
    if (m_iv.size() != 12)
        return -2;
    if (in.empty())
        return -3;

    aes_init();

    unsigned char tag[AES_BLOCK_SIZE] = {0};
    std::copy(in.begin() + (in.size() - 16), in.end(), tag);
    out.resize(in.size());
    int actual_size = 0, final_size = 0;
    EVP_CIPHER_CTX *d_ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit(d_ctx, EVP_aes_256_gcm(), (const unsigned char *)key.data(), m_iv.data());
    EVP_DecryptUpdate(d_ctx, &out[0], &actual_size, &in[0], in.size() - 16);
    EVP_CIPHER_CTX_ctrl(d_ctx, EVP_CTRL_GCM_SET_TAG, 16, tag);
    EVP_DecryptFinal(d_ctx, &out[actual_size], &final_size);
    EVP_CIPHER_CTX_free(d_ctx);
    out.resize(actual_size + final_size);

    return 0;
}

std::vector<unsigned char> GCM::ToBytes()
{
    std::vector<unsigned char> s(3 + m_iv.size(), 0);
    std::copy(m_iv.begin(), m_iv.end(), s.begin() + 3);
    s[0] = 0x12;
    s[1] = 0x34;
    s[2] = static_cast<unsigned char>(m_iv.size());
    return std::move(s);
}

int GCM::GetMask() const
{
    return 0x1234;
}

void GCM::SetIV(const std::vector<unsigned char> &iv)
{
    m_iv.resize(iv.size());
    std::copy(iv.begin(), iv.end(), m_iv.begin());
}

void GCM::aes_init()
{
    static int init = 0;
    if (init == 0)
    {
        OpenSSL_add_all_ciphers();
        init = 1;
    }
}