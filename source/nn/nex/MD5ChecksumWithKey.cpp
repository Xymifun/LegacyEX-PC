#include "nn/nex/MD5ChecksumWithKey.hpp"

#include <string.h>

#include "nn/nex/MD5.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int APPEND_AT_END = 0xFFFFFFFFu;

}

MD5ChecksumWithKey::MD5ChecksumWithKey() : KeyedChecksumAlgorithm(6, 0x40)
{
}

MD5ChecksumWithKey::~MD5ChecksumWithKey()
{
}

void MD5ChecksumWithKey::ChecksumComputeHelper(const unsigned char** ppData, const unsigned int* pLengths, int iCount,
                                               MD5& rMd5)
{
    int iKeyLength = m_key.GetLength();
    for (int i = 0; i < iCount; ++i)
    {
        if (ppData[i] != 0 && pLengths[i] != 0)
            rMd5.update(ppData[i], pLengths[i]);
    }

    if (iKeyLength != 0)
        rMd5.update(m_key.GetContentPtr(), iKeyLength);

    rMd5.finalize();
}

bool MD5ChecksumWithKey::ComputeChecksum(const unsigned char** ppData, const unsigned int* pLengths, int iCount,
                                         SignatureBytes& rSignature)
{
    MD5 md5;
    ChecksumComputeHelper(ppData, pLengths, iCount, md5);
    memcpy(rSignature.m_bytes, md5.GetDigest(), MD5::DigestSize);
    rSignature.m_uiLength = MD5::DigestSize;
    return true;
}

bool MD5ChecksumWithKey::ComputeChecksum(const Buffer& rBuffer, Buffer* pChecksum)
{
    MD5 md5;
    const unsigned char* pData = rBuffer.GetData();
    unsigned int uiLength = rBuffer.GetLength();
    ChecksumComputeHelper(&pData, &uiLength, 1, md5);
    pChecksum->AppendData(md5.GetDigest(), MD5::DigestSize, APPEND_AT_END);
    return true;
}

unsigned int MD5ChecksumWithKey::ComputeChecksumForTransportArray(const unsigned char** ppData,
                                                                  const unsigned int* pLengths, int iCount)
{
    MD5 md5;
    ChecksumComputeHelper(ppData, pLengths, iCount, md5);
    const unsigned char* pDigest = md5.GetDigest();
    unsigned int uiChecksum = pDigest[0] | (pDigest[1] << 8) | (pDigest[2] << 16) | (pDigest[3] << 24);
    return uiChecksum;
}

}
}
