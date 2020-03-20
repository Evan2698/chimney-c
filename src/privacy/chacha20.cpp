#include "privacy/chacha20.h"
#include <vector>

static void core(std::vector<unsigned int> &input, std::vector<unsigned int> &output,
                 unsigned char rounds, bool hchacha)
{

    unsigned int x00 = input[0];
    unsigned int x01 = input[1];
    unsigned int x02 = input[2];
    unsigned int x03 = input[3];
    unsigned int x04 = input[4];
    unsigned int x05 = input[5];
    unsigned int x06 = input[6];
    unsigned int x07 = input[7];
    unsigned int x08 = input[8];
    unsigned int x09 = input[9];
    unsigned int x10 = input[10];
    unsigned int x11 = input[11];
    unsigned int x12 = input[12];
    unsigned int x13 = input[13];
    unsigned int x14 = input[14];
    unsigned int x15 = input[15];

    unsigned int x = 0;
    // Unrolling all 20 rounds kills performance on modern Intel processors
    // (Tested on a i5 Haswell, likely applies to Sandy Bridge+), due to uop
    // cache thrashing.  The straight forward 2 rounds per loop implementation
    // of this has double the performance of the fully unrolled version.
    for (unsigned char i = 0; i < rounds; i += 2)
    {
        x00 += x04;
        x = x12 ^ x00;
        x12 = (x << 16) | (x >> 16);
        x08 += x12;
        x = x04 ^ x08;
        x04 = (x << 12) | (x >> 20);
        x00 += x04;
        x = x12 ^ x00;
        x12 = (x << 8) | (x >> 24);
        x08 += x12;
        x = x04 ^ x08;
        x04 = (x << 7) | (x >> 25);
        x01 += x05;
        x = x13 ^ x01;
        x13 = (x << 16) | (x >> 16);
        x09 += x13;
        x = x05 ^ x09;
        x05 = (x << 12) | (x >> 20);
        x01 += x05;
        x = x13 ^ x01;
        x13 = (x << 8) | (x >> 24);
        x09 += x13;
        x = x05 ^ x09;
        x05 = (x << 7) | (x >> 25);
        x02 += x06;
        x = x14 ^ x02;
        x14 = (x << 16) | (x >> 16);
        x10 += x14;
        x = x06 ^ x10;
        x06 = (x << 12) | (x >> 20);
        x02 += x06;
        x = x14 ^ x02;
        x14 = (x << 8) | (x >> 24);
        x10 += x14;
        x = x06 ^ x10;
        x06 = (x << 7) | (x >> 25);
        x03 += x07;
        x = x15 ^ x03;
        x15 = (x << 16) | (x >> 16);
        x11 += x15;
        x = x07 ^ x11;
        x07 = (x << 12) | (x >> 20);
        x03 += x07;
        x = x15 ^ x03;
        x15 = (x << 8) | (x >> 24);
        x11 += x15;
        x = x07 ^ x11;
        x07 = (x << 7) | (x >> 25);
        x00 += x05;
        x = x15 ^ x00;
        x15 = (x << 16) | (x >> 16);
        x10 += x15;
        x = x05 ^ x10;
        x05 = (x << 12) | (x >> 20);
        x00 += x05;
        x = x15 ^ x00;
        x15 = (x << 8) | (x >> 24);
        x10 += x15;
        x = x05 ^ x10;
        x05 = (x << 7) | (x >> 25);
        x01 += x06;
        x = x12 ^ x01;
        x12 = (x << 16) | (x >> 16);
        x11 += x12;
        x = x06 ^ x11;
        x06 = (x << 12) | (x >> 20);
        x01 += x06;
        x = x12 ^ x01;
        x12 = (x << 8) | (x >> 24);
        x11 += x12;
        x = x06 ^ x11;
        x06 = (x << 7) | (x >> 25);
        x02 += x07;
        x = x13 ^ x02;
        x13 = (x << 16) | (x >> 16);
        x08 += x13;
        x = x07 ^ x08;
        x07 = (x << 12) | (x >> 20);
        x02 += x07;
        x = x13 ^ x02;
        x13 = (x << 8) | (x >> 24);
        x08 += x13;
        x = x07 ^ x08;
        x07 = (x << 7) | (x >> 25);
        x03 += x04;
        x = x14 ^ x03;
        x14 = (x << 16) | (x >> 16);
        x09 += x14;
        x = x04 ^ x09;
        x04 = (x << 12) | (x >> 20);
        x03 += x04;
        x = x14 ^ x03;
        x14 = (x << 8) | (x >> 24);
        x09 += x14;
        x = x04 ^ x09;
        x04 = (x << 7) | (x >> 25);
    }

    if (!hchacha)
    {
        output[0] = x00 + input[0];
        output[1] = x01 + input[1];
        output[2] = x02 + input[2];
        output[3] = x03 + input[3];
        output[4] = x04 + input[4];
        output[5] = x05 + input[5];
        output[6] = x06 + input[6];
        output[7] = x07 + input[7];
        output[8] = x08 + input[8];
        output[9] = x09 + input[9];
        output[10] = x10 + input[10];
        output[11] = x11 + input[11];
        output[12] = x12 + input[12];
        output[13] = x13 + input[13];
        output[14] = x14 + input[14];
        output[15] = x15 + input[15];
    }
    else
    {
        output[0] = x00;
        output[1] = x01;
        output[2] = x02;
        output[3] = x03;
        output[4] = x04;
        output[5] = x05;
        output[6] = x06;
        output[7] = x07;
        output[8] = x08;
        output[9] = x09;
        output[10] = x10;
        output[11] = x11;
        output[12] = x12;
        output[13] = x13;
        output[14] = x14;
        output[15] = x15;
    }
}

