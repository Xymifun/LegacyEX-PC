#include "nn/pia/common/Sha256Context.hpp"

namespace nn
{
namespace pia
{
namespace common
{

namespace
{
unsigned int s_roundConstants[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

inline unsigned int rotateRight(unsigned int value, unsigned int count)
{
    return (value << (32 - count)) | (value >> count);
}
}

void Sha256Context::Initialize()
{
    m_blockCount = 0;
    m_blockCountHigh = 0;
    m_blockSize = 0;
    m_state[0] = 0x6a09e667;
    m_state[1] = 0xbb67ae85;
    m_state[2] = 0x3c6ef372;
    m_state[3] = 0xa54ff53a;
    m_state[4] = 0x510e527f;
    m_state[5] = 0x9b05688c;
    m_state[6] = 0x1f83d9ab;
    m_state[7] = 0x5be0cd19;
}

void Sha256Context::Update(const void* data, unsigned int size)
{
    ShaBlock512BitContext::Update(data, size);
}

void Sha256Context::ProcessBlock()
{
    unsigned int a = m_state[0];
    unsigned int b = m_state[1];
    unsigned int c = m_state[2];
    unsigned int d = m_state[3];
    unsigned int e = m_state[4];
    unsigned int f = m_state[5];
    unsigned int g = m_state[6];
    unsigned int h = m_state[7];

    unsigned int words[64];
    int index;
    for (index = 0; index < 16; ++index)
        words[index] = reinterpret_cast<const unsigned int*>(m_block)[index];
    for (; index < 64; ++index)
    {
        words[index] =
            words[index - 16]
            + (rotateRight(words[index - 15], 7) ^ rotateRight(words[index - 15], 18) ^ (words[index - 15] >> 3))
            + words[index - 7]
            + (rotateRight(words[index - 2], 17) ^ rotateRight(words[index - 2], 19) ^ (words[index - 2] >> 10));
    }

    for (index = 0; index < 64; ++index)
    {
        const unsigned int temp1 = h + (rotateRight(e, 6) ^ rotateRight(e, 11) ^ rotateRight(e, 25))
                                   + ((e & f) ^ (g & ~e)) + s_roundConstants[index] + words[index];
        const unsigned int temp2 = (rotateRight(a, 2) ^ rotateRight(a, 13) ^ rotateRight(a, 22))
                                   + ((a & (b ^ c)) ^ (b & c));
        h = g;
        g = f;
        f = e;
        e = d + temp1;
        d = c;
        c = b;
        b = a;
        a = temp1 + temp2;
    }
    m_state[0] += a;
    m_state[1] += b;
    m_state[2] += c;
    m_state[3] += d;
    m_state[4] += e;
    m_state[5] += f;
    m_state[6] += g;
    m_state[7] += h;
}

void Sha256Context::GetHash(void* hash)
{
    AddPadding();
    unsigned int* output = static_cast<unsigned int*>(hash);
    output[0] = m_state[0];
    output[1] = m_state[1];
    output[2] = m_state[2];
    output[3] = m_state[3];
    output[4] = m_state[4];
    output[5] = m_state[5];
    output[6] = m_state[6];
    output[7] = m_state[7];
}

unsigned int Sha256Context::GetHashSize() const
{
    return 32;
}

}
}
}
