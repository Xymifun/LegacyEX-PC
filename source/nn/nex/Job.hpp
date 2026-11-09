#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOB_HPP

#include <stdint.h>

#include "nn/nex/RefCountedObject.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/Time.hpp"
#include "nn/nex/UserContext.hpp"
#include "nn/nex/qChain.hpp"

namespace nn
{
namespace nex
{

class CallbackRoot;
class DebugString;

class Job : public RefCountedObject
{
public:
    enum JobType
    {
        JOB_TYPE_0 = 0,
        JOB_TYPE_2 = 2
    };

    enum State
    {
        Initial = 0,
        Waiting = 1,
        Suspended = 2,
        Ready = 3,
        Running = 4,
        Complete = 5
    };

    typedef void (*CompletionCallback)(Job* pJob, const UserContext* pUserContext);

    Job(JobType jobType, const DebugString& rDebugString);
    virtual ~Job();

    virtual void DecoratedExecute();
    virtual void Execute() = 0;
    virtual void TestSuspendedJobState();
    virtual void AddActivity(const wchar_t* pActivity);
    virtual String GetTraceInfo() const;
    virtual void SetDefaultPostExecutionState();
    virtual uint32_t SkipWaitDelayAtTermination();
    virtual void CancelJob();
    virtual void ProcessInTermination();
    virtual void ForceTerminate();

    void RegisterCompletionCallback(CompletionCallback pCallback, const UserContext& rUserContext);
    void RegisterCompletionCallback(CallbackRoot* pCallback);

    State GetState() const { return m_state; }

    String GetStateString(State state) const;

    void SetToInitial();
    void SetToWaiting(int waitContext);
    void SetToSuspended();
    void SetToReady();
    void SetToRunning();
    void SetToComplete();
    void SetToCancel();
    void Resume();

    void PerformExecution(const Time& rTime);
    bool WaitForCompletion(uint32_t uiTimeout);

protected:
    void ValidateTransition(State currentState, State newState);
    void SetState(State state);

private:
    friend class DefaultChainPolicy<Job*>;
    friend class PeriodicJob;
    friend class JobDeriveKey;
    friend class Scheduler;

    qChainLink<Job*> m_link;
    bool m_reserved0;
    CompletionCallback m_pCompletionCallback;
    UserContext m_userContext;
    JobType m_jobType;
    uint32_t m_unknown28;
    State m_state;
    int m_waitContext;
    uint32_t m_nbExecutions;
    Time m_executionTime;
    bool m_reserved1;
    bool m_reserved2;
};

}
}

#endif
