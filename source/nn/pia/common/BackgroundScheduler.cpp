#include "nn/pia/common/BackgroundScheduler.hpp"

#include "nn/pia/common/Job.hpp"

#include <stddef.h>

namespace nn
{
namespace pia
{
namespace common
{

namespace
{
const int kThreadStackSize = 0x5000;
}

BackgroundScheduler::BackgroundScheduler(int threadPriority, CriticalSection* criticalSection)
    : m_delegate(this, &BackgroundScheduler::Dispatch),
      m_thread("Tc@Pia BackgroundScheduler", &m_delegate, common::HeapManager::GetHeap(), threadPriority,
               pead::MessageQueue::cNoBlock, pead::Thread::cDefaultQuitMsg, kThreadStackSize,
               pead::Thread::cDefaultMessageQueueSize),
      m_criticalSection(criticalSection),
      m_event(false),
      m_finalize(false),
      m_jobList()
{
    m_thread.setAffinity(pead::CoreIdMask(1 << pead::CoreInfo::getCurrentCoreId().value()));
    m_thread.start();
    m_jobList.SetOffset(offsetof(Job, m_listNode));
}

BackgroundScheduler::~BackgroundScheduler()
{
    m_finalize = true;
    m_event.setSignal();
    m_thread.waitDone();
}

void BackgroundScheduler::EntryJob(Job* job)
{
    Time time = job->m_time;
    for (OffsetList::iterator it = m_jobList.begin(), end = m_jobList.end(); it != end; ++it)
    {
        if (static_cast<Job*>(*it)->m_time <= time)
        {
            m_jobList.InsertBeforeNode(m_jobList.ToListNode(*it), m_jobList.ToListNode(job));
            return;
        }
    }
    m_event.setSignal();
    m_jobList.InsertBeforeNode(m_jobList.Sentinel(), m_jobList.ToListNode(job));
}

void BackgroundScheduler::ResetJob(Job* job)
{
    bool isIncluded = m_jobList.IsIncludeNode(m_jobList.ToListNode(job));
    if (isIncluded)
        m_jobList.EraseNode(m_jobList.ToListNode(job));
}

void BackgroundScheduler::Dispatch(pead::Thread* thread, int)
{
    ScopedLock lock(*m_criticalSection);
    for (;;)
    {
        if (m_finalize)
        {
            thread->quit(false);
            return;
        }
        if (m_jobList.GetNodeNum() == 0)
        {
            m_criticalSection->Unlock();
            m_event.wait();
            m_criticalSection->Lock();
            continue;
        }

        Job* job = static_cast<Job*>(m_jobList.FromListNode(m_jobList.Sentinel()->prev));
        Time now;
        now.SetNow();
        TimeSpan span = job->m_time - now;
        if (TimeSpan::FromMSec(0) < span)
        {
            m_criticalSection->Unlock();
            m_event.wait(pead::TickSpan(span.m_tick));
            m_criticalSection->Lock();
        }
        else
        {
            m_jobList.PopBackNode();
            job->Execute(true);
            m_event.resetSignal();
        }
    }
}

}
}
}
