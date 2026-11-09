#include "nn/pia/common/Job.hpp"

#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/TimeSpan.hpp"

namespace nn
{
namespace pia
{
namespace common
{

Job::Job() : m_innerState(cInnerStateNotEntried), m_executeCount(0), m_time()
{
    m_listNode.prev = 0;
    m_listNode.next = 0;
}

Job::~Job()
{
}

void Job::Reset(bool isBackground)
{
    CriticalSection& criticalSection = g_piaJobScheduler->m_criticalSection;
    criticalSection.Lock();
    switch (m_innerState)
    {
    case cInnerStateForegroundWaiting:
    case cInnerStateForegroundReady:
        g_piaJobScheduler->ResetJob(this);
        break;
    default:
        break;
    }
    m_executeCount = 0;
    m_time.m_tick = 0;
    m_innerState = cInnerStateNotEntried;
    criticalSection.Unlock();
}

void Job::Ready(bool isBackground)
{
    CriticalSection& criticalSection = g_piaJobScheduler->m_criticalSection;
    criticalSection.Lock();
    if (m_innerState == cInnerStateNotEntried)
    {
        m_innerState = isBackground ? cInnerStateBackgroundReady : cInnerStateForegroundReady;
        m_time.SetNow();
        g_piaJobScheduler->EntryJob(this, isBackground);
    }
    criticalSection.Unlock();
}

void Job::Resume(bool isBackground)
{
    CriticalSection& criticalSection = g_piaJobScheduler->m_criticalSection;
    criticalSection.Lock();
    if (m_innerState == cInnerStateSuspended)
    {
        m_innerState = isBackground ? cInnerStateBackgroundReady : cInnerStateForegroundReady;
        m_time.SetNow();
        g_piaJobScheduler->EntryJob(this, isBackground);
    }
    criticalSection.Unlock();
}

Job::State Job::GetState() const
{
    CriticalSection& criticalSection = g_piaJobScheduler->m_criticalSection;
    criticalSection.Lock();
    switch (m_innerState)
    {
    case cInnerStateForegroundWaiting:
        criticalSection.Unlock();
        return cStateWaiting;
    case cInnerStateForegroundReady:
        criticalSection.Unlock();
        return cStateReady;
    case cInnerStateForegroundRunning:
        criticalSection.Unlock();
        return cStateRunning;
    case cInnerStateBackgroundWaiting:
        criticalSection.Unlock();
        return cStateWaiting;
    case cInnerStateBackgroundReady:
        criticalSection.Unlock();
        return cStateReady;
    case cInnerStateBackgroundRunning:
        criticalSection.Unlock();
        return cStateRunning;
    case cInnerStateSuspended:
        criticalSection.Unlock();
        return cStateSuspended;
    case cInnerStateCompleted:
        criticalSection.Unlock();
        return cStateCompleted;
    default:
        break;
    }
    criticalSection.Unlock();
    return cStateNotEntried;
}

void Job::Execute(bool isBackground)
{
    m_innerState = isBackground ? cInnerStateBackgroundRunning : cInnerStateForegroundRunning;
    CriticalSection& criticalSection = g_piaJobScheduler->m_criticalSection;
    criticalSection.Unlock();
    const ExecuteResult result = ExecuteCore();
    criticalSection.Lock();
    m_executeCount = m_executeCount + 1;

    switch (result.m_kind)
    {
    case cExecuteKindContinue:
    case cExecuteKindEntryForeground:
    case cExecuteKindEntryBackground:
        m_time.SetNow();
        {
            bool background = isBackground;
            switch (result.m_kind)
            {
            case cExecuteKindEntryForeground:
                background = false;
                break;
            case cExecuteKindEntryBackground:
                background = true;
                break;
            }
            g_piaJobScheduler->EntryJob(this, background);
            m_innerState = background ? cInnerStateBackgroundReady : cInnerStateForegroundReady;
        }
        break;
    case cExecuteKindFinished:
    case cExecuteKindCancelled:
        m_innerState = cInnerStateCompleted;
        break;
    case cExecuteKindSuspend:
        m_innerState = cInnerStateSuspended;
        break;
    case cExecuteKindWait:
        m_time.SetNow();
        m_time = m_time + TimeSpan::FromMSec(result.m_value);
        g_piaJobScheduler->EntryJob(this, isBackground);
        m_innerState = isBackground ? cInnerStateBackgroundWaiting : cInnerStateForegroundWaiting;
        break;
    case cExecuteKindNextStep:
        m_time.SetNow();
        g_piaJobScheduler->EntryJobNext(this);
        m_innerState = isBackground ? cInnerStateBackgroundWaiting : cInnerStateForegroundWaiting;
        break;
    }
}

bool Job::IsRunning() const
{
    CriticalSection& criticalSection = g_piaJobScheduler->m_criticalSection;
    criticalSection.Lock();
    bool running;
    switch (m_innerState)
    {
    case cInnerStateForegroundWaiting:
    case cInnerStateForegroundReady:
    case cInnerStateForegroundRunning:
    case cInnerStateBackgroundWaiting:
    case cInnerStateBackgroundReady:
    case cInnerStateBackgroundRunning:
    case cInnerStateSuspended:
        running = true;
        break;
    default:
        running = false;
        break;
    }
    criticalSection.Unlock();
    return running;
}

bool Job::IsForeground() const
{
    CriticalSection& criticalSection = g_piaJobScheduler->m_criticalSection;
    criticalSection.Lock();
    bool foreground;
    switch (m_innerState)
    {
    case cInnerStateForegroundWaiting:
    case cInnerStateForegroundReady:
    case cInnerStateForegroundRunning:
        foreground = true;
        break;
    default:
        foreground = false;
        break;
    }
    criticalSection.Unlock();
    return foreground;
}

bool Job::IsBackground() const
{
    CriticalSection& criticalSection = g_piaJobScheduler->m_criticalSection;
    criticalSection.Lock();
    bool background;
    switch (m_innerState)
    {
    case cInnerStateBackgroundWaiting:
    case cInnerStateBackgroundReady:
    case cInnerStateBackgroundRunning:
        background = true;
        break;
    default:
        background = false;
        break;
    }
    criticalSection.Unlock();
    return background;
}

}
}
}
