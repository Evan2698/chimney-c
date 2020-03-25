#include "privacy/chacha20.h"
#include <vector>
#include "privacy/xchacha20.h"
#include "core/g.h"


int ChaCha20::Compress(const std::vector<unsigned char> &in,
                       const std::vector<unsigned char> &key,
                       std::vector<unsigned char> &out)
{
    if (key.size() != 32)
    {
        LOG_S(INFO) << "key size must be 32 bytes long. " << std::endl;
        return -1;
    }

    if (m_IV.size() != 24)
    {
        LOG_S(ERROR) << "IV must be 24 bytes long. " << std::endl;
        return -2;
    }

    if (in.empty())
    {
        LOG_S(ERROR) << "input can not be empty. " << std::endl;
        return -3;
    }


    out.resize(in.size());
    XChaCha_ctx ctx;
    uint8_t counter[8] = {0x0};   // 0 or 1,  for golang use 0, for Crypto++ use 1.
    xchacha_keysetup(&ctx, key.data(), m_IV.data());
    xchacha_set_counter(&ctx, counter);
	xchacha_encrypt_bytes(&ctx, in.data(), out.data(), in.size());

    return 0;
}

int ChaCha20::UnCompress(const std::vector<unsigned char> &in,
                         const std::vector<unsigned char> &key,
                         std::vector<unsigned char> &out)
{
    return Compress(in, key, out);
}

void ChaCha20::SetIV(const std::vector<unsigned char> &iv)
{
    this->m_IV.resize(iv.size());
    std::copy(iv.begin(), iv.end(), m_IV.begin());
}
std::vector<unsigned char> ChaCha20::ToBytes()
{
    std::vector<unsigned char> s(3 + m_IV.size(), 0);
    std::copy(m_IV.begin(), m_IV.end(), s.begin() + 3);
    s[0] = 0x12;
    s[1] = 0x35;
    s[2] = static_cast<unsigned char>(m_IV.size());
    return std::move(s);
}

int ChaCha20::GetMask() const
{
    return 0x1235;
}