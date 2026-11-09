#include "nn/nex/Scheduler.hpp"

#include <algorithm>

#include "nn/nex/Core.hpp"
#include "nn/nex/Event.hpp"
#include "nn/nex/InstanceControl.hpp"
#include "nn/nex/Platform.hpp"
#include "nn/nex/PseudoSingleton.hpp"
#include "nn/nex/SpinTest.hpp"
#include "nn/nex/SystemError.hpp"
#include "nn/nex/qList.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

bool g_useGlobalSingleThreadDispatch = true;
unsigned int g_dispatcherThreadSleepInterval = 8;

Scheduler::SchedulerWorkerThread::~SchedulerWorkerThread()
{
}

void Scheduler::SchedulerWorkerThread::Initialize()
{
    m_pScheduler->InitializeThread();
}

void Scheduler::SchedulerWorkerThread::Work()
{
    m_pScheduler->DispatchInternal(100, 0);
    Platform::Sleep(g_dispatcherThreadSleepInterval);
}

Scheduler::Scheduler(unsigned int uiTraceMask, SchedulerWorkerThread* pWorkerThreads)
    : m_pCurrentJob(0),
      m_bCurrentJobValid(false),
      m_pWorkerThreads(pWorkerThreads),
      m_eventHandler(1),
      m_pJobQueuedEvent(0),
      m_criticalSection(uiTraceMask),
      m_queueCriticalSection(0x400),
      m_specialCriticalSection(0x400),
      m_readyJobs(),
      m_waitingJobs(),
      m_delayedJobs(),
      m_specialDispatchJobs(),
      m_bTerminating(false),
      m_jobProcessingProfilingUnit(L"JobProcessing", 0, L""),
      m_reserved0(0),
      m_reserved1(0),
      m_reserved2(0),
      m_reserved3(0),
      m_uiContextId(0),
      m_callPolicy(),
      m_dispatchTime(),
      m_bActivityTraceEnabled(false)
{
    m_pJobQueuedEvent = m_eventHandler.CreateEventObject(0, 0);

    if (m_pWorkerThreads == 0)
        m_pWorkerThreads = new SchedulerWorkerThread();

    m_pWorkerThreads->m_pScheduler = this;

    if (g_usingInstantiationContext)
        m_uiContextId = g_instantiationContextVariable->GetRef();
}

Scheduler::~Scheduler()
{
    m_pWorkerThreads->Stop();
    delete m_pWorkerThreads;

    while (m_specialDispatchJobs.size() != 0)
        UnregisterSpecialDispatchJob(m_specialDispatchJobs.front());

    m_eventHandler.DeleteEventObject(m_pJobQueuedEvent);
}

void Scheduler::StartDispatcherThread()
{
    m_pWorkerThreads->Start(1);
}

void Scheduler::InitializeThread()
{
    PseudoSingleton::SetContextIfRequired(m_uiContextId);
}

void Scheduler::QueueImpl(Job* pJob, bool bFront)
{
    switch (pJob->GetState())
    {
    case Job::Initial:
        pJob->SetToReady();

    case Job::Ready:
        if (bFront)
            m_readyJobs.push_front(pJob);
        else
            m_readyJobs.push_back(pJob);
        break;

    case Job::Waiting:
        m_delayedJobs.insert(JobMap::value_type(Time::GetTime() + pJob->m_waitContext, pJob));
        break;

    case Job::Suspended:
        if (bFront)
            m_waitingJobs.push_front(pJob);
        else
            m_waitingJobs.push_back(pJob);
        break;

    default:
        break;
    }

    m_pJobQueuedEvent->Set();
}

void Scheduler::Queue(Job* pJob, bool bFront)
{
    if (!g_threadSafetyEnabled)
    {
        QueueImpl(pJob, bFront);
        return;
    }

    ScopedCS lock(&m_queueCriticalSection);
    QueueImpl(pJob, bFront);
}

void Scheduler::DeleteJob(Job* pJob)
{
    pJob->ReleaseRef();
}

void Scheduler::ProcessJob(Job* pJob, const Time& rDeadline)
{
    if (pJob->GetState() == Job::Ready)
    {
        pJob->SetToRunning();

        if (m_bActivityTraceEnabled)
            pJob->AddActivity(0);

        pJob->PerformExecution(rDeadline);
    }

    if (pJob->GetState() == Job::Complete)
    {
        DeleteJob(pJob);
        return;
    }

    Queue(pJob, false);
}

