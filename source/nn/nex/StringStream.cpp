#include "nn/nex/StringStream.hpp"

#include <cstring>
#include <cwchar>

#include "nn/nex/SpecialArray.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int NUMBER_FIELD_LENGTH = 0x20;

}

StringStream::StringStream()
{
    m_pBuffer = m_aInlineBuffer;
    m_uiCapacity = INLINE_BUFFER_LENGTH;
    m_pCursor = m_aInlineBuffer;
    m_aInlineBuffer[0] = 0;
    m_bHex = false;
    m_bAddBase = true;
    m_bTextualBool = true;
}

StringStream::~StringStream()
{
    FreeBuffer();
}

int StringStream::GetLength() const
{
    return static_cast<int>(m_pCursor - m_pBuffer);
}

void StringStream::FreeBuffer(wchar_t* pBuffer)
{
    if (pBuffer == m_aInlineBuffer)
    {
        return;
    }

    SpecialDeleteArray(pBuffer);
}

void StringStream::FreeBuffer()
{
    FreeBuffer(m_pBuffer);
}

void StringStream::ResizeBuffer(unsigned int uiCapacity)
{
    const int iLength = GetLength();
    wchar_t* pOldBuffer = m_pBuffer;

    m_pBuffer = SpecialNewArray<wchar_t>(uiCapacity, L"", 0);
    m_uiCapacity = uiCapacity;

    std::memcpy(m_pBuffer, pOldBuffer, iLength * sizeof(wchar_t) + sizeof(wchar_t));

    m_pCursor = m_pBuffer + iLength;

    FreeBuffer(pOldBuffer);
}

void StringStream::TestFreeRoom(unsigned int uiCount)
{
    const unsigned int uiRequired = GetLength() + uiCount;

    if (uiRequired > m_uiCapacity)
    {
        unsigned int uiCapacity = m_uiCapacity;

        while (uiCapacity < uiRequired)
        {
            uiCapacity += uiCapacity / 2;
        }

        ResizeBuffer(uiCapacity);
    }
}

void StringStream::AddBaseIfRequired()
{
    if (m_bAddBase)
    {
        std::wcscat(m_pCursor, L"0x");
        m_pCursor += 2;
    }
}

void StringStream::Clear()
{
    FreeBuffer();

    m_pBuffer = m_aInlineBuffer;
    m_uiCapacity = INLINE_BUFFER_LENGTH;
    m_pCursor = m_aInlineBuffer;
    m_aInlineBuffer[0] = 0;
}

StringStream& StringStream::operator<<(const wchar_t* pString)
{
    if (pString == 0)
    {
        return *this << L"(null)";
    }

    const unsigned int uiLength = static_cast<unsigned int>(std::wcslen(pString));

    TestFreeRoom(uiLength + 1);
    std::wcscpy(m_pCursor, pString);
    m_pCursor += uiLength;
    return *this;
}

StringStream& StringStream::operator<<(const void* pPointer)
{
    TestFreeRoom(NUMBER_FIELD_LENGTH);
    AddBaseIfRequired();

    wchar_t* pCursor = m_pCursor;

    m_pCursor = pCursor
                + std::swprintf(pCursor, NUMBER_FIELD_LENGTH, L"%llx",
                                static_cast<unsigned long long>(reinterpret_cast<unsigned int>(pPointer)));
    return *this;
}

StringStream& StringStream::operator<<(bool bValue)
{
    if (m_bTextualBool)
    {
        if (bValue)
        {
            return *this << L"true";
        }

        return *this << L"false";
    }

    return StreamNumber(static_cast<int>(bValue));
}

StringStream& StringStream::operator<<(unsigned long long uiValue)
{
    TestFreeRoom(NUMBER_FIELD_LENGTH);

    const wchar_t* pFormat;

    if (m_bHex)
    {
        AddBaseIfRequired();
        pFormat = m_bZeroPadded ? L"%016llx" : L"%llx";
    }
    else
    {
        pFormat = L"%llu";
    }

    m_pCursor += std::swprintf(m_pCursor, NUMBER_FIELD_LENGTH, pFormat, uiValue);
    return *this;
}

StringStream& StringStream::StreamNumber(int iValue)
{
    TestFreeRoom(NUMBER_FIELD_LENGTH);

    const wchar_t* pFormat;

    if (m_bHex)
    {
        AddBaseIfRequired();
        pFormat = m_bZeroPadded ? L"%08x" : L"%x";
    }
    else
    {
        pFormat = L"%d";
    }

    m_pCursor += std::swprintf(m_pCursor, NUMBER_FIELD_LENGTH, pFormat, iValue);
    return *this;
}

StringStream& StringStream::StreamNumber(unsigned int uiValue)
{
    TestFreeRoom(NUMBER_FIELD_LENGTH);

    const wchar_t* pFormat;

    if (m_bHex)
    {
        AddBaseIfRequired();
        pFormat = m_bZeroPadded ? L"%08x" : L"%x";
    }
    else
    {
        pFormat = L"%u";
    }

    m_pCursor += std::swprintf(m_pCursor, NUMBER_FIELD_LENGTH, pFormat, uiValue);
    return *this;
}

StringStream& StringStream::StreamNumber(unsigned char uiValue)
{
    TestFreeRoom(NUMBER_FIELD_LENGTH);

    const wchar_t* pFormat;

    if (m_bHex)
    {
        AddBaseIfRequired();
        pFormat = m_bZeroPadded ? L"%02x" : L"%x";
    }
    else
    {
        pFormat = L"%u";
    }

    m_pCursor += std::swprintf(m_pCursor, NUMBER_FIELD_LENGTH, pFormat, uiValue);
    return *this;
}

void zerohex(StringStream& rStream)
{
    rStream.m_bHex = true;
    rStream.m_bZeroPadded = true;
}

StringStream& hex(StringStream& rStream)
{
    rStream.m_bHex = true;
    rStream.m_bZeroPadded = false;
    return rStream;
}

StringStream& noshowbase(StringStream& rStream)
{
    rStream.m_bAddBase = false;
    return rStream;
}

StringStream& operator<<(StringStream& rStream, const String& rString)
{
    return rStream << static_cast<const wchar_t*>(rString);
}

}
}
