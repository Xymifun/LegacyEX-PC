#include "nn/nex/EventHandler.hpp"

#include "nn/nex/Event.hpp"
#include "nn/nex/Platform.hpp"
#include "nn/nex/SpecialArray.hpp"
#include "nn/nex/SystemError.hpp"
#include "nn/nex/Time.hpp"

#include <cstring>

namespace nn
{
namespace nex
{
namespace
{

const unsigned short NO_SIGNALED_EVENT = 0xffff;

unsigned int s_nextEvent = 0;

}

EventHandler::EventHandler(unsigned short maxEvents)
{
    m_ppSignaled = new (RootObject::operator new(sizeof(volatile bool*))) volatile bool*[1]();
    m_maxEvents = maxEvents;
    *m_ppSignaled = SpecialNewArray<bool>(maxEvents, L"", 0);

    for (int index = 0; index < m_maxEvents; ++index)
        (*m_ppSignaled)[index] = false;

    m_pEvents = SpecialNewArray<Event*>(m_maxEvents, L"", 0);

    memset(m_pEvents, 0, m_maxEvents * sizeof(Event*));

    m_nbEvents = 0;
}

EventHandler::~EventHandler()
{
    SpecialDeleteArray(m_pEvents);
    SpecialDeleteArray(*m_ppSignaled);
    RootObject::operator delete(m_ppSignaled);
}

unsigned int EventHandler::GetEventIndex(Event* pEvent) const
{
    ScopedCS lock(&m_criticalSection);

    for (unsigned short index = 0; index < m_maxEvents; ++index)
    {
        if (m_pEvents[index] == pEvent)
        {
            return index;
        }
    }

    return 0;
}

void EventHandler::SetEvent(Event* pEvent)
{
    (*m_ppSignaled)[GetEventIndex(pEvent)] = true;
}

void EventHandler::ResetEvent(Event* pEvent)
{
    (*m_ppSignaled)[GetEventIndex(pEvent)] = false;
}

bool EventHandler::EventSet(Event* pEvent) const
{
    return (*m_ppSignaled)[GetEventIndex(pEvent)];
}

Event* EventHandler::CreateEventObject(unsigned int type, unsigned int context)
{
    int slot;

    ScopedCS lock(&m_criticalSection);

    for (slot = 0; slot < m_maxEvents; ++slot)
    {
        if (m_pEvents[slot] == 0)
            break;
    }

    m_pEvents[slot] = new Event(this, type, context);
    ++m_nbEvents;

    ResetEvent(m_pEvents[slot]);

    Event* pEvent = m_pEvents[slot];

    return pEvent;
}

void EventHandler::DeleteEventObject(Event* pEvent)
{
    ScopedCS lock(&m_criticalSection);

    ResetEvent(pEvent);
    m_pEvents[GetEventIndex(pEvent)] = 0;

    delete pEvent;
}

unsigned short EventHandler::GetSignaledEvent() const
{
    ScopedCS lock(&m_criticalSection);

    for (unsigned int index = 0; index < m_nbEvents; ++index)
    {
        if (EventSet(m_pEvents[(index + s_nextEvent) % m_nbEvents]))
        {
            unsigned short signaled = static_cast<unsigned short>((index + s_nextEvent) % m_nbEvents);

            return signaled;
        }
    }

    ++s_nextEvent;

    return NO_SIGNALED_EVENT;
}

bool EventHandler::WaitForBoolEvent(unsigned int timeout, Event** ppEvent, bool bTimeOutAllowed) const
{
    const Time started = Time::GetTime();
    unsigned short signaled;

    for (;;)
    {
        signaled = GetSignaledEvent();

        if (signaled != NO_SIGNALED_EVENT)
            break;

        if (timeout != 0xffffffffu)
        {
            if (static_cast<int64_t>(Time::GetTime() - started) >= timeout)
            {
                if (!bTimeOutAllowed)
                {
                    SystemError::SignalError(
                        L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Platform/Core/EventHandler.cpp",
                        311, 0xe000000cu, 0);
                }

                return false;
            }
        }

        Platform::Sleep(1);
    }

    m_pEvents[signaled]->Reset();
    *ppEvent = m_pEvents[signaled];
    return true;
}

bool EventHandler::WaitForEventOrTimeOut(unsigned int timeout, Event** ppEvent) const
{
    return WaitForBoolEvent(timeout, ppEvent, true);
}

}
}
