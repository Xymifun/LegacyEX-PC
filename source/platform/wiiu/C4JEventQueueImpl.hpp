#ifndef SOURCE_PLATFORM_WIIU_C4JEVENTQUEUEIMPL_HPP
#define SOURCE_PLATFORM_WIIU_C4JEVENTQUEUEIMPL_HPP

#include <queue>

#include "platform/wiiu/C4JEventQueue.hpp"
#include "platform/wiiu/C4JEventImpl.hpp"
#include "platform/wiiu/C4JEventArrayImpl.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "game/core/CriticalSection.hpp"

class C4JEventQueueImpl : public C4JEventQueue
{
public:
    C4JEventQueueImpl(void (*eventHandler)(void*), void (*startedHandler)(), const char* name);
    virtual void setProcessor(int processor);
    virtual void setPriority(int priority);
    virtual void sendEvent(void* event);
    virtual void waitForFinish();
    void threadPoll();
    static int threadFunc(void* argument);
    void init();

private:
    C4JThreadImpl* m_thread;
    std::queue<void*> m_events;
    C4JEventArrayImpl* m_eventArray;
    C4JEventImpl* m_finishEvent;
    OSFastMutex m_mutex;
    void (*m_eventHandler)(void*);
    void (*m_startedHandler)();
    char m_name[64];
    int m_processor;
    int m_priority;
};

#endif
