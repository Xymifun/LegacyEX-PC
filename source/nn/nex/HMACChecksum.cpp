#include "nn/nex/HMACChecksum.hpp"

#include <string.h>

#include "nn/nex/MD5.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int APPEND_AT_END = 0xFFFFFFFFu;
const unsigned char INNER_PAD_XOR = 0x36;
const unsigned char OUTER_PAD_XOR = 0x5C;

}

HMACChecksum::HMACChecksum() : KeyedChecksumAlgorithm(6, 0x40)
{
}

HMACChecksum::~HMACChecksum()
{
}

void HMACChecksum::ChecksumComputeHelper(const unsigned char** ppData, const unsigned int* pLengths, int iCount,
                                         MD5& rMd5)
{
    MD5 inner;
    inner.update(m_innerPad, PadBlockSize);
    for (int i = 0; i < iCount; ++i)
    {
        if (ppData[i] != 0 && pLengths[i] != 0)
            inner.update(ppData[i], pLengths[i]);
    }

    inner.finalize();
    rMd5.update(m_outerPad, PadBlockSize);
    rMd5.update(inner.GetDigest(), MD5::DigestSize);
    rMd5.finalize();
}

bool HMACChecksum::ComputeChecksum(const Buffer& rBuffer, Buffer* pChecksum)
{
    MD5 md5;
    const unsigned char* pData = rBuffer.GetData();
    unsigned int uiLength = rBuffer.GetLength();
    ChecksumComputeHelper(&pData, &uiLength, 1, md5);
    pChecksum->AppendData(md5.GetDigest(), MD5::DigestSize, APPEND_AT_END);
    return true;
}

bool HMACChecksum::ComputeChecksum(const unsigned char** ppData, const unsigned int* pLengths, int iCount,
                                   SignatureBytes& rSignature)
{
    MD5 md5;
    ChecksumComputeHelper(ppData, pLengths, iCount, md5);
    memcpy(rSignature.m_bytes, md5.GetDigest(), MD5::DigestSize);
    rSignature.m_uiLength = MD5::DigestSize;
    return true;
}

unsigned int HMACChecksum::ComputeChecksumForTransportArray(const unsigned char** ppData, const unsigned int* pLengths,
                                                            int iCount)
{
    unsigned int uiChecksum;
    MD5 md5;
    ChecksumComputeHelper(ppData, pLengths, iCount, md5);
    memcpy(&uiChecksum, md5.GetDigest(), sizeof(uiChecksum));
    return (uiChecksum >> 24) | ((uiChecksum >> 8) & 0xFF00) | ((uiChecksum & 0xFF00) << 8) | (uiChecksum << 24);
}

void HMACChecksum::KeyHasChanged()
{
    const unsigned char* pKey = m_key.GetContentPtr();
    int iKeyLength = m_key.GetLength();
    if (iKeyLength > PadBlockSize)
    {
        MD5 md5;
        md5.update(pKey, iKeyLength);
        md5.finalize();
        pKey = md5.GetDigest();
        iKeyLength = MD5::DigestSize;
    }

    memset(m_innerPad, 0, sizeof(m_innerPad));
    memset(m_outerPad, 0, sizeof(m_outerPad));
    memcpy(m_innerPad, pKey, iKeyLength);
    memcpy(m_outerPad, pKey, iKeyLength);
    for (int i = 0; i < PadBlockSize; ++i)
    {
        m_innerPad[i] = m_innerPad[i] ^ INNER_PAD_XOR;
        m_outerPad[i] = m_outerPad[i] ^ OUTER_PAD_XOR;
    }
}

}
}
