#include "platform/wiiu/C4JEventImpl.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"

C4JEventImpl::C4JEventImpl(C4JEvent::EMode mode) : m_mode(mode), m_waitCount(0)
{
    OSInitEvent(&m_event, 0, static_cast<OSEventMode>(mode != C4JEvent::Mode1));
}

C4JEventImpl::~C4JEventImpl()
{
    while (m_waitCount != 0)
    {
        Sleep(1);
    }
}

void C4JEventImpl::Set()
{
    OSSignalEvent(&m_event);
}

void C4JEventImpl::Clear()
{
    OSResetEvent(&m_event);
}

unsigned int C4JEventImpl::WaitForSignal(int timeout)
{
    ++m_waitCount;
    if (timeout == -1)
    {
        OSWaitEvent(&m_event);
        --m_waitCount;
        return WaitSignalled;
    }

    int result = OSWaitEventWithTimeout(&m_event, timeout * 1000000);
    --m_waitCount;
    if (result != 0)
    {
        return WaitSignalled;
    }
    return WaitTimeout;
}
