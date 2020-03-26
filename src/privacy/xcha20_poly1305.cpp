#include "privacy/xcha20_poly1305.h"
#include "sodium.h"
#include "core/g.h"

inline std::size_t alloSize(size_t o)
{
    return (((o >> 5) + 2) << 5) + 64;
}

int XChaCha20Poly1305::Compress(const std::vector<unsigned char> &in,
                                const std::vector<unsigned char> &key,
                                std::vector<unsigned char> &out)
{
    if (key.size() != 32)
    {
        LOG(ERROR) << "key size must be 32 bytes long. " << std::endl;
        return -1;
    }

    if (m_iv.size() != 24)
    {
        LOG(ERROR) << "IV must be 24 bytes long. " << std::endl;
        return -2;
    }

    if (in.empty())
    {
        LOG(ERROR) << "input can not be empty. " << std::endl;
        return -3;
    }

    out.resize(alloSize(in.size()));

    unsigned long long ciphertext_len = out.size();
    auto r = crypto_aead_xchacha20poly1305_ietf_encrypt(out.data(), &ciphertext_len,
                                                        in.data(), in.size(),
                                                        nullptr, 0,
                                                        NULL, m_iv.data(), key.data());
    if (r != 0)
    {
        LOG(ERROR) << "crypto_aead_xchacha20poly1305_ietf_encrypt failed. " << r << std::endl;
        return -4;
    }

    out.resize(ciphertext_len);
    return 0;
}

int XChaCha20Poly1305::UnCompress(const std::vector<unsigned char> &in,
                                  const std::vector<unsigned char> &key,
                                  std::vector<unsigned char> &out)
{
    if (key.size() != 32)
    {
        LOG(ERROR) << "key size must be 32 bytes long. " << std::endl;
        return -1;
    }

    if (m_iv.size() != 24)
    {
        LOG(ERROR) << "IV must be 24 bytes long. " << std::endl;
        return -2;
    }

    if (in.empty())
    {
        LOG(ERROR) << "input can not be empty. " << std::endl;
        return -3;
    }

    out.resize(alloSize(in.size()));
    unsigned long long decrypted_len = out.size();
    auto r = crypto_aead_xchacha20poly1305_ietf_decrypt(out.data(), &decrypted_len,
                                                        NULL,
                                                        in.data(), in.size(),
                                                        nullptr,
                                                        0,
                                                        m_iv.data(), key.data());
    if (r != 0)
    {
        LOG(ERROR) << "crypto_aead_xchacha20poly1305_ietf_decrypt failed. " << r << std::endl;
        return -4;
    }

    out.resize(decrypted_len);

    return 0;
}

void XChaCha20Poly1305::SetIV(const std::vector<unsigned char> &iv)
{
    m_iv.resize(iv.size());
    std::copy(iv.begin(), iv.end(), m_iv.begin());
}

std::vector<unsigned char> XChaCha20Poly1305::ToBytes()
{
    std::vector<unsigned char> s(3 + m_iv.size(), 0);
    std::copy(m_iv.begin(), m_iv.end(), s.begin() + 3);
    s[0] = 0x12;
    s[1] = 0x36;
    s[2] = static_cast<unsigned char>(m_iv.size());
    return std::move(s);
}

int XChaCha20Poly1305::GetMask() const
{
    return 0x1236;
}
