#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HMACCHECKSUM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HMACCHECKSUM_HPP

#include "nn/nex/KeyedChecksumAlgorithm.hpp"

namespace nn
{
namespace nex
{

class MD5;

class HMACChecksum : public KeyedChecksumAlgorithm
{
public:
    enum
    {
        PadBlockSize = 0x40
    };

    HMACChecksum();
    virtual ~HMACChecksum();

    virtual bool ComputeChecksum(const Buffer& rBuffer, Buffer* pChecksum);
    virtual bool ComputeChecksum(const unsigned char** ppData, const unsigned int* pLengths, int iCount,
                                 SignatureBytes& rSignature);
    virtual unsigned int ComputeChecksumForTransportArray(const unsigned char** ppData, const unsigned int* pLengths,
                                                          int iCount);
    virtual unsigned char GetChecksumLength() { return 16; }
    virtual void KeyHasChanged();

private:
    void ChecksumComputeHelper(const unsigned char** ppData, const unsigned int* pLengths, int iCount, MD5& rMd5);

    unsigned char m_innerPad[PadBlockSize + 1];
    unsigned char m_outerPad[PadBlockSize + 1];
};

}
}

#endif
