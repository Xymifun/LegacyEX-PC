#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CRC16CHECKSUM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CRC16CHECKSUM_HPP

#include "nn/nex/ChecksumAlgorithm.hpp"

namespace nn
{
namespace nex
{

class CRC16Checksum : public ChecksumAlgorithm
{
public:
    CRC16Checksum();
    virtual ~CRC16Checksum();

    virtual bool ComputeChecksum(const Buffer& rBuffer, Buffer* pChecksum);
    virtual bool ComputeChecksum(const unsigned char** ppData, const unsigned int* pLengths, int iCount,
                                 SignatureBytes& rSignature);
    virtual unsigned int ComputeChecksumForTransportArray(const unsigned char** ppData, const unsigned int* pLengths,
                                                          int iCount);
    virtual unsigned char GetChecksumLength() { return 2; }
};

}
}

#endif
