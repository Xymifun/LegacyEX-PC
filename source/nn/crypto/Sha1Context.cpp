#include "nn/crypto/Sha1Context.hpp"

#include "nn/dbg/Break.hpp"

#include <stdint.h>

namespace nn
{
namespace crypto
{

namespace
{

const unsigned int InitialState0 = 0x67452301;
const unsigned int InitialState1 = 0xEFCDAB89;
const unsigned int InitialState2 = 0x98BADCFE;
const unsigned int InitialState3 = 0x10325476;
const unsigned int InitialState4 = 0xC3D2E1F0;

const unsigned int RoundConstant0 = 0x5A827999;
const unsigned int RoundConstant1 = 0x6ED9EBA1;
const unsigned int RoundConstant2 = 0x8F1BBCDC;
const unsigned int RoundConstant3 = 0xCA62C1D6;

inline unsigned int RotateLeft(unsigned int uiValue, unsigned int uiShift)
{
    return (uiValue << uiShift) | (uiValue >> (32 - uiShift));
}

}

void Sha1Context::Initialize()
{
    m_blockSize = 0;
    m_blockCount = 0;
    m_blockCountHigh = 0;
    m_state[0] = InitialState0;
    m_state[1] = InitialState1;
    m_state[2] = InitialState2;
    m_state[3] = InitialState3;
    m_state[4] = InitialState4;
}

void Sha1Context::InitializeWithContext(const void* pContext, unsigned long long uiSize)
{
    const unsigned int* pState = static_cast<const unsigned int*>(pContext);
    const unsigned long long uiBlockCount = uiSize / BlockSize;
    m_blockCountHigh = static_cast<unsigned int>(uiBlockCount >> 32);
    m_blockCount = static_cast<unsigned int>(uiBlockCount);
    m_blockSize = 0;
    m_state[0] = pState[0];
    m_state[1] = pState[1];
    m_state[2] = pState[2];
    m_state[3] = pState[3];
    m_state[4] = pState[4];
}

void Sha1Context::Update(const void* pData, unsigned int uiSize)
{
    if (pData == 0)
    {
        nndbgBreak(NN_DBG_BREAK_REASON_ASSERT);
    }
    if (uiSize == 0)
    {
        nndbgBreak(NN_DBG_BREAK_REASON_ASSERT);
    }
    ShaBlock512BitContext::Update(pData, uiSize);
}

void Sha1Context::GetHash(void* pHash)
{
    if (pHash == 0 || (reinterpret_cast<uintptr_t>(pHash) & 3) != 0)
    {
        nndbgBreak(NN_DBG_BREAK_REASON_ASSERT);
    }
    AddPadding();
    unsigned int* pOut = static_cast<unsigned int*>(pHash);
    pOut[0] = m_state[0];
    pOut[1] = m_state[1];
    pOut[2] = m_state[2];
    pOut[3] = m_state[3];
    pOut[4] = m_state[4];
}

void Sha1Context::ProcessBlock()
{
    unsigned int a = m_state[0];
    unsigned int b = m_state[1];
    unsigned int c = m_state[2];
    unsigned int d = m_state[3];
    unsigned int e = m_state[4];
    unsigned int words[BlockWordCount];
    int t;

    for (t = 0; t < BlockWordCount; ++t)
    {
        words[t] = m_block.words[t];
    }

    for (t = 0; t < 20; ++t)
    {
        if (t >= 16)
        {
            const unsigned int x = words[(t - 16) & 15] ^ words[(t - 14) & 15] ^ words[(t - 8) & 15]
                                   ^ words[(t - 3) & 15];
            words[t & 15] = RotateLeft(x, 1);
        }
        const unsigned int temp = ((b & c) | (~b & d)) + RoundConstant0 + words[t & 15] + RotateLeft(a, 5) + e;
        e = d;
        d = c;
        c = RotateLeft(b, 30);
        b = a;
        a = temp;
    }
    for (; t < 40; ++t)
    {
        if (t >= 16)
        {
            const unsigned int x = words[(t - 16) & 15] ^ words[(t - 14) & 15] ^ words[(t - 8) & 15]
                                   ^ words[(t - 3) & 15];
            words[t & 15] = RotateLeft(x, 1);
        }
        const unsigned int temp = (b ^ c ^ d) + RoundConstant1 + words[t & 15] + RotateLeft(a, 5) + e;
        e = d;
        d = c;
        c = RotateLeft(b, 30);
        b = a;
        a = temp;
    }
    for (; t < 60; ++t)
    {
        if (t >= 16)
        {
            const unsigned int x = words[(t - 16) & 15] ^ words[(t - 14) & 15] ^ words[(t - 8) & 15]
                                   ^ words[(t - 3) & 15];
            words[t & 15] = RotateLeft(x, 1);
        }
        const unsigned int temp = ((b & (c | d)) | (c & d)) + RoundConstant2 + words[t & 15] + RotateLeft(a, 5) + e;
        e = d;
        d = c;
        c = RotateLeft(b, 30);
        b = a;
        a = temp;
    }
    for (; t < 80; ++t)
    {
        if (t >= 16)
        {
            const unsigned int x = words[(t - 16) & 15] ^ words[(t - 14) & 15] ^ words[(t - 8) & 15]
                                   ^ words[(t - 3) & 15];
            words[t & 15] = RotateLeft(x, 1);
        }
        const unsigned int temp = (b ^ c ^ d) + RoundConstant3 + words[t & 15] + RotateLeft(a, 5) + e;
        e = d;
        d = c;
        c = RotateLeft(b, 30);
        b = a;
        a = temp;
    }

    m_state[0] += a;
    m_state[1] += b;
    m_state[2] += c;
    m_state[3] += d;
    m_state[4] += e;
}

void CalculateSha1(void* pHash, const void* pData, unsigned int uiSize)
{
    Sha1Context context;
    context.Initialize();
    context.Update(pData, uiSize);
    context.GetHash(pHash);
}

}
}
