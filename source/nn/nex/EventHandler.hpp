#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_EVENTHANDLER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_EVENTHANDLER_HPP

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Event;

class EventHandler : public RootObject
{
public:
    explicit EventHandler(unsigned short maxEvents);
    virtual ~EventHandler();

    Event* CreateEventObject(unsigned int type, unsigned int context);
    void DeleteEventObject(Event* pEvent);

    void SetEvent(Event* pEvent);
    void ResetEvent(Event* pEvent);
    bool EventSet(Event* pEvent) const;

    unsigned int GetEventIndex(Event* pEvent) const;
    unsigned short GetSignaledEvent() const;

    bool WaitForEvent(unsigned int timeout, Event** ppEvent) const;
    bool WaitForEventOrTimeOut(unsigned int timeout, Event** ppEvent) const;
    bool WaitForBoolEvent(unsigned int timeout, Event** ppEvent, bool bTimeOutAllowed) const;

    void* GetPlatformSpecificEvent(Event* pEvent);

private:
    mutable CriticalSection m_criticalSection;
    volatile bool** m_ppSignaled;
    Event** m_pEvents;
    unsigned int m_nbEvents;
    unsigned short m_maxEvents;
};

}
}

#endif
