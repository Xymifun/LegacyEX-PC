#ifndef SOURCE_PLATFORM_WIIU_C4JEVENTIMPL_HPP
#define SOURCE_PLATFORM_WIIU_C4JEVENTIMPL_HPP

#include <coreinit/event.h>
#include "platform/wiiu/C4JEvent.hpp"

class C4JEventImpl : public C4JEvent
{
public:
    C4JEventImpl(C4JEvent::EMode mode = C4JEvent::Mode0);
    virtual ~C4JEventImpl();
    virtual void Set();
    virtual void Clear();
    virtual unsigned int WaitForSignal(int timeout);

private:
    C4JEvent::EMode m_mode;
    int m_waitCount;
    OSEvent m_event;
};

#endif
