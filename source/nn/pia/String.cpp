#include "nn/pia/String.hpp"

#include <stdint.h>
#include <cstdio>
#include <cstring>

namespace nn
{
namespace pia
{
namespace common
{

String::String(const char* text) : m_bufferSize(0)
{
    SetString(text, 0, EncodingType_Ascii);
}

String::String(const wchar_t* text) : m_bufferSize(0)
{
    SetString(text, 0, EncodingType_Utf16);
}

void String::Clear()
{
    m_text = 0;
    m_bytes = 0;
    m_encoding = EncodingType();
}

Result String::SetString(const void* text, unsigned int, EncodingType encoding)
{
    m_text = text;
    unsigned int bytes = 0;
    m_encoding = encoding;
    if (encoding == EncodingType_Ascii)
    {
        const unsigned char* value = static_cast<const unsigned char*>(text);
        while (*value != 0)
        {
            ++value;
            ++bytes;
        }
    }
    else if (encoding == EncodingType_Utf16)
    {
        const uint16_t* value = static_cast<const uint16_t*>(text);
        while (*value != 0)
        {
            ++value;
            bytes += 2;
        }
    }
    m_bytes = bytes;
    return 0;
}

Result String::Copy(const String& other)
{
    Result result = 0;
    if (!IsSameString(other))
    {
        if (other.GetStringLength() == 0)
        {
            Clear();
            Trace(8);
            return result;
        }
        result = SetString(other.m_text, other.m_bytes, other.m_encoding);
    }
    Trace(8);
    return result;
}

int String::Format(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    return FormatV(format, args);
}

int String::FormatV(const char* format, va_list args)
{
    char* buffer = const_cast<char*>(static_cast<const char*>(m_text));
    if (buffer == 0 || m_bufferSize == 0 || m_bytes != 0 || m_encoding != 0)
        return -1;

    int written = ::vsnprintf(buffer, m_bufferSize, format, args);
    m_encoding = EncodingType_Ascii;
    if (written < 0 || written >= static_cast<int>(m_bufferSize))
    {
        buffer[m_bufferSize - 1] = '\0';
        written = static_cast<int>(m_bufferSize) - 1;
    }
    m_bytes = static_cast<unsigned int>(written);

    unsigned int length = GetStringLength();
    if (length >= (m_bufferSize >> 2))
    {
        length = 0;
        unsigned int index = 0;
        unsigned int code;
        while (length < (m_bufferSize >> 2) - 1 && (code = static_cast<unsigned char>(buffer[index])) != 0)
        {
            if (code - 1 < 0x7f)
            {
                ++length;
                index += 1;
            }
            else if (code - 0xc2 < 0x1e)
            {
                ++length;
                index += 2;
            }
            else if (code - 0xe0 < 0x10)
            {
                ++length;
                index += 3;
            }
            else if (code - 0xf0 < 8)
            {
                ++length;
                index += 4;
            }
            else if (code - 0xf8 < 4)
            {
                ++length;
                index += 5;
            }
            else if (code - 0xfc < 2)
            {
                ++length;
                index += 6;
            }
            else
            {
                index += 1;
            }
        }
        buffer[index] = '\0';
        m_bytes = index;
    }
    return static_cast<int>(length);
}

unsigned int String::CalcBytes(const void* text, EncodingType encoding)
{
    if (text == 0 || encoding == 0)
        return 0;

    unsigned int bytes = 0;
    if (encoding == EncodingType_Utf16)
    {
        const uint16_t* value = static_cast<const uint16_t*>(text);
        while (*value != 0)
        {
            ++value;
            bytes += 2;
        }
    }
    else
    {
        const unsigned char* value = static_cast<const unsigned char*>(text);
        while (*value != 0)
        {
            ++value;
            ++bytes;
        }
    }
    return bytes;
}

const char* String::GetCharArray() const
{
    return static_cast<const char*>(m_text);
}

const void* String::GetBuffer() const
{
    return m_text;
}

unsigned int String::GetBytes() const
{
    return m_bytes;
}

String::EncodingType String::GetEncodingType() const
{
    return m_encoding;
}

unsigned int String::GetStringLength() const
{
    unsigned int length = 0;
    if (m_text != 0 && m_bytes != 0 && m_encoding != 0)
    {
        if (m_encoding == EncodingType_Utf16)
        {
            length = m_bytes >> 1;
        }
        else if (m_encoding == EncodingType_Ascii)
        {
            const unsigned char* value = static_cast<const unsigned char*>(m_text);
            while (*value != 0
                   && static_cast<int>(value - static_cast<const unsigned char*>(m_text)) < static_cast<int>(m_bytes))
            {
                unsigned int code = *value;
                if (code - 1 < 0x7f)
                {
                    ++length;
                    value += 1;
                }
                else if (code - 0xc2 < 0x1e)
                {
                    ++length;
                    value += 2;
                }
                else if (code - 0xe0 < 0x10)
                {
                    ++length;
                    value += 3;
                }
                else if (code - 0xf0 < 8)
                {
                    ++length;
                    value += 4;
                }
                else if (code - 0xf8 < 4)
                {
                    ++length;
                    value += 5;
                }
                else if (code - 0xfc < 2)
                {
                    ++length;
                    value += 6;
                }
                else
                {
                    value += 1;
                }
            }
        }
    }
    return length;
}

bool String::IsSameString(const String& other) const
{
    if (m_text == 0)
    {
        if (other.m_text != 0)
            return false;
    }
    else if (other.m_text == 0)
    {
        return false;
    }

    if (m_encoding != other.m_encoding)
        return false;

    unsigned int bytes = m_bytes;
    if (bytes != other.GetBytes())
        return false;

    if (::memcmp(m_text, other.m_text, bytes) != 0)
        return false;

    return true;
}

void String::Trace(unsigned long long) const
{
}

}
}
}
