#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_LITEBUFFER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_LITEBUFFER_HPP

#include <cstring>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/SystemError.hpp"

namespace nn
{
namespace nex
{

template <int Capacity>
class LiteBuffer : public RootObject
{
public:
    LiteBuffer() : m_length(0) {}

    virtual ~LiteBuffer() {}

    bool AppendData(const void* data, unsigned int size, unsigned int offset)
    {
        if (size == 0)
        {
            return true;
        }

        unsigned int length = m_length;
        if (offset == 0xFFFFFFFFu)
        {
            offset = length;
        }

        if (offset > length || offset + size > length)
        {
            length = offset + size;
        }

        if (length > static_cast<unsigned int>(Capacity))
        {
            SystemError::SignalError(
                L"D:\\home\\jenkins\\workspace\\Master\\Pack\\d4f027b8\\.\\OnlineCore/src/Core/LiteBuffer.h", 79,
                0xE0000019u, 0);
            return false;
        }

        m_length = length;
        std::memcpy(m_data + offset, data, size);
        return true;
    }

    unsigned char* GetData() { return m_data; }

    const unsigned char* GetData() const { return m_data; }

    unsigned int GetLength() const { return m_length; }

    void Clear() { m_length = 0; }

private:
    unsigned int m_length;
    unsigned char m_data[Capacity];
};

}
}

#endif
