#include "nn/pia/common/Md5Context.hpp"

#include <coreinit/memory.h>

namespace nn
{
namespace pia
{
namespace common
{

namespace
{

const unsigned int cInitialStateA = 0x67452301;
const unsigned int cInitialStateB = 0xefcdab89;
const unsigned int cInitialStateC = 0x98badcfe;
const unsigned int cInitialStateD = 0x10325476;

const unsigned int cRoundConstants[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

const unsigned int cWordOrder[48] = {1, 6, 11, 0,  5,  10, 15, 4,  9,  14, 3, 8,  13, 2,  7,  12,
                                     5, 8, 11, 14, 1,  4,  7,  10, 13, 0,  3, 6,  9,  12, 15, 2,
                                     0, 7, 14, 5,  12, 3,  10, 1,  8,  15, 6, 13, 4,  11, 2,  9};

unsigned char sPadding[64] = {0x80};

inline unsigned int SwapBytes(unsigned int value)
{
    value = ((value & 0xff00ff00) >> 8) | ((value & 0x00ff00ff) << 8);
    return (value << 16) | (value >> 16);
}

inline unsigned long long SwapBytes(unsigned long long value)
{
    value = ((value & 0x00ff00ff00ff00ffULL) << 8) | ((value & 0xff00ff00ff00ff00ULL) >> 8);
    value = ((value & 0x0000ffff0000ffffULL) << 16) | ((value & 0xffff0000ffff0000ULL) >> 16);
    return (value << 32) | (value >> 32);
}

inline unsigned int RotateLeft(unsigned int value, unsigned int shift)
{
    return (value << shift) | (value >> (32 - shift));
}

}

void Md5Context::Initialize()
{
    m_state[0] = cInitialStateA;
    m_state[1] = cInitialStateB;
    m_state[2] = cInitialStateC;
    m_state[3] = cInitialStateD;
    m_length = 0;
}

void Md5Context::Update(const void* data, unsigned int size)
{
    unsigned int offset = m_length & (cBlockSize - 1);
    unsigned int room = cBlockSize - offset;
    m_length += size;

    const unsigned char* source = static_cast<const unsigned char*>(data);
    if (room > size)
    {
        if (size != 0)
            OSBlockMove(m_block.bytes + offset, source, size, FALSE);
        return;
    }

    OSBlockMove(m_block.bytes + offset, source, room, FALSE);
    ProcessBlock();
    source += room;
    size -= room;

    int blockNum = size / cBlockSize;
    for (int i = 0; i < blockNum; ++i)
    {
        OSBlockMove(m_block.bytes, source, cBlockSize, FALSE);
        source += cBlockSize;
        ProcessBlock();
    }

    size &= cBlockSize - 1;
    if (size != 0)
        OSBlockMove(m_block.bytes, source, size, FALSE);
}

void Md5Context::ProcessBlock()
{
    unsigned int a = m_state[0];
    unsigned int b = m_state[1];
    unsigned int c = m_state[2];
    unsigned int d = m_state[3];
    const unsigned int* block = m_block.words;
    const unsigned int* constant = cRoundConstants;
    const unsigned int* word = block;

    for (int i = 0; i < 4; ++i)
    {
        a = b + RotateLeft(a + ((b & c) | (d & ~b)) + SwapBytes(*word++) + *constant++, 7);
        d = a + RotateLeft(d + ((a & b) | (c & ~a)) + SwapBytes(*word++) + *constant++, 12);
        c = d + RotateLeft(c + ((d & a) | (b & ~d)) + SwapBytes(*word++) + *constant++, 17);
        b = c + RotateLeft(b + ((c & d) | (a & ~c)) + SwapBytes(*word++) + *constant++, 22);
    }

    const unsigned int* order = cWordOrder;
    for (int i = 0; i < 4; ++i)
    {
        a = b + RotateLeft(a + ((b & d) | (c & ~d)) + SwapBytes(block[*order++]) + *constant++, 5);
        d = a + RotateLeft(d + ((a & c) | (b & ~c)) + SwapBytes(block[*order++]) + *constant++, 9);
        c = d + RotateLeft(c + ((d & b) | (a & ~b)) + SwapBytes(block[*order++]) + *constant++, 14);
        b = c + RotateLeft(b + ((c & a) | (d & ~a)) + SwapBytes(block[*order++]) + *constant++, 20);
    }

    for (int i = 0; i < 4; ++i)
    {
        a = b + RotateLeft(a + (b ^ c ^ d) + SwapBytes(block[*order++]) + *constant++, 4);
        d = a + RotateLeft(d + (a ^ b ^ c) + SwapBytes(block[*order++]) + *constant++, 11);
        c = d + RotateLeft(c + (d ^ a ^ b) + SwapBytes(block[*order++]) + *constant++, 16);
        b = c + RotateLeft(b + (c ^ d ^ a) + SwapBytes(block[*order++]) + *constant++, 23);
    }

    for (int i = 0; i < 4; ++i)
    {
        a = b + RotateLeft(a + (c ^ (b | ~d)) + SwapBytes(block[*order++]) + *constant++, 6);
        d = a + RotateLeft(d + (b ^ (a | ~c)) + SwapBytes(block[*order++]) + *constant++, 10);
        c = d + RotateLeft(c + (a ^ (d | ~b)) + SwapBytes(block[*order++]) + *constant++, 15);
        b = c + RotateLeft(b + (d ^ (c | ~a)) + SwapBytes(block[*order++]) + *constant++, 21);
    }

    m_state[0] += a;
    m_state[1] += b;
    m_state[2] += c;
    m_state[3] += d;
}

void Md5Context::GetHash(void* hash)
{
    unsigned long long bitLength = m_length << 3;
    Update(sPadding, 1);

    unsigned int offset = m_length & (cBlockSize - 1);
    unsigned int room = cBlockSize - offset;
    if (room < cLengthSize)
    {
        OSBlockSet(m_block.bytes + offset, 0, room);
        ProcessBlock();
        offset = 0;
        room = cBlockSize;
    }
    if (room > cLengthSize)
        OSBlockSet(m_block.bytes + offset, 0, room - cLengthSize);

    unsigned long long swappedLength = SwapBytes(bitLength);
    m_block.words[cBlockWordNum - 2] = static_cast<unsigned int>(swappedLength >> 32);
    m_block.words[cBlockWordNum - 1] = static_cast<unsigned int>(swappedLength);
    ProcessBlock();

    unsigned int* output = static_cast<unsigned int*>(hash);
    for (int i = 0; i < cStateWordNum; ++i)
        output[i] = SwapBytes(m_state[i]);
}

unsigned int Md5Context::GetHashSize() const
{
    return cHashSize;
}

unsigned int Md5Context::GetBlockSize() const
{
    return cBlockSize;
}

}
}
}