void Scheduler::SpecialDispatch(const Time& rDeadline)
{
    ScopedCS lock(&m_criticalSection);
    ScopedCS innerLock(&m_specialCriticalSection);

    JobChain::iterator position = m_specialDispatchJobs.begin();

    while (position != m_specialDispatchJobs.end())
    {
        Job* pJob = *position;

        pJob->AddRef();
        pJob->SetToReady();
        ProcessJob(pJob, rDeadline);
        pJob->SetToInitial();

        ++position;
    }
}

bool Scheduler::QueueNewlyReadyJobs()
{
    bool bQueued = false;

    const Time now(Time::GetTime());

    ScopedCS lock(&m_criticalSection);
    ScopedCS innerLock(&m_queueCriticalSection);

    JobMap::iterator delayed = m_delayedJobs.begin();

    while (delayed != m_delayedJobs.end())
    {
        Job* pJob = delayed->second;

        if (!m_bTerminating || pJob->SkipWaitDelayAtTermination() == 0)
        {
            if (pJob->GetState() != Job::Complete && now < delayed->first)
                break;
        }

        JobMap::iterator next = delayed;
        ++next;

        m_delayedJobs.erase(delayed);
        delayed = next;

        if (pJob->GetState() == Job::Waiting)
        {
            pJob->SetToReady();
            Queue(pJob, false);
            bQueued = true;
        }
        else
        {
            DeleteJob(pJob);
        }
    }

    JobChain::iterator waiting = m_waitingJobs.begin();

    while (waiting != m_waitingJobs.end())
    {
        Job* pJob = *waiting;

        if (pJob->GetState() == Job::Suspended)
            pJob->TestSuspendedJobState();

        if (pJob->GetState() == Job::Ready)
        {
            waiting = m_waitingJobs.erase(waiting);
            Queue(pJob, false);
            bQueued = true;
        }
        else if (pJob->GetState() == Job::Complete)
        {
            waiting = m_waitingJobs.erase(waiting);
            DeleteJob(pJob);
        }
        else
        {
            ++waiting;
        }
    }


    return bQueued;
}

void Scheduler::ProcessCurrentJob(const Time& rDeadline)
{
    Job* pJob = m_pCurrentJob;

    if (m_bCurrentJobValid)
        ProcessJob(pJob, rDeadline);

    pJob->ReleaseRef();

    m_bCurrentJobValid = false;
    m_pCurrentJob = 0;
}

void Scheduler::DispatchImpl(Time deadline)
{
    bool bMoreJobs = true;

    QueueNewlyReadyJobs();

    for (;;)
    {
        {
            ScopedCS lock(&m_queueCriticalSection);

            if (m_readyJobs.size() != 0)
            {
                m_bCurrentJobValid = true;
                m_pCurrentJob = m_readyJobs.front();
                m_pCurrentJob->AddRef();
                m_readyJobs.erase(m_readyJobs.begin());
            }
            else
            {
                m_pCurrentJob = 0;
                m_bCurrentJobValid = false;
            }
        }

        if (m_pCurrentJob != 0)
        {
            if (m_pCurrentJob->m_reserved1)
            {
                ScopedCS instanceLock(&g_instanceTableCriticalSection);
                ScopedCS lock(&m_criticalSection);
                ProcessCurrentJob(deadline);
            }
            else
            {
                ScopedCS lock(&m_criticalSection);
                ProcessCurrentJob(deadline);
            }
        }
        else if (!QueueNewlyReadyJobs())
        {
            bMoreJobs = false;
        }

        const bool bDeadlineReached = TimeToDeadline(deadline) == 0;
        if (bDeadlineReached || !bMoreJobs)
            return;
    }
}

unsigned int Scheduler::GetReadyQueueSize() const
{
    ScopedCS lock(&m_queueCriticalSection);
    const unsigned int uiSize = m_readyJobs.size();
    return uiSize;
}

unsigned int Scheduler::GetTotalQueueSize() const
{
    ScopedCS lock(&m_queueCriticalSection);
    const unsigned int uiReadySize = m_readyJobs.size();
    const unsigned int uiDelayedSize = m_delayedJobs.size();
    const unsigned int uiWaitingSize = m_waitingJobs.size();
    return uiReadySize + uiDelayedSize + uiWaitingSize;
}

bool Scheduler::GetTotalNumberOfJobs() const
{
    ScopedCS lock(&m_queueCriticalSection);
    const unsigned int uiQueueSize = GetTotalQueueSize();
    const bool bCurrentJobValid = m_bCurrentJobValid;
    return uiQueueSize + static_cast<unsigned int>(bCurrentJobValid) != 0;
}

