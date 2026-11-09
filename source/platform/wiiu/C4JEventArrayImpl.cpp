#include "platform/wiiu/C4JEventArrayImpl.hpp"

#include <coreinit/time.h>

C4JEventArrayImpl::C4JEventArrayImpl(int count, C4JEventArray::EMode mode) : m_count(count), m_mode(mode)
{
    m_events = new OSEvent[static_cast<unsigned int>(count)];
    for (int index = 0; index < count; ++index)
    {
        OSInitEvent(&m_events[index], 0, static_cast<OSEventMode>(m_mode != C4JEventArray::Mode1));
    }
}

C4JEventArrayImpl::~C4JEventArrayImpl()
{
}

void C4JEventArrayImpl::Set(int index)
{
    OSSignalEvent(&m_events[index]);
}

void C4JEventArrayImpl::Clear(int index)
{
    OSResetEvent(&m_events[index]);
}

void C4JEventArrayImpl::SetAll()
{
    for (int index = 0; index < m_count; ++index)
    {
        Set(index);
    }
}

void C4JEventArrayImpl::ClearAll()
{
    for (int index = 0; index < m_count; ++index)
    {
        Clear(index);
    }
}

unsigned int C4JEventArrayImpl::WaitForAll(int timeout)
{
    if (timeout == -1)
    {
        for (int index = 0; index < m_count; ++index)
        {
            OSWaitEvent(&m_events[index]);
        }
        return 0;
    }

    for (int index = 0; index < m_count; ++index)
    {
        OSTick start = OSGetTick();
        BOOL result = timeout != 0 ? OSWaitEventWithTimeout(&m_events[index], timeout) : FALSE;
        bool signaled = result != FALSE;
        OSTick now = OSGetTick();
        if (signaled)
        {
            timeout -= OSTicksToMilliseconds(now - start);
            if (timeout < 0)
            {
                timeout = 0;
            }
        }
        else
        {
            return 0x102;
        }
    }
    return 0;
}

unsigned int C4JEventArrayImpl::WaitForAny(int timeout)
{
    if (timeout == -1)
    {
        for (;;)
        {
            for (int index = 0; index < m_count; ++index)
            {
                if (OSWaitEventWithTimeout(&m_events[index], 1000) != 0)
                {
                    return 0;
                }
            }
        }
    }

    OSTick start = OSGetTick();
    do
    {
        for (int index = 0; index < m_count; ++index)
        {
            if (OSWaitEventWithTimeout(&m_events[index], 1000) != 0)
            {
                return 0;
            }
        }
    } while (OSTicksToMilliseconds(OSGetTick() - start) < timeout);
    return 0x102;
}

unsigned int C4JEventArrayImpl::WaitForSingle(int index, int timeout)
{
    if (timeout == -1)
    {
        OSWaitEvent(&m_events[index]);
        return 0;
    }
    int timeoutNs = timeout * 1000000;
    OSWaitEventWithTimeout(&m_events[index], timeoutNs);
    return 0x102;
}

void C4JEventArrayImpl::Cancel()
{
    (void)m_count;
}
