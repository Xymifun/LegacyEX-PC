#include "nn/nex/MD5Checksum.hpp"

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

MD5Checksum::MD5Checksum()
{
}

MD5Checksum::~MD5Checksum()
{
}

void MD5Checksum::ChecksumComputeHelper(const unsigned char** ppData, const unsigned int* pLengths, int iCount,
                                        MD5& rMd5)
{
    for (int i = 0; i < iCount; ++i)
    {
        if (ppData[i] != 0 && pLengths[i] != 0)
            rMd5.update(ppData[i], pLengths[i]);
    }

    rMd5.finalize();
}

bool MD5Checksum::ComputeChecksum(const unsigned char** ppData, const unsigned int* pLengths, int iCount,
                                  SignatureBytes& rSignature)
{
    MD5 md5;
    ChecksumComputeHelper(ppData, pLengths, iCount, md5);
    memcpy(rSignature.m_bytes, md5.GetDigest(), MD5::DigestSize);
    rSignature.m_uiLength = MD5::DigestSize;
    return true;
}

bool MD5Checksum::ComputeChecksum(const Buffer& rBuffer, Buffer* pChecksum)
{
    MD5 md5;
    md5.update(rBuffer.GetData(), rBuffer.GetLength());
    md5.finalize();
    pChecksum->AppendData(md5.GetDigest(), MD5::DigestSize, APPEND_AT_END);
    return true;
}

unsigned int MD5Checksum::ComputeChecksumForTransportArray(const unsigned char** ppData, const unsigned int* pLengths,
                                                           int iCount)
{
    MD5 md5;
    ChecksumComputeHelper(ppData, pLengths, iCount, md5);
    const unsigned char* pDigest = md5.GetDigest();
    unsigned int uiChecksum = pDigest[0] | (pDigest[1] << 8) | (pDigest[2] << 16) | (pDigest[3] << 24);
    return uiChecksum;
}

unsigned char MD5Checksum::GetChecksumLength()
{
    return MD5::DigestSize;
}

}
}
