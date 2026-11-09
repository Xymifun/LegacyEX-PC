#include "nn/nex/JobBackEndServicesTerminate.hpp"

#include "nn/nex/BackEndServices.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/Credentials.hpp"
#include "nn/nex/DebugString.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint64_t TRACE_BACK_END_SERVICES = 0x1000;
const int WAIT_CONTEXT_PENDING_JOB = 100;

enum TerminateResult
{
    TERMINATE_SUCCEEDED_RESULT = 0x10001
};

}

JobBackEndServicesTerminate::JobBackEndServicesTerminate(unsigned int uiCallId, BackEndServices* pBackEndServices)
    : StepSequenceJob(JOB_TYPE_2, DebugString(L"")),
      m_callContext()
{
    m_pBackEndServices = pBackEndServices;
    m_uiCallId = uiCallId;
    m_traceData = TRACE_BACK_END_SERVICES;
    SetStep(Step(static_cast<Step::Function>(&JobBackEndServicesTerminate::StepWaitOnPendingJobs), L""));
}

JobBackEndServicesTerminate::~JobBackEndServicesTerminate()
{
}

void JobBackEndServicesTerminate::StepWaitOnPendingJobs()
{
    BackEndServices* pBackEndServices = m_pBackEndServices;
    if (pBackEndServices->LoginJobIsInProgress())
    {
        pBackEndServices->CancelLoginJob();
        SetStep(Step(static_cast<Step::Function>(&JobBackEndServicesTerminate::StepWaitOnLoginCancel), L""));
    }
    else if (pBackEndServices->LogoutJobIsInProgress())
    {
        SetToWaiting(WAIT_CONTEXT_PENDING_JOB);
    }
    else
    {
        SetStep(Step(static_cast<Step::Function>(&JobBackEndServicesTerminate::StepInitLogoutIfRequired), L""));
    }
}

void JobBackEndServicesTerminate::StepWaitOnLoginCancel()
{
    if (m_pBackEndServices->LoginJobIsInProgress())
    {
        SetToWaiting(WAIT_CONTEXT_PENDING_JOB);
        return;
    }

    SetStep(Step(static_cast<Step::Function>(&JobBackEndServicesTerminate::StepInitLogoutIfRequired), L""));
}

void JobBackEndServicesTerminate::SetFinalState(CallContext::State state, const qResult& rResult)
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext != 0)
        pCallContext->SetStateImpl(state, rResult, true);

    SetToComplete();
}

void JobBackEndServicesTerminate::StepInitLogoutIfRequired()
{
    if (m_pBackEndServices->GetCredentials() != 0)
    {
        m_pBackEndServices->LogoutImpl(&m_callContext, m_pBackEndServices->GetCredentials());
        SetToSuspended();
        ResumeOnCallCompletion(
            &m_callContext,
            new Step(static_cast<Step::Function>(&JobBackEndServicesTerminate::StepLogoutComplete), L""));
        return;
    }

    SetFinalState(CallContext::CallSucceeded, qResult(TERMINATE_SUCCEEDED_RESULT));
}

void JobBackEndServicesTerminate::StepLogoutComplete()
{
    const bool bSucceeded = m_callContext.GetResult();
    SetFinalState(m_callContext.GetState(), m_callContext.GetResult());
}

}
}
