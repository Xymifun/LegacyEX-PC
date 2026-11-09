#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_BYTESTREAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_BYTESTREAM_HPP

#include <stdint.h>

#include "nn/nex/Buffer.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Time;

inline void SetUnalignedNexNetworkOrder16(uint16_t uiValue, unsigned char* pDestination)
{
    pDestination[0] = static_cast<unsigned char>(uiValue);
    pDestination[1] = static_cast<unsigned char>(uiValue >> 8);
}

inline void SetUnalignedNexNetworkOrder32(uint32_t uiValue, unsigned char* pDestination)
{
    pDestination[0] = static_cast<unsigned char>(uiValue);
    pDestination[1] = static_cast<unsigned char>(uiValue >> 8);
    pDestination[2] = static_cast<unsigned char>(uiValue >> 16);
    pDestination[3] = static_cast<unsigned char>(uiValue >> 24);
}

void SetUnalignedNexNetworkOrder64(uint64_t uiValue, unsigned char* pDestination);

template <unsigned int uiSize>
struct PrimitiveReader;

template <>
struct PrimitiveReader<sizeof(uint8_t)>
{
    template <typename T>
    static void Read(const unsigned char* pSource, T* pValue)
    {
        *reinterpret_cast<uint8_t*>(pValue) = pSource[0];
    }
};

template <>
struct PrimitiveReader<sizeof(uint16_t)>
{
    template <typename T>
    static void Read(const unsigned char* pSource, T* pValue)
    {
        *reinterpret_cast<uint16_t*>(pValue) = static_cast<uint16_t>(pSource[0] | (pSource[1] << 8));
    }
};

template <>
struct PrimitiveReader<sizeof(uint32_t)>
{
    template <typename T>
    static void Read(const unsigned char* pSource, T* pValue)
    {
        *reinterpret_cast<uint32_t*>(pValue) = static_cast<uint32_t>(pSource[0])
                                               | (static_cast<uint32_t>(pSource[1]) << 8)
                                               | (static_cast<uint32_t>(pSource[2]) << 16)
                                               | (static_cast<uint32_t>(pSource[3]) << 24);
    }
};

template <>
struct PrimitiveReader<sizeof(uint64_t)>
{
    template <typename T>
    static void Read(const unsigned char* pSource, T* pValue)
    {
        *reinterpret_cast<uint64_t*>(
            pValue) = static_cast<uint64_t>(pSource[0]) | (static_cast<uint64_t>(pSource[1]) << 8)
                      | (static_cast<uint64_t>(pSource[2]) << 16) | (static_cast<uint64_t>(pSource[3]) << 24)
                      | (static_cast<uint64_t>(pSource[4]) << 32) | (static_cast<uint64_t>(pSource[5]) << 40)
                      | (static_cast<uint64_t>(pSource[6]) << 48) | (static_cast<uint64_t>(pSource[7]) << 56);
    }
};

class ByteStream : public RootObject
{
public:
    ByteStream();
    explicit ByteStream(unsigned int uiSize);
    explicit ByteStream(Buffer* pBuffer);
    virtual ~ByteStream();

    void Init();
    void Clear();

    bool SetLength(unsigned int uiLength);
    bool ValidateBufferLimit(unsigned int uiSize);

    bool AppendRaw(const unsigned char* pData, unsigned int uiSize);
    bool Append(const unsigned char* pData, unsigned int uiSize, unsigned int uiCount);
    bool Append(const Time* pTime);
    bool AppendString(const wchar_t* pString, unsigned int uiSize, unsigned int uiCount);
    bool AppendString(const wchar_t* pString);
    bool AppendString(const wchar_t* pString, unsigned int uiLength);

    bool ExtractRaw(unsigned char* pData, unsigned int uiSize);
    bool Extract(unsigned char* pData, unsigned int uiSize, unsigned int uiCount);
    void Extract(Time* pTime);
    bool ExtractString(wchar_t* pString, unsigned short uiLength);

    bool CanAppend(unsigned int uiSize) const;
    bool CanExtract(unsigned int uiSize) const;

    bool operator==(const ByteStream& rStream) const;
    ByteStream& operator<<(const Buffer& rBuffer);
    ByteStream& operator<<(const bool& rValue);
    ByteStream& operator>>(Buffer& rBuffer);
    ByteStream& operator>>(ByteStream& rStream);
    ByteStream& operator>>(bool& rValue);

    static void TraceBytes(uint64_t uiFilter, unsigned char* pData, unsigned int uiSize);
    void Trace(uint64_t uiFilter, bool bDumpContents);

    template <typename T>
    bool AppendPrimitive(const T& rValue)
    {
        unsigned char aBytes[sizeof(uint64_t)];

        switch (sizeof(T))
        {
        case sizeof(uint8_t):
            aBytes[0] = *reinterpret_cast<const uint8_t*>(&rValue);
            break;
        case sizeof(uint16_t):
            SetUnalignedNexNetworkOrder16(*reinterpret_cast<const uint16_t*>(&rValue), aBytes);
            break;
        case sizeof(uint32_t):
            SetUnalignedNexNetworkOrder32(*reinterpret_cast<const uint32_t*>(&rValue), aBytes);
            break;
        case sizeof(uint64_t):
            SetUnalignedNexNetworkOrder64(*reinterpret_cast<const uint64_t*>(&rValue), aBytes);
            break;
        }

        return m_pBuffer->AppendData(aBytes, sizeof(T), APPEND_AT_END);
    }

    template <typename T>
    bool ExtractPrimitive(T* pValue)
    {
        if (!ValidateBufferLimit(sizeof(T)))
        {
            *pValue = 0;
            return false;
        }

        const unsigned char* pSource = m_pBuffer->GetData() + m_uiPosition;

        PrimitiveReader<sizeof(T)>::Read(pSource, pValue);

        m_uiPosition += sizeof(T);
        return true;
    }

    Buffer* GetBuffer() const { return m_pBuffer; }

    bool HasError() const { return m_bError; }

    void SetError() { m_bError = true; }

    unsigned int GetPosition() const { return m_uiPosition; }

    void SetPosition(unsigned int uiPosition) { m_uiPosition = uiPosition; }

    unsigned int GetLength() const { return m_pBuffer->GetLength(); }

protected:
    enum
    {
        APPEND_AT_END = 0xFFFFFFFFu
    };

    bool m_bError;
    Buffer* m_pBuffer;
    unsigned int m_uiPosition;
};

}
}

#endif
