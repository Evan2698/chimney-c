#include "privacy/raw.h"

int RawEncrypt::Compress(const std::vector<unsigned char> &in,
                         const std::vector<unsigned char> &key,
                         std::vector<unsigned char> &out)
{
    out.resize(in.size());
    std::copy(in.begin(), in.end(), out.begin());
    return 0;
}

int RawEncrypt::UnCompress(const std::vector<unsigned char> &in,
                           const std::vector<unsigned char> &key,
                           std::vector<unsigned char> &out)
{
    out.resize(in.size());
    std::copy(in.begin(), in.end(), out.begin());
    return 0;
}

void RawEncrypt::SetIV(const std::vector<unsigned char> &iv)
{
    m_IV.resize(iv.size());
    std::copy(iv.begin(), iv.end(), m_IV.begin());
}

std::vector<unsigned char> RawEncrypt::ToBytes()
{
    std::vector<unsigned char> s(3 + m_IV.size(), 0);
    std::copy(m_IV.begin(), m_IV.end(), s.begin() + 3);
    s[0] = 0x12;
    s[1] = 0x37;
    s[2] = static_cast<unsigned char>(m_IV.size());
    return std::move(s);
}

int RawEncrypt::GetMask() const
{
    return 0x1237;
}
