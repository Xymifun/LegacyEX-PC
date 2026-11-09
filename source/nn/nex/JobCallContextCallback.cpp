#include "nn/nex/JobCallContextCallback.hpp"

#include "nn/nex/CallContext.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

namespace
{

enum CallContextResult
{
    CALL_CONTEXT_RESULT_CONNECTION_LOST = static_cast<int>(0x80010004u)
};

}

JobCallContextCallback::JobCallContextCallback(JobType jobType, const wchar_t* pName, CallContext* pCallContext)
    : StepSequenceJob(jobType, DebugString(pName))
{
    m_pCallContext = pCallContext;
    m_uiCallId = 0;
    m_pJobCallContext = 0;
}

JobCallContextCallback::~JobCallContextCallback()
{
    if (m_pJobCallContext != 0)
        delete m_pJobCallContext;
}

bool JobCallContextCallback::StartJob()
{
    CriticalSection& csScheduler = Scheduler::GetInstance()->GetCriticalSection();
    csScheduler.Enter();
    if (!m_pCallContext->InitiateCall())
    {
        csScheduler.Leave();
        return false;
    }

    m_uiCallId = m_pCallContext->m_uiCallId;
    Scheduler::GetInstance()->Queue(this, false);
    Core::GetInstance()->GetCallContextRegister()->AssociateCallContext(m_uiCallId, m_pJobCallContext->m_uiCallId);
    SetStep(Step(static_cast<Step::Function>(&JobCallContextCallback::StepCallStart), L""));
    csScheduler.Leave();
    return true;
}

void JobCallContextCallback::CompleteJob(const qResult& rResult)
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext != 0)
    {
        if (rResult)
        {
            pCallContext->SignalSuccess(rResult);
            ProccessSuccess(pCallContext);
        }
        else
        {
            pCallContext->SignalFailure(rResult);
            ProccessFailure(pCallContext);
        }
    }
    else
    {
        m_pJobCallContext->Cancel(CallContext::CallCanceled);
        ProccessCancel();
    }

    SetToComplete();
}

void JobCallContextCallback::StepCallStart()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(CALL_CONTEXT_RESULT_CONNECTION_LOST));
        return;
    }

    if (m_pJobCallContext->GetState() != CallContext::CallInProgress)
    {
        SetStep(Step(static_cast<Step::Function>(&JobCallContextCallback::StepCallCompletion), L""));
        return;
    }

    SetToSuspended();
    ResumeOnCallCompletion(m_pJobCallContext,
                           new Step(static_cast<Step::Function>(&JobCallContextCallback::StepCallCompletion), L""));
}

void JobCallContextCallback::StepFinish()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(CALL_CONTEXT_RESULT_CONNECTION_LOST));
        return;
    }

    CompleteJob(m_pJobCallContext->GetResult());
}

CallContext* JobCallContextCallback::CreateCallContext()
{
    return new CallContext();
}

void JobCallContextCallback::StepCallCompletion()
{
    StepCallCompletionImp();
}

void JobCallContextCallback::StepCallCompletionImp()
{
    StepFinish();
}

void JobCallContextCallback::ProccessSuccess(CallContext*)
{
}

void JobCallContextCallback::ProccessFailure(CallContext*)
{
}

void JobCallContextCallback::ProccessCancel()
{
}

}
}
