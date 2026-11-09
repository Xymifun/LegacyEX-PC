#include "nn/nex/LogDeviceMemory.hpp"

#include <cstring>

#include "nn/nex/StringConversion.hpp"

namespace nn
{
namespace nex
{

LogDeviceMemory::~LogDeviceMemory()
{
}

void LogDeviceMemory::SetBuffer(char* pBuffer, int iSize, char** ppEndPosition)
{
    ScopedCS lock(&m_criticalSection);
    m_pBuffer = pBuffer;
    m_iSize = iSize;
    m_iStartPosition = 0;
    m_iWritePosition = 0;
    m_bWrapped = false;
    m_ppEndPosition = ppEndPosition;
    std::memset(pBuffer, 0, iSize);
    if (m_ppEndPosition != 0)
    {
        *m_ppEndPosition = m_pBuffer + m_iStartPosition;
    }
}

void LogDeviceMemory::Output(const wchar_t* pText)
{
    ScopedCS lock(&m_criticalSection);
    StringConversion::TToUtf8(pText, m_line, MAX_LINE_LENGTH + 1);
    int length = std::strlen(m_line);
    if (length > MAX_LINE_LENGTH)
    {
        length = MAX_LINE_LENGTH;
    }
    m_line[length++] = '\n';
    m_line[length] = '\0';
    std::strcat(m_line + length - 1, m_pEndMarker);
    int remaining = length + std::strlen(m_pEndMarker);
    const char* source = m_line;
    int limit = m_iSize - 1;
    while (remaining > 0)
    {
        int count = remaining;
        if (m_iWritePosition + count > limit)
        {
            count = limit - m_iWritePosition;
        }
        std::strncpy(m_pBuffer + m_iWritePosition, source, count);
        m_iWritePosition += count;
        if (m_iWritePosition < limit)
        {
            if (m_bWrapped)
            {
                m_iStartPosition = m_iWritePosition;
                if (m_ppEndPosition != 0)
                {
                    *m_ppEndPosition = m_pBuffer + m_iWritePosition;
                }
            }
        }
        else
        {
            m_iWritePosition = 0;
            m_iStartPosition = 0;
            m_bWrapped = true;
            if (m_ppEndPosition != 0)
            {
                *m_ppEndPosition = m_pBuffer + m_iWritePosition;
            }
        }
        remaining -= count;
        source += count;
    }
    m_iWritePosition -= std::strlen(m_pEndMarker);
    if (m_iWritePosition < 0)
    {
        m_iWritePosition += limit;
    }
}

}
}
