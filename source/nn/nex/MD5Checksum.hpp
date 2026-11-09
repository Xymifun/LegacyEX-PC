#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_MD5CHECKSUM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_MD5CHECKSUM_HPP

#include "nn/nex/ChecksumAlgorithm.hpp"

namespace nn
{
namespace nex
{

class MD5;

class MD5Checksum : public ChecksumAlgorithm
{
public:
    MD5Checksum();
    virtual ~MD5Checksum();

    virtual bool ComputeChecksum(const Buffer& rBuffer, Buffer* pChecksum);
    virtual bool ComputeChecksum(const unsigned char** ppData, const unsigned int* pLengths, int iCount,
                                 SignatureBytes& rSignature);
    virtual unsigned int ComputeChecksumForTransportArray(const unsigned char** ppData, const unsigned int* pLengths,
                                                          int iCount);
    virtual unsigned char GetChecksumLength();

private:
    void ChecksumComputeHelper(const unsigned char** ppData, const unsigned int* pLengths, int iCount, MD5& rMd5);
};

}
}

#endif
