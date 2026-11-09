#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/SessionStateMonitoringContent.hpp"

#include "nn/pia/common/BackgroundScheduler.hpp"
#include "nn/pia/common/Job.hpp"
#include "nn/pia/common/Memory.hpp"

#include <stddef.h>
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
    cResultInvalidArgument = 0x10c07,
    cResultInvalidState = 0x10c08,
    cResultNotInitialized = 0x10c0c,
    cResultAlreadyCreated = 0x10c0f
};

const int cThreadPriorityNum = 32;
}

Scheduler* g_piaJobScheduler = 0;
static unsigned char* s_schedulerStorage = 0;

Scheduler::Scheduler(int threadPriority)
    : m_jobList(),
      m_nextJobList(),
      m_dispatchTime(),
      m_dispatchCount(0),
      m_criticalSection(6)
{
    m_nextJobList.SetOffset(offsetof(Job, m_listNode));
    m_jobList.SetOffset(offsetof(Job, m_listNode));
    m_backgroundScheduler = NewObj<BackgroundScheduler>(threadPriority, &m_criticalSection);
}

Result Scheduler::CreateInstance(int threadPriority)
{
    if (IsInitialized() == 0)
        return ResultNotInitialized();
    if (IsInSetupMode() == 0)
        return ResultInvalidState();
    if (g_piaJobScheduler != 0)
        return cResultAlreadyCreated;
    if (static_cast<unsigned int>(threadPriority) >= cThreadPriorityNum)
        return ResultInvalidArgument();

    s_schedulerStorage = NewArray<unsigned char>(sizeof(Scheduler), 8);
    if (s_schedulerStorage == 0)
        return cResultAllocationFailed;

    g_piaJobScheduler = new (s_schedulerStorage) Scheduler(threadPriority);
    return 0;
}

Scheduler::~Scheduler()
{
    DeleteObj(m_backgroundScheduler);
}

void Scheduler::DestroyInstance()
{
    if (g_piaJobScheduler != 0)
    {
        g_piaJobScheduler->~Scheduler();
        g_piaJobScheduler = 0;
    }
    if (s_schedulerStorage != 0)
    {
        DeleteArray(s_schedulerStorage);
        s_schedulerStorage = 0;
    }
}

void Scheduler::Dispatch(unsigned int limitMsec)
{
    Time now;
    now.SetNow();
    m_dispatchTime = now;
    ++m_dispatchCount;

    Time limitTime;
    if (limitMsec == 0)
        limitTime = Time::c_Max;
    else
        limitTime = m_dispatchTime + TimeSpan(TimeSpan::GetTicksPerMSec().m_tick * limitMsec);

    Time currentTime;
    ScopedLock lock(m_criticalSection);
    for (;;)
    {
        if (m_jobList.GetNodeNum() == 0)
            break;
        currentTime.SetNow();
        if (!(currentTime <= limitTime))
        {
            Job* job = static_cast<Job*>(m_jobList.GetNodeNum() > 0 ? m_jobList.FromListNode(m_jobList.Sentinel()->next)
                                                                    : 0);
            ListNode* node = m_nextJobList.PopFrontNode();
            if (node == 0)
                return;
            Job* nextJob = static_cast<Job*>(m_nextJobList.ObjectOf(node));
            while (nextJob != 0)
            {
                for (;;)
                {
                    if (job == 0)
                    {
                        m_jobList.InsertBeforeNode(m_jobList.Sentinel(), m_jobList.ToListNode(nextJob));
                        break;
                    }
                    if (job->m_time <= nextJob->m_time)
                    {
                        m_jobList.InsertBeforeNode(m_jobList.ToListNode(job), m_jobList.ToListNode(nextJob));
                        break;
                    }
                    ListNode* next = m_jobList.ToListNode(job)->next;
                    job = next == m_jobList.Sentinel() ? 0 : static_cast<Job*>(m_jobList.ObjectOf(next));
                }
                nextJob = static_cast<Job*>(m_nextJobList.FromListNode(m_nextJobList.PopFrontNode()));
            }
            return;
        }

        Job* job = static_cast<Job*>(m_jobList.GetNodeNum() > 0 ? m_jobList.FromListNode(m_jobList.Sentinel()->prev)
                                                                : 0);
        if (!(job->m_time <= currentTime))
            break;
        m_jobList.PopBackNode();
        job->Execute(false);
    }

    while (m_nextJobList.GetNodeNum() != 0)
    {
        Job* job = static_cast<Job*>(m_nextJobList.FromListNode(m_nextJobList.PopFrontNode()));
        m_jobList.InsertBeforeNode(m_jobList.Sentinel(), m_jobList.ToListNode(job));
    }
}

void Scheduler::SetMonitoringData()
{
    g_sessionStateMonitoringContent.m_schedulerDispatchCount = m_dispatchCount;
}

void Scheduler::EntryJob(Job* job, bool isBackground)
{
    if (isBackground)
    {
        m_backgroundScheduler->EntryJob(job);
        return;
    }

    Time time = job->m_time;
    for (OffsetList::iterator it = m_jobList.begin(), end = m_jobList.end(); it != end; ++it)
    {
        if (static_cast<Job*>(*it)->m_time <= time)
        {
            m_jobList.InsertBeforeNode(m_jobList.ToListNode(*it), m_jobList.ToListNode(job));
            return;
        }
    }
    m_jobList.InsertBeforeNode(m_jobList.Sentinel(), m_jobList.ToListNode(job));
}

void Scheduler::EntryJobNext(Job* job)
{
    m_nextJobList.InsertAfterNode(m_nextJobList.Sentinel(), m_nextJobList.ToListNode(job));
}

void Scheduler::ResetJob(Job* job)
{
    if (job->IsForeground())
    {
        if (m_jobList.IsIncludeNode(m_jobList.ToListNode(job)))
        {
            m_jobList.EraseNode(m_jobList.ToListNode(job));
            return;
        }
        if (m_nextJobList.IsIncludeNode(m_nextJobList.ToListNode(job)))
            m_nextJobList.EraseNode(m_nextJobList.ToListNode(job));
    }
    else if (job->IsBackground())
    {
        m_backgroundScheduler->ResetJob(job);
    }
}

}
}
}
