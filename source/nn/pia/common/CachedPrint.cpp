#include "nn/pia/common/CachedPrint.hpp"

#include "nn/pia/common/Memory.hpp"

#include <cstdio>
#include <new>

namespace nn
{
namespace pia
{
namespace common
{

namespace
{
enum
{
    cResultAllocationFailed = 0x10c01,
    cResultAlreadyCreated = 0x10c0f
};

const unsigned int cMinimumCapacity = 0x200;
}

CachedPrint* CachedPrint::s_instance = 0;
unsigned char* CachedPrint::s_storage = 0;

CachedPrint::CachedPrint(unsigned int capacity)
    : m_buffer(0),
      m_capacity(capacity),
      m_start(0),
      m_position(0),
      m_end(0),
      m_criticalSection(7)
{
    m_buffer = NewArray<char>(capacity);
}

CachedPrint::~CachedPrint()
{
    DeleteArray(m_buffer);
}

Result CachedPrint::CreateInstance(unsigned int capacity)
{
    if (IsInitialized() == 0)
        return ResultNotInitialized();
    if (IsInSetupMode() == 0)
        return ResultInvalidState();
    if (s_instance != 0)
        return cResultAlreadyCreated;
    if (capacity < cMinimumCapacity)
        return ResultInvalidArgument();

    s_storage = NewArray<unsigned char>(sizeof(CachedPrint), 8);
    if (s_storage == 0)
        return cResultAllocationFailed;
    s_instance = new (s_storage) CachedPrint(capacity);
    return 0;
}

void CachedPrint::DestroyInstance()
{
    if (s_instance != 0)
    {
        s_instance->~CachedPrint();
        s_instance = 0;
    }
    if (s_storage != 0)
    {
        DeleteArray(s_storage);
        s_storage = 0;
    }
}

void CachedPrint::VPrintf(const char* format, va_list args)
{
    common::ScopedLock lock(m_criticalSection);
    unsigned int maxLength = m_capacity - m_position;
    if (maxLength < 0x100)
    {
        maxLength = m_capacity;
        if (m_end != m_position || m_start == 0)
            m_start = 1;
        m_position = 0;
    }
    int written = std::vsnprintf(m_buffer + m_position, maxLength, format, args);
    if (written < 0 || static_cast<unsigned int>(written) >= maxLength)
    {
        written = static_cast<int>(maxLength - 1);
        m_buffer[m_capacity - 1] = 0;
    }
    const unsigned int newPosition = m_position + static_cast<unsigned int>(written);
    if (m_end == m_position)
    {
        m_position = newPosition;
        m_end = newPosition;
    }
    else if (newPosition < m_end)
    {
        m_position = newPosition;
        if (m_start <= newPosition)
            m_start = newPosition + 1;
    }
    else
    {
        m_position = newPosition;
        m_end = newPosition;
        m_start = 0;
    }
}

void CachedPrint::Printf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    VPrintf(format, args);
    va_end(args);
}

void CachedPrint::Clear()
{
    common::ScopedLock lock(m_criticalSection);
    m_start = 0;
    m_position = 0;
    m_end = 0;
}

void CachedPrint::Flush()
{
    common::ScopedLock lock(m_criticalSection);
    const unsigned int start = m_start;
    unsigned int end = m_end;
    if (end != start)
    {
        pead::system::PrintString(m_buffer + start, static_cast<int>(end - start));
        end = m_end;
    }
    if (end != m_position && m_position != 0)
        pead::system::PrintString(m_buffer, static_cast<int>(m_position));
    Clear();
}

}
}
}