unsigned int Scheduler::GetNbSpecialJobs() const
{
    unsigned int uiCount = 0;

    ScopedCS lock(&m_queueCriticalSection);

    for (JobMap::const_iterator delayed = m_delayedJobs.begin(); delayed != m_delayedJobs.end(); ++delayed)
    {
        if (delayed->second->m_reserved0)
            ++uiCount;
    }

    for (JobChain::iterator waiting = m_waitingJobs.begin(); waiting != m_waitingJobs.end(); ++waiting)
    {
        if ((*waiting)->m_reserved0)
            ++uiCount;
    }

    for (JobChain::iterator ready = m_readyJobs.begin(); ready != m_readyJobs.end(); ++ready)
    {
        if ((*ready)->m_reserved0)
            ++uiCount;
    }


    return uiCount;
}

unsigned int Scheduler::GetOneTypeJobsCount(Job::JobType jobType) const
{
    unsigned int uiCount = 0;

    ScopedCS lock(&m_queueCriticalSection);

    for (JobChain::iterator ready = m_readyJobs.begin(); ready != m_readyJobs.end(); ++ready)
    {
        if ((*ready)->m_jobType == jobType)
            ++uiCount;
    }

    for (JobChain::iterator waiting = m_waitingJobs.begin(); waiting != m_waitingJobs.end(); ++waiting)
    {
        if ((*waiting)->m_jobType == jobType)
            ++uiCount;
    }

    for (JobMap::const_iterator delayed = m_delayedJobs.begin(); delayed != m_delayedJobs.end(); ++delayed)
    {
        if (delayed->second->m_jobType == jobType)
            ++uiCount;
    }


    return uiCount;
}

int Scheduler::TimeToDeadline(Time deadline) const
{
    if (deadline == 0)
        return -1;

    const Time now(Time::GetTime());

    if (now < deadline)
        return static_cast<int>(deadline - now);

    return 0;
}

void Scheduler::WaitForJobToExecute(unsigned int uiTimeout)
{
    {
        ScopedCS lock(&m_queueCriticalSection);

        if (m_delayedJobs.size() != 0)
        {
            const unsigned int uiDelay = static_cast<unsigned int>(TimeToDeadline(m_delayedJobs.begin()->first));
            if (uiDelay < uiTimeout)
                uiTimeout = uiDelay;
        }
    }

    Event* pSignaledEvent;
    m_eventHandler.WaitForEventOrTimeOut(uiTimeout, &pSignaledEvent);
}

unsigned int Scheduler::DispatchInternal(unsigned int uiTimeout, unsigned int uiFlags)
{
    if (uiTimeout == 0)
        uiTimeout = 0xFFFFFFFF;

    m_callPolicy.BeginCallImpl();

    const unsigned long long now = Time::GetTime();
    m_dispatchTime = Time(now);

    Time deadline;
    if (uiTimeout != 0xFFFFFFFF)
        deadline = Time(now + static_cast<unsigned long long>(static_cast<int>(uiTimeout)));

    SpecialDispatch(deadline);

    if ((uiFlags & 2) == 0)
    {
        for (;;)
        {
            DispatchImpl(deadline);

            if ((uiFlags & 1) == 0)
                break;

            const int iRemaining = TimeToDeadline(deadline);
            if (iRemaining == 0)
                break;

            WaitForJobToExecute(static_cast<unsigned int>(iRemaining));

            if (TimeToDeadline(deadline) == 0)
                break;
        }
    }

    const unsigned int uiReadyQueueSize = GetReadyQueueSize();

    if (TimeToDeadline(deadline) != 0)
        SpecialDispatch(deadline);

    m_callPolicy.EndCallImpl();

    return uiReadyQueueSize;
}

unsigned int Scheduler::Dispatch(unsigned int uiTimeout, unsigned int uiFlags)
{
    if (m_pWorkerThreads->GetNbWorkers() != 0)
        return 0;

    return DispatchInternal(uiTimeout, uiFlags);
}

bool Scheduler::CurrentThreadCanWaitForJob()
{
    if (GetInstance()->m_pWorkerThreads->GetNbWorkers() == 0)
        return false;

    if (GetInstance()->m_callPolicy.CurrentThreadIsDispatchingJobs())
        return false;

    return true;
}

