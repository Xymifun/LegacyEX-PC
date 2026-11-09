#include "game/core/StringConversion.hpp"

#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <iomanip>
#include <sstream>

#include "nn/nex/StringConversion.hpp"

std::wstring char16towstring(wchar_t* text)
{
    return std::wstring(text);
}

void charTowchar_tArray(const char* text, wchar_t* destination, unsigned int capacity)
{
    bool finished = false;
    wchar_t* start = destination;
    for (;;)
    {
        *destination = *text;
        if (*text == '\0')
            finished = true;
        ++text;
        ++destination;
        if (finished)
            break;
        if (capacity <= (unsigned int)(destination - start))
            return;
    }
}

bool compareStrings(const std::wstring& first, const std::wstring& second)
{
    return std::wcscmp(first.c_str(), second.c_str()) < 0;
}

wchar_t* stringtowchararray(const std::string& text)
{
    static wchar_t buffer[256];

    const std::string::size_type length = text.size();
    for (std::string::size_type index = 0; index < length; ++index)
        buffer[index] = static_cast<signed char>(text[index]);
    buffer[length] = 0;
    return buffer;
}

std::wstring convStringToWstring(wchar_t* text)
{
    return std::wstring(text);
}

std::wstring convStringToWstring(const std::string& text)
{
    const unsigned int length = static_cast<unsigned int>(text.size());
    wchar_t* buffer = new wchar_t[length + 1];
    nn::nex::StringConversion::Utf8ToWideChar(text.c_str(), buffer, length + 1);
    std::wstring result;
    result.assign(buffer, std::wcslen(buffer));
    delete[] buffer;
    return result;
}

std::wstring convUtf8ToWstring(const char* text)
{
    return convStringToWstring(std::string(text));
}

std::string convWstringToString(const std::wstring& text)
{
    char* buffer = new char[text.size() + 1];
    std::string result;
    result.assign(buffer, std::strlen(buffer));
    delete[] buffer;
    return result;
}

template <typename T>
std::wstring _toString(T value)
{
    std::wostringstream stream;
    stream << value;
    return stream.str();
}

template <typename T>
T _fromString(const std::wstring& value)
{
    std::wistringstream stream(value);
    T result = T();
    stream >> result;
    return result;
}

template <typename T>
std::wstring _toHexString(T value)
{
    std::wostringstream stream;
    stream << std::hex << value;
    return stream.str();
}

template <typename T>
T _fromHEXString(const std::wstring& value)
{
    std::wistringstream stream(value);
    T result = T();
    stream >> std::hex >> result;
    return result;
}

template std::wstring _toString<bool>(bool);
template std::wstring _toString<short>(short);
template std::wstring _toString<unsigned short>(unsigned short);
template std::wstring _toString<unsigned char>(unsigned char);
template std::wstring _toString<int>(int);
template std::wstring _toString<unsigned int>(unsigned int);
template std::wstring _toString<float>(float);
template std::wstring _toString<double>(double);
template std::wstring _toString<long long>(long long);
template std::wstring _toString<unsigned long long>(unsigned long long);
template std::wstring _toString<const Direction*>(const Direction*);
template std::wstring _toString<Block*>(Block*);
template std::wstring _toString<const BlockState*>(const BlockState*);

template bool _fromString<bool>(const std::wstring&);
template unsigned short _fromString<unsigned short>(const std::wstring&);
template int _fromString<int>(const std::wstring&);
template unsigned int _fromString<unsigned int>(const std::wstring&);
template float _fromString<float>(const std::wstring&);
template double _fromString<double>(const std::wstring&);
template long long _fromString<long long>(const std::wstring&);

template std::wstring _toHexString<long long>(long long);
template long long _fromHEXString<long long>(const std::wstring&);
template int _fromHEXString<int>(const std::wstring&);
