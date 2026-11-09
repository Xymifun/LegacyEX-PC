#include "nn/nex/StringConversion.hpp"

#include <cstddef>
#include <cwchar>

#include "nn/nex/SpecialArray.hpp"

namespace nn
{
namespace nex
{
namespace
{

const unsigned char kHexLetterValues[] = {0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

int UnicodeToUtf8(const wchar_t* source, char* destination, unsigned int capacity)
{
    int written = 0;
    unsigned int left = capacity - 1;
    bool unsupported = false;

    while (*source != 0 && left != 0 && !unsupported)
    {
        unsigned int character = static_cast<unsigned int>(*source);

        if (character < 0x80)
        {
            destination[0] = static_cast<char>(character);
            destination += 1;
            left -= 1;
            written += 1;
            ++source;
        }
        else if (character < 0x800)
        {
            destination[0] = static_cast<char>(0xc0 | (character >> 6));
            destination[1] = static_cast<char>(0x80 | (character & 0x3f));
            destination += 2;
            left -= 2;
            written += 2;
            ++source;
        }
        else if (character < 0x10000)
        {
            destination[0] = static_cast<char>(0xe0 | (character >> 12));
            destination[1] = static_cast<char>(0x80 | ((character >> 6) & 0x3f));
            destination[2] = static_cast<char>(0x80 | (character & 0x3f));
            destination += 3;
            left -= 3;
            written += 3;
            ++source;
        }
        else
        {
            ++source;
            unsupported = true;
        }
    }

    *destination = 0;
    return written;
}

int Utf8ToUnicode(const char* source, wchar_t* destination, unsigned int capacity)
{
    int converted = 0;
    unsigned int left = capacity - 1;
    bool invalid = false;
    wchar_t* output = destination;

    do
    {
        unsigned int lead = static_cast<unsigned char>(*source);

        if (lead == 0 || left == 0)
        {
            *output = 0;
            return converted + 1;
        }

        if (lead < 0x80)
        {
            *output = static_cast<wchar_t>(static_cast<char>(lead));
            ++output;
            --left;
            ++converted;
        }
        else if (lead - 0xc2 < 0x1e)
        {
            unsigned int trail = static_cast<unsigned char>(source[1]);

            *output = static_cast<wchar_t>(lead * 0x40 + trail - 0x3080);
            ++output;
            ++source;
            --left;
            ++converted;
        }
        else if (lead - 0xe0 < 0x10)
        {
            unsigned int second = static_cast<unsigned char>(source[1]);
            unsigned int third = static_cast<unsigned char>(source[2]);

            *output = static_cast<wchar_t>(lead * 0x1000 + second * 0x40 + third - 0x2080);
            ++output;
            source += 2;
            --left;
            ++converted;
        }
        else
        {
            invalid = true;
        }

        ++source;
    } while (!invalid);

    *destination = 0;
    return -1;
}

int GetNeededUtf8BufferSize(const wchar_t* source)
{
    int size = 1;

    while (*source != 0)
    {
        unsigned int character = static_cast<unsigned int>(*source);

        if (character < 0x80)
            size += 1;
        else if (character < 0x800)
            size += 2;
        else
            size += 3;

        ++source;
    }

    return size;
}

}

int StringConversion::T2Char8(const wchar_t* source, char* destination, unsigned int capacity)
{
    return UnicodeToUtf8(source, destination, capacity);
}

int StringConversion::TToUtf8(const wchar_t* source, char* destination, unsigned int capacity)
{
    return UnicodeToUtf8(source, destination, capacity);
}

int StringConversion::Char8_2T(const char* source, wchar_t* destination, unsigned int capacity)
{
    return Utf8ToUnicode(source, destination, capacity);
}

int StringConversion::Utf8ToT(const char* source, wchar_t* destination, unsigned int capacity)
{
    return Utf8ToUnicode(source, destination, capacity);
}

int StringConversion::Utf8ToWideChar(const char* source, wchar_t* destination, unsigned int capacity)
{
    return Utf8ToUnicode(source, destination, capacity);
}

int StringConversion::GetTToUtf8BufferSize(const wchar_t* source)
{
    return GetNeededUtf8BufferSize(source);
}

void StringConversion::T2Char8Alloc(const wchar_t* source, char** destination)
{
    std::size_t length = std::wcslen(source);
    char* buffer = SpecialNewArray<char>(length + 1, L"", 0);

    *destination = buffer;
    UnicodeToUtf8(source, buffer, length + 1);
}

void StringConversion::T2Char8Release(char* buffer)
{
    SpecialDeleteArray(buffer);
}

unsigned char StringConversion::GetCharValue(wchar_t character)
{
    if (character >= L'A' && character <= L'F')
        return kHexLetterValues[character - L'A'];

    return static_cast<unsigned char>(character - L'0');
}

void StringConversion::StringToHex(const wchar_t* source, unsigned char* destination, unsigned int count)
{
    unsigned int index;

    for (index = 0; index < count; ++index)
    {
        destination[index] = static_cast<unsigned char>((GetCharValue(source[index * 2]) << 4)
                                                        | GetCharValue(source[index * 2 + 1]));
    }
}

int StringConversion::HexToString(const unsigned char* source, wchar_t* destination, unsigned int count)
{
    int written = 0;
    unsigned int index;

    for (index = 0; index < count; ++index)
        written += std::swprintf(destination + written, 3, L"%02X", source[index]);

    return written;
}

}
}
