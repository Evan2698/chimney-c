/*******************************************************************************
 *
 * aes-gcm
 * encrypt method for aes-gcm
 *
 * Copyright 2020 Evan . All rights reserved.
 *
 ******************************************************************************/
#include <iostream>
#include "core/g.h"
#include "privacy/gcm.h"
#include <sodium.h>

inline std::size_t alloSize(size_t o)
{
    return (((o >> 5) + 2) << 5) + 16;
}

static bool init()
{
    auto r = sodium_init();
    return true;
}

bool GCM::inited = init();

int GCM::Compress(const std::vector<unsigned char> &in,
                  const std::vector<unsigned char> &key,
                  std::vector<unsigned char> &out)
{

    if (key.size() != 32)
    {
        LOG(ERROR) << "key size must be 32 bytes long. " << std::endl;
        return -1;
    }

    if (m_iv.size() != 12)
    {
        LOG(ERROR) << "IV must be 12 bytes long. " << std::endl;
        return -2;
    }

    if (in.empty())
    {
        LOG(ERROR) << "input can not be empty. " << std::endl;
        return -3;
    }

    if (!inited)
    {
        LOG(ERROR) << "WHY??? " << std::endl;
        return -4;
    }

    if (crypto_aead_aes256gcm_is_available() == 0)
    {
        LOG(ERROR) << "NOT SUPPORT GCM ON THIS CPU." << std::endl;
        return -5;
    }

    out.resize(alloSize(in.size()));

    unsigned long long ciphertext_len = out.size();
    auto ret = crypto_aead_aes256gcm_encrypt(out.data(), &ciphertext_len,
                                  in.data(), in.size(),
                                  nullptr, 0,
                                  nullptr, m_iv.data(), key.data());
    out.resize(ciphertext_len);

    return (ret == 0) ? ret :  1;
}

int GCM::UnCompress(const std::vector<unsigned char> &in,
                    const std::vector<unsigned char> &key,
                    std::vector<unsigned char> &out)
{
    if (key.size() != 32)
    {
        LOG(ERROR) << "key size must be 32 bytes long. " << std::endl;
        return -1;
    }

    if (m_iv.size() != 12)
    {
        LOG(ERROR) << "IV must be 12 bytes long. " << std::endl;
        return -2;
    }

    if (in.empty())
    {
        LOG(ERROR) << "input can not be empty. " << std::endl;
        return -3;
    }

    if (!inited)
    {
        LOG(ERROR) << "WHY??? " << std::endl;
        return -4;
    }

    if (crypto_aead_aes256gcm_is_available() == 0)
    {
        LOG(ERROR) << "NOT SUPPORT GCM ON THIS CPU." << std::endl;
        return -5;
    }

    out.resize(alloSize(in.size()));

    unsigned long long decrypted_len = out.size();
    auto ret = crypto_aead_aes256gcm_decrypt(out.data(), &decrypted_len,
                                  nullptr,
                                  in.data(), in.size(),
                                  nullptr,
                                  0,
                                  m_iv.data(), key.data());
    out.resize(decrypted_len);

    return (ret == 0) ? 0 : 1;
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
