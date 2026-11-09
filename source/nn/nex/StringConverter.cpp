#include "nn/nex/StringConverter.hpp"

#include "nn/nex/SpecialArray.hpp"
#include "nn/nex/StringConversion.hpp"

namespace nn
{
namespace nex
{

StringConverter::StringConverter(int size) : m_buffer(0), m_bufferSize(0)
{
    int bufferSize = size * 3;

    m_bufferSize = static_cast<unsigned int>(bufferSize);
    if (bufferSize > 0)
        m_buffer = SpecialNewArray<char>(static_cast<unsigned int>(bufferSize), L"", 0);
}

StringConverter::~StringConverter()
{
    if (m_buffer != 0)
        SpecialDeleteArray(m_buffer);
}

void StringConverter::Encode(const wchar_t* source, bool addTerminator)
{
    if (m_buffer != 0)
        SpecialDeleteArray(m_buffer);

    m_bufferSize = static_cast<unsigned int>(StringConversion::GetTToUtf8BufferSize(source));
    m_buffer = SpecialNewArray<char>(m_bufferSize, L"", 0);
    StringConversion::TToUtf8(source, m_buffer, m_bufferSize);

    if (addTerminator)
        m_buffer[m_bufferSize - 1] = 0;
}

unsigned int StringConverter::GetBufferSize()
{
    return m_bufferSize;
}

char* StringConverter::GetBuffer()
{
    return m_buffer;
}

void StringConverter::Decode(wchar_t* destination, unsigned short length, bool addTerminator)
{
    StringConversion::Utf8ToT(m_buffer, destination, length);

    if (addTerminator)
        destination[length - 1] = 0;
}

}
}
