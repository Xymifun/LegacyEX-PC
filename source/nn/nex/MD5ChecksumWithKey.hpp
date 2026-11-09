#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_MD5CHECKSUMWITHKEY_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_MD5CHECKSUMWITHKEY_HPP

#include "nn/nex/KeyedChecksumAlgorithm.hpp"

namespace nn
{
namespace nex
{

class MD5;

class MD5ChecksumWithKey : public KeyedChecksumAlgorithm
{
public:
    MD5ChecksumWithKey();
    virtual ~MD5ChecksumWithKey();

    virtual bool ComputeChecksum(const Buffer& rBuffer, Buffer* pChecksum);
    virtual bool ComputeChecksum(const unsigned char** ppData, const unsigned int* pLengths, int iCount,
                                 SignatureBytes& rSignature);
    virtual bool IsReady() const { return true; }
    virtual unsigned int ComputeChecksumForTransportArray(const unsigned char** ppData, const unsigned int* pLengths,
                                                          int iCount);
    virtual unsigned char GetChecksumLength() { return 16; }

private:
    void ChecksumComputeHelper(const unsigned char** ppData, const unsigned int* pLengths, int iCount, MD5& rMd5);
};

}
}

#endif
