#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_LITEBYTESTREAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_LITEBYTESTREAM_HPP

#include <stdint.h>

#include "nn/nex/ByteStream.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/LiteBuffer.hpp"

namespace nn
{
namespace nex
{

template <int Capacity>
class LiteByteStream : public RootObject
{
public:
    LiteByteStream()
    {
        m_uiPosition = 0;
        m_bError = false;
    }

    virtual ~LiteByteStream() {}

    LiteBuffer<Capacity>& GetBuffer() { return m_buffer; }

    const LiteBuffer<Capacity>& GetBuffer() const { return m_buffer; }

    void Clear()
    {
        m_buffer.Clear();
        m_bError = false;
        m_uiPosition = 0;
    }

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

        return m_buffer.AppendData(aBytes, sizeof(T), APPEND_AT_END);
    }

private:
    enum
    {
        APPEND_AT_END = 0xFFFFFFFFu
    };

    unsigned int m_uiPosition;
    LiteBuffer<Capacity> m_buffer;
    bool m_bError;
};

}
}

#endif