void Scheduler::SingleThreadDispatch(unsigned int uiTimeout)
{
    Scheduler* pScheduler;

    {
        ScopedCS lock(&g_instanceTableCriticalSection);

        unsigned int uiContextId = 0;
        if (g_usingInstantiationContext)
            uiContextId = g_instantiationContextVariable->GetRef();

        if (!InstanceControl::ContextIsValid(uiContextId))
            return;

        pScheduler = GetInstance();
        if (pScheduler == 0 || pScheduler->m_pWorkerThreads->GetNbWorkers() != 0)
            return;

        Core::AcquireInstance();
    }

    pScheduler->DispatchInternal(uiTimeout, 0);

    Core::ReleaseInstance();
}

void Scheduler::GlobalSingleThreadDispatch(unsigned int uiTimeout)
{
    if (!g_useGlobalSingleThreadDispatch)
    {
        SingleThreadDispatch(uiTimeout);
        return;
    }

    unsigned int uiCurrentContextId = 0;
    if (g_usingInstantiationContext)
        uiCurrentContextId = g_instantiationContextVariable->GetRef();

    const unsigned int uiHighestId = static_cast<unsigned int>(InstanceControl::GetHighestID());

    for (unsigned int uiContextId = 0; uiContextId <= uiHighestId; ++uiContextId)
    {
        if (InstanceControl::ContextIsValid(uiContextId))
        {
            PseudoSingleton::SetContext(uiContextId);
            SingleThreadDispatch(uiTimeout);
        }
    }

    PseudoSingleton::SetContext(uiCurrentContextId);
}

void Scheduler::SingleThreadSpecialDispatch()
{
    Scheduler* pScheduler = GetInstance();
    if (pScheduler == 0)
        return;

    if (pScheduler->m_pWorkerThreads->GetNbWorkers() != 0)
        return;

    pScheduler->SpecialDispatch(Time());
}

void Scheduler::GlobalSingleThreadSpecialDispatch()
{
    if (!g_useGlobalSingleThreadDispatch)
    {
        SingleThreadSpecialDispatch();
        return;
    }

    unsigned int uiCurrentContextId = 0;
    if (g_usingInstantiationContext)
        uiCurrentContextId = g_instantiationContextVariable->GetRef();

    const unsigned int uiHighestId = static_cast<unsigned int>(InstanceControl::GetHighestID());

    for (unsigned int uiContextId = 0; uiContextId <= uiHighestId; ++uiContextId)
    {
        if (InstanceControl::ContextIsValid(uiContextId))
        {
            PseudoSingleton::SetContext(uiContextId);
            SingleThreadSpecialDispatch();
        }
    }

    PseudoSingleton::SetContext(uiCurrentContextId);
}

bool Scheduler::PreventBlockCall(bool bAllowDispatch, unsigned int uiTimeout, bool bErrorValue,
                                 PreventBlockCallback cbCallback, RootObject* pObject, bool* pResult)
{
    if (!bAllowDispatch || CurrentThreadCanWaitForJob())
        return false;

    if (GetInstance()->m_callPolicy.CurrentThreadIsDispatchingJobs())
    {
        SystemError::SignalError(L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Core/Scheduler.cpp",
                                 0x69, 0xE000001A, 0);
        *pResult = bErrorValue;
        return true;
    }

    SpinTest spinTest(10, uiTimeout);
    spinTest.LeaveOnTimeout();

    bool vResult = cbCallback(pObject, 0);

    while (vResult == bErrorValue)
    {
        GlobalSingleThreadDispatch(static_cast<unsigned int>(spinTest.GetRemainingTime()));

        if (spinTest.SpinOnce(L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Core/Scheduler.cpp", 0x72,
                              L"(vResult = cbCallback(object, 0)) == bErrorValue")
            == 0)
            break;

        vResult = cbCallback(pObject, 0);
    }

    if (vResult == bErrorValue)
        SystemError::SignalError(L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Core/Scheduler.cpp",
                                 0x79, 0xE000000C, 0);

    *pResult = vResult;
    return true;
}

