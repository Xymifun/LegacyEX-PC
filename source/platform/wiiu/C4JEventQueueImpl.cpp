#include "platform/wiiu/C4JEventQueueImpl.hpp"
#include "game/core/ShutdownManager.hpp"

#include <cstring>

C4JEventQueueImpl::C4JEventQueueImpl(void (*eventHandler)(void*), void (*startedHandler)(), const char* name)
    : m_thread(NULL),
      m_eventArray(NULL),
      m_finishEvent(NULL),
      m_eventHandler(eventHandler),
      m_startedHandler(startedHandler),
      m_processor(-1),
      m_priority(3)
{
    m_name[0] = '\0';
    if (name != NULL)
    {
        std::strncpy(m_name, name, sizeof(m_name) - 1);
        m_name[sizeof(m_name) - 1] = '\0';
    }
}

void C4JEventQueueImpl::setProcessor(int processor)
{
    m_processor = processor;
    if (m_thread != NULL)
    {
        m_thread->SetProcessor(processor);
    }
}

void C4JEventQueueImpl::setPriority(int priority)
{
    m_priority = priority;
    if (m_thread != NULL)
    {
        m_thread->SetPriority(priority);
    }
}

void C4JEventQueueImpl::threadPoll()
{
    ShutdownManager::HasStarted(ShutdownManager::eThreadId_EventQueue, m_eventArray);
    if (m_startedHandler != NULL)
    {
        m_startedHandler();
    }
    while (ShutdownManager::ShouldRun(ShutdownManager::eThreadId_EventQueue))
    {
        unsigned int result = m_eventArray->WaitForAny(-1);
        if (result == 0)
        {
            bool empty = true;
            do
            {
                EnterCriticalSection(&m_mutex);
                void* event = m_events.front();
                LeaveCriticalSection(&m_mutex);
                m_eventHandler(event);
                EnterCriticalSection(&m_mutex);
                m_events.pop();
                empty = m_events.empty();
                if (empty)
                {
                    m_finishEvent->Set();
                }
                LeaveCriticalSection(&m_mutex);
            } while (!empty);
        }
    }
    ShutdownManager::HasFinished(ShutdownManager::eThreadId_EventQueue);
}

int C4JEventQueueImpl::threadFunc(void* argument)
{
    static_cast<C4JEventQueueImpl*>(argument)->threadPoll();
    return 0;
}

void C4JEventQueueImpl::init()
{
    m_eventArray = new C4JEventArrayImpl(1, C4JEventArray::Mode0);
    m_finishEvent = new C4JEventImpl();
    InitializeCriticalSection(&m_mutex);
    m_thread = new C4JThreadImpl(threadFunc, this, m_name, 0);
    if (m_processor >= 0)
    {
        m_thread->SetProcessor(m_processor);
    }
    if (m_priority != 3)
    {
        m_thread->SetPriority(m_priority);
    }
    m_thread->Run();
}

void C4JEventQueueImpl::sendEvent(void* event)
{
    if (m_thread == NULL)
    {
        init();
    }
    EnterCriticalSection(&m_mutex);
    m_events.push(event);
    m_eventArray->Set(0);
    m_finishEvent->Clear();
    LeaveCriticalSection(&m_mutex);
}

void C4JEventQueueImpl::waitForFinish()
{
    if (m_thread == NULL)
    {
        init();
    }
    EnterCriticalSection(&m_mutex);
    if (m_events.empty())
    {
        LeaveCriticalSection(&m_mutex);
        return;
    }
    LeaveCriticalSection(&m_mutex);
    m_finishEvent->WaitForSignal(-1);
}
