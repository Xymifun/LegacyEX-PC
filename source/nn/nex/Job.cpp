#include "nn/nex/Job.hpp"

#include "nn/nex/RootObject.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/SpinTest.hpp"

bool JobPreventBlockCallCallback(nn::nex::RootObject* pObject, unsigned int uiTimeout);

namespace nn
{
namespace nex
{

Job::Job(JobType jobType, const DebugString& rDebugString) : m_jobType(jobType)
{
    m_state = Initial;
    m_waitContext = 0;
    m_nbExecutions = 0;
    m_pCompletionCallback = 0;
    m_reserved1 = false;
    m_reserved2 = false;
    m_reserved0 = false;
}
Job::~Job()
{
}

void Job::DecoratedExecute()
{
    Execute();
}

void Job::TestSuspendedJobState()
{
}

void Job::AddActivity(const wchar_t* pActivity)
{
}

String Job::GetTraceInfo() const
{
    return String("");
}

void Job::SetDefaultPostExecutionState()
{
    SetState(Complete);
}

uint32_t Job::SkipWaitDelayAtTermination()
{
    return 0;
}

void Job::CancelJob()
{
}

void Job::ProcessInTermination()
{
}

void Job::ForceTerminate()
{
}

void Job::ValidateTransition(State currentState, State newState)
{
}

void Job::SetState(State state)
{
    ValidateTransition(m_state, state);
    m_state = state;

    if (state == Complete && m_pCompletionCallback != 0)
        m_pCompletionCallback(this, &m_userContext);
}

void Job::SetToInitial()
{
    SetState(Initial);
}

void Job::SetToWaiting(int waitContext)
{
    SetState(Waiting);
    m_waitContext = waitContext;
}

void Job::SetToSuspended()
{
    SetState(Suspended);
}

void Job::SetToReady()
{
    SetState(Ready);
}

void Job::SetToRunning()
{
    SetState(Running);
}

void Job::SetToComplete()
{
    SetState(Complete);
}

void Job::SetToCancel()
{
    SetState(Complete);
}

void Job::Resume()
{
    SetState(Ready);
}

void Job::PerformExecution(const Time& rTime)
{
    m_executionTime = static_cast<uint64_t>(rTime);
    DecoratedExecute();

    if (m_state == Running)
        SetDefaultPostExecutionState();

    ++m_nbExecutions;
}
bool Job::WaitForCompletion(uint32_t uiTimeout)
{
    bool bCompleted = false;

    if (Scheduler::PreventBlockCall(uiTimeout != 0, uiTimeout, false, &JobPreventBlockCallCallback, this, &bCompleted)
        != 0)
        return bCompleted;

    SpinTest spinTest(10, uiTimeout);

    spinTest.LeaveOnTimeout();

    while (m_state != Complete
           && spinTest.SpinOnce(L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Core/Job.cpp", 208,
                                L"GetState()!=Complete")
                  != 0)
    {
    }

    return m_state == Complete;
}

}
}

bool JobPreventBlockCallCallback(nn::nex::RootObject* pObject, unsigned int uiTimeout)
{
    return static_cast<nn::nex::Job*>(pObject)->WaitForCompletion(uiTimeout);
}