bool Scheduler::PreventRegularBlockCall(bool bAllowDispatch, unsigned int uiTimeout, PreventBlockCallback cbCallback,
                                        RootObject* pObject, qResult* pResult)
{
    if (!bAllowDispatch || CurrentThreadCanWaitForJob())
        return false;

    if (GetInstance()->m_callPolicy.CurrentThreadIsDispatchingJobs())
    {
        SystemError::SignalError(L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Core/Scheduler.cpp",
                                 0x93, 0xE000001A, 0);
        *pResult = qResult(static_cast<int>(0x8001000B));
        return true;
    }

    qResult vResult(static_cast<int>(0x8001000B));

    SpinTest spinTest(10, uiTimeout);
    spinTest.LeaveOnTimeout();

    while (!vResult)
    {
        vResult = cbCallback(pObject, 0);

        if (spinTest.SpinOnce(L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Core/Scheduler.cpp", 0xA0,
                              L"PreventRegularBlockCall")
            == 0)
            break;

        GlobalSingleThreadDispatch(static_cast<unsigned int>(spinTest.GetRemainingTime()));
    }

    if (!vResult)
    {
        SystemError::SignalError(L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Core/Scheduler.cpp",
                                 0xA9, 0xE000000C, 0);
        *pResult = qResult(static_cast<int>(0x8001000B));
        return true;
    }

    *pResult = vResult;
    return true;
}

bool Scheduler::Cancel(Job* pJob)
{
    bool bCancelled = false;

    ScopedCS lock(&m_criticalSection);
    ScopedCS innerLock(&m_queueCriticalSection);

    JobMap::iterator delayed = m_delayedJobs.begin();
    while (delayed != m_delayedJobs.end() && delayed->second != pJob)
        ++delayed;

    if (delayed != m_delayedJobs.end())
    {
        m_delayedJobs.erase(delayed);
        m_pJobQueuedEvent->Set();
        bCancelled = true;
        DeleteJob(pJob);
    }
    else
    {
        JobChain::iterator ready = std::find(m_readyJobs.begin(), m_readyJobs.end(), pJob);

        if (ready != m_readyJobs.end())
        {
            m_readyJobs.erase(ready);
            bCancelled = true;
            DeleteJob(pJob);
        }
        else
        {
            JobChain::iterator waiting = std::find(m_waitingJobs.begin(), m_waitingJobs.end(), pJob);

            if (waiting != m_waitingJobs.end())
            {
                m_waitingJobs.erase(waiting);
                bCancelled = true;
                DeleteJob(pJob);
            }
            else if (m_pCurrentJob == pJob)
            {
                bCancelled = true;
                m_bCurrentJobValid = false;
                DeleteJob(pJob);
            }
        }
    }


    return bCancelled;
}

bool Scheduler::IsQueued(const Job* pJob) const
{
    ScopedCS lock(&m_queueCriticalSection);

    JobMap::const_iterator delayed = m_delayedJobs.begin();
    while (delayed != m_delayedJobs.end() && delayed->second != pJob)
        ++delayed;

    if (delayed != m_delayedJobs.end())
    {
        return true;
    }

    if (std::find(m_readyJobs.begin(), m_readyJobs.end(), pJob) != m_readyJobs.end())
    {
        return true;
    }

    if (std::find(m_waitingJobs.begin(), m_waitingJobs.end(), pJob) != m_waitingJobs.end())
    {
        return true;
    }

    return false;
}

void Scheduler::ClearAllQueues()
{
    ScopedCS lock(&m_queueCriticalSection);

    JobChain::iterator ready = m_readyJobs.begin();
    while (ready != m_readyJobs.end())
        ready = m_readyJobs.erase(ready);

    JobChain::iterator waiting = m_waitingJobs.begin();
    while (waiting != m_waitingJobs.end())
        waiting = m_waitingJobs.erase(waiting);

    m_delayedJobs.clear();
}

void Scheduler::RegisterSpecialDispatchJob(Job* pJob, bool bFront)
{
    ScopedCS lock(&m_specialCriticalSection);

    if (bFront)
        m_specialDispatchJobs.push_front(pJob);
    else
        m_specialDispatchJobs.push_back(pJob);
}

void Scheduler::UnregisterSpecialDispatchJob(Job* pJob)
{
    ScopedCS lock(&m_criticalSection);
    ScopedCS innerLock(&m_specialCriticalSection);

    JobChain::iterator position = std::find(m_specialDispatchJobs.begin(), m_specialDispatchJobs.end(), pJob);

    m_specialDispatchJobs.erase(position);
    DeleteJob(pJob);
}

void Scheduler::WaitForAllJobsToComplete(unsigned int uiTimeout, bool bRequeueDelayedJobs)
{
    if (bRequeueDelayedJobs)
    {
        ScopedCS lock(&m_queueCriticalSection);

        for (JobMap::iterator delayed = m_delayedJobs.begin(); delayed != m_delayedJobs.end(); ++delayed)
        {
            delayed->second->SetToReady();
            Queue(delayed->second, false);
        }

        m_delayedJobs.clear();
    }

    SpinTest spinTest(10, uiTimeout);
    spinTest.LeaveOnTimeout();

    while (GetTotalNumberOfJobs())
    {
        GlobalSingleThreadDispatch(static_cast<unsigned int>(spinTest.GetRemainingTime()));

        if (spinTest.SpinOnce(L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Core/Scheduler.cpp",
                              0x278, L"GetTotalNumberOfJobs() != 0")
            == 0)
            break;
    }

    if (GetTotalNumberOfJobs())
        GetTotalNumberOfJobs();
}

