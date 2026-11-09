#include "nn/nex/String.hpp"

#include <cstdarg>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cwchar>

#include "nn/nex/SpecialArray.hpp"
#include "nn/nex/StringConversion.hpp"

namespace nn
{
namespace nex
{

void CopyString(wchar_t** destination, const wchar_t* source)
{
    std::size_t length;

    if (source == 0)
    {
        *destination = 0;
        return;
    }

    length = std::wcslen(source);
    *destination = SpecialNewArray<wchar_t>(static_cast<unsigned int>(length + 1), L"", 0);
    std::wcscpy(*destination, source);
}

void CopyString(wchar_t** destination, const char* source)
{
    unsigned int length;

    if (source == 0)
    {
        *destination = 0;
        return;
    }

    length = static_cast<unsigned int>(std::strlen(source) + 1);
    *destination = SpecialNewArray<wchar_t>(length, L"", 0);
    StringConversion::Utf8ToWideChar(source, *destination, length);
}

void CopyString(char** destination, const wchar_t* source)
{
    std::size_t length;

    if (source == 0)
    {
        *destination = 0;
        return;
    }

    length = std::wcslen(source);
    *destination = SpecialNewArray<char>(static_cast<unsigned int>(length + 1), L"", 0);
    std::wcstombs(*destination, source, length + 1);
}

String::String()
{
    CopyString(&m_data, L"");
}

String::String(const wchar_t* source)
{
    CopyString(&m_data, source);
}

String::String(const char* source)
{
    CopyString(&m_data, source);
}

String::String(const String& other)
{
    CopyString(&m_data, other.m_data);
}

String::~String()
{
    if (m_data != 0)
        SpecialDeleteArray(m_data);
}

String& String::operator=(const wchar_t* source)
{
    if (m_data != source)
    {
        if (m_data != 0)
            SpecialDeleteArray(m_data);

        CopyString(&m_data, source);
    }

    return *this;
}

String& String::operator=(const char* source)
{
    if (m_data != 0)
        SpecialDeleteArray(m_data);

    CopyString(&m_data, source);
    return *this;
}

String& String::operator=(const String& other)
{
    if (m_data != other.m_data)
    {
        if (m_data != 0)
            SpecialDeleteArray(m_data);

        CopyString(&m_data, other.m_data);
    }

    return *this;
}

String& String::operator+=(const String& other)
{
    return *this = *this + other;
}

bool String::operator<(const String& other) const
{
    if (m_data == other.m_data)
        return false;

    if (m_data == 0)
        return String(L"") < other;

    if (other.m_data == 0)
        return *this < String(L"");

    return std::wcscmp(m_data, other.m_data) < 0;
}

String& String::Format(const wchar_t* format, ...)
{
    wchar_t buffer[256];
    va_list arguments;

    va_start(arguments, format);
    std::vswprintf(buffer, 256, format, arguments);
    va_end(arguments);

    return *this = buffer;
}

unsigned int String::GetLength() const
{
    if (m_data == 0)
        return 0;

    return static_cast<unsigned int>(std::wcslen(m_data));
}

void String::Reserve(int size)
{
    if (m_data != 0)
        SpecialDeleteArray(m_data);

    m_data = SpecialNewArray<wchar_t>(static_cast<unsigned int>(size), L"", 0);
    m_data[0] = 0;
}

String String::Truncate(unsigned int length) const
{
    String truncated;
    unsigned int available = GetLength();

    if (available < length)
        length = available;

    truncated.Reserve(static_cast<int>(length + 1));
    std::memcpy(truncated.m_data, m_data, length * sizeof(wchar_t));
    truncated.m_data[length] = 0;

    return truncated;
}

void String::CreateCopy(char** destination) const
{
    CopyString(destination, m_data);
}

void String::ReleaseCopy(char* copy)
{
    if (copy == 0)
        return;

    SpecialDeleteArray(copy);
}

void String::ToUpper()
{
    wchar_t* character = m_data;

    if (character == 0)
        return;

    while (*character != 0)
    {
        if (*character >= L'a' && *character <= L'z')
            *character = static_cast<wchar_t>(*character - 0x20);

        ++character;
    }
}

void String::ToLower()
{
    wchar_t* character = m_data;

    if (character == 0)
        return;

    while (*character != 0)
    {
        if (*character >= L'A' && *character <= L'Z')
            *character = static_cast<wchar_t>(*character + 0x20);

        ++character;
    }
}

int String::FindSubstringCase(const wchar_t* substring, int offset) const
{
    const wchar_t* found;

    if (m_data == 0 || substring == 0 || m_data[0] == 0 || substring[0] == 0 || offset < 0)
        return -1;

    if (offset >= static_cast<int>(std::wcslen(m_data)))
        return -1;

    found = std::wcsstr(m_data + offset, substring);
    if (found == 0)
        return -1;

    return static_cast<int>(found - m_data);
}

int String::FindSubstringNoCase(const wchar_t* substring) const
{
    int position = -1;

    if (m_data != 0 && substring != 0 && m_data[0] != 0 && substring[0] != 0)
    {
        String text(m_data);
        String pattern(substring);
        const wchar_t* found;

        text.ToUpper();
        pattern.ToUpper();

        found = std::wcsstr(text.m_data, pattern.m_data);
        if (found != 0)
            position = static_cast<int>(found - text.m_data);
    }

    return position;
}

unsigned long long String::ToUInt64() const
{
    char* copy;
    char* end;

    CreateCopy(&copy);
    end = 0;

    unsigned long long value = std::strtoull(copy, &end, 10);
    ReleaseCopy(copy);

    return value;
}

bool String::IsEqual(const wchar_t* left, const wchar_t* right)
{
    if (left == right)
        return true;

    if (left == 0)
        return IsEqual(L"", right);

    if (right == 0)
        return IsEqual(left, L"");

    return std::wcscmp(left, right) == 0;
}

String String::_Copy(const wchar_t* first, unsigned int firstLength, const wchar_t* second, unsigned int secondLength)
{
    String joined;

    joined.Reserve(static_cast<int>(firstLength + secondLength + 1));
    std::wcscpy(joined.m_data, first);
    std::wcscpy(joined.m_data + firstLength, second);

    return joined;
}

String operator+(const String& left, const String& right)
{
    unsigned int leftLength = left.GetLength();
    unsigned int rightLength;

    if (leftLength == 0)
        return right;

    rightLength = right.GetLength();
    if (rightLength == 0)
        return left;

    return String::_Copy(left.m_data, leftLength, right.m_data, rightLength);
}

String operator+(const String& left, const wchar_t* right)
{
    unsigned int leftLength = left.GetLength();

    if (leftLength == 0)
        return String(right);

    if (right != 0 && right[0] != 0)
        return String::_Copy(left.m_data, leftLength, right, static_cast<unsigned int>(std::wcslen(right)));

    return left;
}

String operator+(const wchar_t* left, const String& right)
{
    if (left != 0 && left[0] != 0)
    {
        unsigned int rightLength = right.GetLength();

        if (rightLength == 0)
            return String(left);

        return String::_Copy(left, static_cast<unsigned int>(std::wcslen(left)), right.m_data, rightLength);
    }

    return right;
}

}
}
