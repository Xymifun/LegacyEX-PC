#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_SCHEDULER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_SCHEDULER_HPP

#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/EventHandler.hpp"
#include "nn/nex/Job.hpp"
#include "nn/nex/ProfilingUnit.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/SingleThreadCallPolicy.hpp"
#include "nn/nex/Time.hpp"
#include "nn/nex/WorkerThreads.hpp"
#include "nn/nex/qChain.hpp"
#include "nn/nex/qMultiMap.hpp"

namespace nn
{
namespace nex
{

class Event;
class qResult;

extern bool g_useGlobalSingleThreadDispatch;

class Scheduler : public RootObject
{
public:
    typedef qChain<Job*, DefaultChainPolicy<Job*> > JobChain;
    typedef qMultiMap<Time, Job*> JobMap;
    typedef bool (*PreventBlockCallback)(RootObject* pObject, unsigned int uiTimeout);

    class SchedulerWorkerThread : public WorkerThreads
    {
    public:
        SchedulerWorkerThread() : WorkerThreads(), m_pScheduler(0) {}

        virtual ~SchedulerWorkerThread();

    protected:
        virtual void Initialize();
        virtual void Work();

    private:
        friend class Scheduler;

        Scheduler* m_pScheduler;
    };

    Scheduler(unsigned int uiTraceMask = 8, SchedulerWorkerThread* pWorkerThreads = 0);
    virtual ~Scheduler();

    static Scheduler* GetInstance()
    {
        Core* pCore = Core::GetInstance();
        if (pCore == 0)
            return 0;

        return pCore->GetScheduler();
    }

    CriticalSection& GetCriticalSection() { return m_criticalSection; }

    void StartDispatcherThread();
    void StopDispatcherThread();
    bool HasDispatcherThread();
    void InitializeThread();

    void Queue(Job* pJob, bool bFront);
    bool Cancel(Job* pJob);
    bool IsQueued(const Job* pJob) const;

    void RegisterSpecialDispatchJob(Job* pJob, bool bFront);
    void UnregisterSpecialDispatchJob(Job* pJob);

    unsigned int Dispatch(unsigned int uiTimeout, unsigned int uiFlags);
    void SpecialDispatch(const Time& rDeadline);

    static void SingleThreadDispatch(unsigned int uiTimeout);
    static void GlobalSingleThreadDispatch(unsigned int uiTimeout);
    static void SingleThreadSpecialDispatch();
    static void GlobalSingleThreadSpecialDispatch();
    static void BypassGlobalSingleThreadDispatch();
    static void UseGlobalSingleThreadDispatch();

    static bool CurrentThreadCanWaitForJob();
    static bool PreventBlockCall(bool bAllowDispatch, unsigned int uiTimeout, bool bErrorValue,
                                 PreventBlockCallback cbCallback, RootObject* pObject, bool* pResult);
    static bool PreventRegularBlockCall(bool bAllowDispatch, unsigned int uiTimeout, PreventBlockCallback cbCallback,
                                        RootObject* pObject, qResult* pResult);

    void WaitForAllJobsToComplete(unsigned int uiTimeout, bool bRequeueDelayedJobs);
    void WaitForEmptyQueues() const;
    void ClearAllQueues();

    unsigned int GetReadyQueueSize() const;
    unsigned int GetTotalQueueSize() const;
    bool GetTotalNumberOfJobs() const;
    unsigned int GetNbSpecialJobs() const;
    unsigned int GetOneTypeJobsCount(Job::JobType jobType) const;

    void ProcessJobsInTermination(Job::JobType jobType);
    void TerminateDOCoreJobs();
    void ForceTerminateJobs();

    void RequeueWaitingJob(Job* pJob, Time delay);

    void EnableActivityTrace(bool bEnable);
    void SetName(const wchar_t* pName);
    static void SetJobStepExecutionBusyIntervalThreshold(long long llThreshold);

private:
    void QueueImpl(Job* pJob, bool bFront);
    void DeleteJob(Job* pJob);
    void ProcessJob(Job* pJob, const Time& rDeadline);
    void ProcessCurrentJob(const Time& rDeadline);
    bool QueueNewlyReadyJobs();
    void DispatchImpl(Time deadline);
    void WaitForJobToExecute(unsigned int uiTimeout);
    unsigned int DispatchInternal(unsigned int uiTimeout, unsigned int uiFlags);
    int TimeToDeadline(Time deadline) const;

    void DeleteDOCoreJobs(JobChain* pJobs);
    void DeleteDOCoreJobs(JobMap* pJobs);

    void DispatchBegins();
    void DispatchEnds();

    Job* m_pCurrentJob;
    bool m_bCurrentJobValid;
    SchedulerWorkerThread* m_pWorkerThreads;
    EventHandler m_eventHandler;
    Event* m_pJobQueuedEvent;
    mutable CriticalSection m_criticalSection;
    mutable CriticalSection m_queueCriticalSection;
    mutable CriticalSection m_specialCriticalSection;
    JobChain m_readyJobs;
    JobChain m_waitingJobs;
    JobMap m_delayedJobs;
    JobChain m_specialDispatchJobs;
    bool m_bTerminating;
    ProfilingUnit m_jobProcessingProfilingUnit;
    unsigned int m_reserved0;
    unsigned int m_reserved1;
    unsigned int m_reserved2;
    unsigned int m_reserved3;
    unsigned int m_uiContextId;
    SingleThreadCallPolicy m_callPolicy;
    Time m_dispatchTime;
    bool m_bActivityTraceEnabled;
};

}
}

#endif