void Scheduler::ProcessJobsInTermination(Job::JobType jobType)
{
    for (JobChain::iterator ready = m_readyJobs.begin(); ready != m_readyJobs.end(); ++ready)
    {
        if ((*ready)->m_jobType == jobType)
            (*ready)->ProcessInTermination();
    }

    for (JobChain::iterator waiting = m_waitingJobs.begin(); waiting != m_waitingJobs.end(); ++waiting)
    {
        if ((*waiting)->m_jobType == jobType)
            (*waiting)->ProcessInTermination();
    }

    for (JobMap::iterator delayed = m_delayedJobs.begin(); delayed != m_delayedJobs.end(); ++delayed)
    {
        if (delayed->second->m_jobType == jobType)
            delayed->second->ProcessInTermination();
    }
}

void Scheduler::DeleteDOCoreJobs(JobChain* pJobs)
{
    CriticalSection* pLock = &m_queueCriticalSection;

    pLock->Enter();
    JobChain::iterator position = pJobs->begin();
    bool bDone = (position == pJobs->end());
    pLock->Leave();

    while (!bDone)
    {
        Job* pJob = *position;

        if (!pJob->m_reserved0)
        {
            pLock->Enter();
            position = pJobs->erase(position);
            pLock->Leave();

            DeleteJob(pJob);
        }

        pLock->Enter();
        if (position == pJobs->end())
        {
            pLock->Leave();
            return;
        }
        pLock->Leave();
    }
}

void Scheduler::DeleteDOCoreJobs(JobMap* pJobs)
{
    CriticalSection* pLock = &m_queueCriticalSection;

    JobMap::iterator position;

    pLock->Enter();
    position = pJobs->begin();
    bool bDone = (position == pJobs->end());
    pLock->Leave();

    while (!bDone)
    {
        Job* pJob = position->second;

        if (!pJob->m_reserved0)
        {
            pLock->Enter();
            pJobs->erase(position++);
            pLock->Leave();

            DeleteJob(pJob);
        }

        pLock->Enter();
        if (position == pJobs->end())
        {
            pLock->Leave();
            return;
        }
        pLock->Leave();
    }
}

void Scheduler::TerminateDOCoreJobs()
{
    DeleteDOCoreJobs(&m_readyJobs);
    DeleteDOCoreJobs(&m_waitingJobs);
    DeleteDOCoreJobs(&m_delayedJobs);
}

void Scheduler::RequeueWaitingJob(Job* pJob, Time delay)
{
    ScopedCS lock(&m_queueCriticalSection);

    JobMap::iterator position = m_delayedJobs.begin();
    while (position != m_delayedJobs.end() && position->second != pJob)
        ++position;

    if (position != m_delayedJobs.end())
    {
        m_delayedJobs.erase(position);

        pJob->m_waitContext = static_cast<int>(static_cast<unsigned long long>(delay));

        m_delayedJobs.insert(JobMap::value_type(Time::GetTime() + pJob->m_waitContext, pJob));
    }
}

void Scheduler::ForceTerminateJobs()
{
    for (JobChain::iterator ready = m_readyJobs.begin(); ready != m_readyJobs.end(); ++ready)
        (*ready)->ForceTerminate();

    for (JobChain::iterator waiting = m_waitingJobs.begin(); waiting != m_waitingJobs.end(); ++waiting)
        (*waiting)->ForceTerminate();

    qList<Job*> terminatedJobs;

    for (JobMap::iterator delayed = m_delayedJobs.begin(); delayed != m_delayedJobs.end(); ++delayed)
    {
        Job* pJob = delayed->second;
        const Job::State previousState = pJob->GetState();

        pJob->ForceTerminate();

        if (previousState != pJob->GetState() && pJob->GetState() == Job::Complete)
            terminatedJobs.push_back(pJob);
    }

    for (qList<Job*>::iterator terminated = terminatedJobs.begin(); terminated != terminatedJobs.end(); ++terminated)
    {
        RequeueWaitingJob(*terminated, Time());
    }
}

}
}