struct stream
{
    stream() : rounds(0), offset(0), state(16, 0), block(64, 0)
    {
    }

    ~stream() = default;

    int Intailization(std::vector<unsigned char> key, std::vector<unsigned char> nonce, unsigned char r = 20)
    {
        if (key.size() != 32)
            return -1;
        if (nonce.size() != 24)
            return -2;
        if (r != 20)
            return -3;
        this->init(key, nonce, r);

        // Call HChaCha to derive the subkey using the key and the first 16 bytes
        // of the nonce, and re-initialize the state using the subkey and the
        // remaining nonce.
        //blockArr := (*[stateSize]uint32)(unsafe.Pointer(&s.block));
        core(state, block, this->rounds, true);
        state[4] = block[0];
        state[5] = block[1];
        state[6] = block[2];
        state[7] = block[3];

        state[8] = block[12];
        state[9] = block[13];
        state[10] = block[14];
        state[11] = block[15];

        state[12] = 0;
        state[13] = 0;
        unsigned char *pnonce = nonce.data();
        state[14] = LittleEndian(&pnonce[16]);
        state[15] = LittleEndian(&pnonce[20]);

        advance();

        return 0;
    }

    void XORKeyStream(const std::vector<unsigned char> &src, std::vector<unsigned char> &dst)
    {
        // Stride over the input in 64-byte blocks, minus the amount of keystream
        // previously used. This will produce best results when processing blocks
        // of a size evenly divisible by 64.
        std::vector<unsigned char>::size_type i = 0;
        auto max = src.size();
        while (i < max)
        {
            auto gap = 64 - offset;

            auto limit = i + gap;
            if (limit > max)
            {
                limit = max;
            }

            auto o = offset;
            for (auto j = i; j < limit; j++)
            {
                dst[j] = src[j] ^ block[o];
                o++;
            }

            i += gap;
            offset = o;

            if (o == 64)
            {
                advance();
            }
        }
    }

private:
    unsigned int LittleEndian(unsigned char *b)
    {

        return ((unsigned int)(b[0])) | ((unsigned int)(b[1])) << 8 | ((unsigned int)(b[2])) << 16 | ((unsigned int)(b[3])) << 24;
    }

    bool init(std::vector<unsigned char> &key, std::vector<unsigned char> &nonce, unsigned char r)
    {

        if (nonce.size() != 24)
            return false;
        // the magic constants for 256-bit keys
        state[0] = 0x61707865;
        state[1] = 0x3320646e;
        state[2] = 0x79622d32;
        state[3] = 0x6b206574;

        unsigned char *keyData = key.data();

        state[4] = LittleEndian(&keyData[0]);
        state[5] = LittleEndian(&keyData[4]);
        state[6] = LittleEndian(&keyData[8]);
        state[7] = LittleEndian(&keyData[12]);
        state[8] = LittleEndian(&keyData[16]);
        state[9] = LittleEndian(&keyData[20]);
        state[10] = LittleEndian(&keyData[24]);
        state[11] = LittleEndian(&keyData[28]);

        unsigned char *nonceData = nonce.data();

        state[12] = LittleEndian(&nonceData[0]);
        state[13] = LittleEndian(&nonceData[4]);
        state[14] = LittleEndian(&nonceData[8]);
        state[15] = LittleEndian(&nonceData[12]);

        this->rounds = r;

        return true;
    }

    void advance()
    {
        //core(&s.state, (*[stateSize]uint32)(unsafe.Pointer(&s.block)), s.rounds, false)
        core(state, block, rounds, false);

        const unsigned int blockSize = 64;

        if (bigEndian)
        {
            unsigned int j = blockSize - 1;
            for (unsigned int i = 0; i < blockSize / 2; i++)
            {
                std::swap(block[j], block[i]);
                j--;
            }
        }

        offset = 0;
        unsigned int i = state[12] + 1;
        state[12] = i;
        if (i == 0)
        {
            state[13]++;
        }
    }

private:
    std::vector<unsigned int> state; //16
    std::vector<unsigned int> block; // 64
    int offset;
    unsigned char rounds;
    static bool bigEndian;
};

static bool initBigEndian()
{
    unsigned int x = 0x04030201;
    unsigned char y[4] = {0x1, 0x2, 0x3, 0x4};
    unsigned char *pX = reinterpret_cast<unsigned char *>(&x);
    if (pX[0] != y[0] && pX[1] != y[1] && pX[2] != y[2] && pX[3] != y[3])
    {
        return true;
    }

    return false;
}

bool stream::bigEndian = initBigEndian();

int ChaCha20::Compress(const std::vector<unsigned char> &in,
                       const std::vector<unsigned char> &key,
                       std::vector<unsigned char> &out)
{

    if (key.size() != 32 ) return -1;
    if (in.size() == 0) return -2;
    if (m_IV.size() != 24) return -3;
    out.resize(in.size());
    stream m;
    m.Intailization(key, this->m_IV);
    m.XORKeyStream(in,out);
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
    std::copy(iv.begin(), iv.end(),m_IV.begin());
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