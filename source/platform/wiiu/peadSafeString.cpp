#include "platform/wiiu/pead.hpp"

namespace pead
{
template <> const char SafeStringBase<char>::cNullChar = '\0';
template <> const wchar_t SafeStringBase<wchar_t>::cNullChar = L'\0';

template <>
SafeStringBase<char>& SafeStringBase<char>::operator=(const SafeStringBase<char>& other)
{
    mStringTop = other.mStringTop;
    return *this;
}

template <>
SafeStringBase<wchar_t>& SafeStringBase<wchar_t>::operator=(const SafeStringBase<wchar_t>& other)
{
    mStringTop = other.mStringTop;
    return *this;
}

template <>
BufferedSafeStringBase<char>& BufferedSafeStringBase<char>::operator=(const SafeStringBase<char>& other)
{
    copy(other);
    return *this;
}

template <>
int BufferedSafeStringBase<char>::formatImpl_(char* buffer, int size, const char* format, va_list args)
{
    int result = StringUtil::vsnprintf(buffer, size, format, args);
    if (result < 0)
        result = size - 1;
    return result;
}

template <>
int BufferedSafeStringBase<char>::formatV(const char* format, va_list args)
{
    return formatImpl_(getMutableStringTop_(), mBufferSize, format, args);
}

template <>
int BufferedSafeStringBase<char>::format(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int result = formatV(format, args);
    va_end(args);
    return result;
}

template <>
int BufferedSafeStringBase<char>::appendWithFormatV(const char* format, va_list args)
{
    char* buffer = getMutableStringTop_();
    int length = calcLength();
    int result = formatImpl_(buffer + length, mBufferSize - length, format, args);
    return length + result;
}

template <>
int BufferedSafeStringBase<char>::appendWithFormat(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int result = appendWithFormatV(format, args);
    va_end(args);
    return result;
}

template <>
void BufferedSafeStringBase<char>::assureTerminationImpl_() const
{
    const_cast<char*>(mStringTop)[getBufferSize() - 1] = cNullChar;
}

template <>
BufferedSafeStringBase<wchar_t>& BufferedSafeStringBase<wchar_t>::operator=(const SafeStringBase<wchar_t>& other)
{
    copy(other);
    return *this;
}

template <>
int BufferedSafeStringBase<wchar_t>::formatImpl_(wchar_t* buffer, int size, const wchar_t* format, va_list args)
{
    int result = StringUtil::vsw16printf(buffer, size, format, args);
    if (result < 0 || result >= size)
    {
        buffer[size - 1] = cNullChar;
        return size - 1;
    }
    return result;
}

template <>
int BufferedSafeStringBase<wchar_t>::formatV(const wchar_t* format, va_list args)
{
    return formatImpl_(getMutableStringTop_(), mBufferSize, format, args);
}

template <>
int BufferedSafeStringBase<wchar_t>::format(const wchar_t* format, ...)
{
    va_list args;
    va_start(args, format);
    int result = formatV(format, args);
    va_end(args);
    return result;
}

template <>
int BufferedSafeStringBase<wchar_t>::appendWithFormatV(const wchar_t* format, va_list args)
{
    wchar_t* buffer = getMutableStringTop_();
    int length = calcLength();
    int result = formatImpl_(buffer + length, mBufferSize - length, format, args);
    return length + result;
}

template <>
int BufferedSafeStringBase<wchar_t>::appendWithFormat(const wchar_t* format, ...)
{
    va_list args;
    va_start(args, format);
    int result = appendWithFormatV(format, args);
    va_end(args);
    return result;
}

template <>
void BufferedSafeStringBase<wchar_t>::assureTerminationImpl_() const
{
    const_cast<wchar_t*>(mStringTop)[getBufferSize() - 1] = cNullChar;
}

template <typename T>
HeapSafeStringBase<T>::~HeapSafeStringBase()
{
    DeleteArray(this->getMutableStringTop_());
}

template <>
HeapSafeStringBase<char>& HeapSafeStringBase<char>::operator=(const SafeStringBase<char>& other)
{
    BufferedSafeStringBase<char>::operator=(other);
    return *this;
}

template <>
HeapSafeStringBase<wchar_t>& HeapSafeStringBase<wchar_t>::operator=(const SafeStringBase<wchar_t>& other)
{
    BufferedSafeStringBase<wchar_t>::operator=(other);
    return *this;
}

template class SafeStringBase<char>;
template class SafeStringBase<wchar_t>;
template class BufferedSafeStringBase<char>;
template class BufferedSafeStringBase<wchar_t>;
template class HeapSafeStringBase<char>;
template class HeapSafeStringBase<wchar_t>;
}
