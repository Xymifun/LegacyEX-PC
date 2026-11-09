#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_BITSTREAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_BITSTREAM_HPP

#include <stdint.h>

#include "nn/nex/Buffer.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class BitStream : public RootObject
{
public:
    BitStream();
    explicit BitStream(Buffer* pBuffer);
    virtual ~BitStream();

    void Init();
    void AdjustLength();
    BitStream& SetPosition(unsigned int uiPosition);
    bool CanExtract(unsigned int uiBits) const;

    void PrepareAppend(unsigned int uiBits);
    void AppendByteImpl(unsigned char uiByte);
    void AppendBitImpl(bool bBit);
    void AppendRaw(const unsigned char* pData, unsigned int uiBits);
    void Append(const unsigned char* pData, unsigned int uiSize, unsigned int uiCount);

    void ExtractByteImpl(unsigned char& rByte);
    void ExtractBitImpl(bool& rBit);
    void ExtractRaw(unsigned char* pData, unsigned int uiBits);
    void Extract(unsigned char* pData, unsigned int uiSize, unsigned int uiCount);

    BitStream& operator<<(const Buffer& rBuffer);
    BitStream& operator>>(Buffer& rBuffer);
    BitStream& operator>>(BitStream& rStream);

    static void TraceBits(uint64_t uiFilter, unsigned char* pData, unsigned int uiBits);

    Buffer* GetBuffer() const { return m_pBuffer; }

protected:
    friend class Ticket;
    friend class KerberosEncryption;
    friend class KerberosAuthentication;
    bool m_bError;
    Buffer* m_pBuffer;
    unsigned int m_uiBytePosition;
    unsigned int m_uiBitPosition;
    unsigned int m_uiBitLength;
};

}
}

#endif
