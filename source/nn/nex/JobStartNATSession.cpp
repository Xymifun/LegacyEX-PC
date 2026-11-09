#include "nn/nex/JobStartNATSession.hpp"

#include "nn/nex/CallContext.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/JobNameResolve.hpp"
#include "nn/nex/JobResolveRelayInfo.hpp"
#include "nn/nex/Network.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

namespace
{

enum StartNATSessionResult
{
    START_NAT_SESSION_ABORTED_RESULT = static_cast<int>(0x80010004u),
    START_NAT_SESSION_NAME_RESOLVE_FAILED_RESULT = static_cast<int>(0x80050014u)
};

enum
{
    START_NAT_SESSION_WAIT_CONTEXT = 10
};

}

JobStartNATSession::~JobStartNATSession()
{
    if (m_pRelayInfoJob != 0)
        m_pRelayInfoJob->ReleaseRef();
}

bool JobStartNATSession::StartJob()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!m_pCallContext->InitiateCall())
        return false;

    m_uiCallId = m_pCallContext->GetCallID();
    Scheduler::GetInstance()->Queue(this, false);
    if (m_sessionContext.GetState() != CallContext::CallSucceeded)
    {
        Core::GetInstance()->GetCallContextRegister()->AssociateCallContext(m_uiCallId, m_sessionContext.GetCallID());
    }

    m_relayInfoContext.Reset();
    m_pRelayInfoJob = JobResolveRelayInfo::CreateAndStartJob(&m_relayInfoContext);
    if (m_pRelayInfoJob == 0)
    {
        SetStep(Step(static_cast<Step::Function>(&JobStartNATSession::StepReady), L""));
        return true;
    }

    Core::GetInstance()->GetCallContextRegister()->AssociateCallContext(m_uiCallId, m_relayInfoContext.GetCallID());
    SetStep(Step(static_cast<Step::Function>(&JobStartNATSession::StepReady), L""));
    return true;
}

void JobStartNATSession::CompleteJob(const qResult& rResult)
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext != 0)
    {
        if (rResult)
        {
            pCallContext->SignalSuccess(rResult);
            SetToComplete();
            return;
        }

        pCallContext->SignalFailure(rResult);
        SetToComplete();
        return;
    }

    m_sessionContext.Cancel(CallContext::CallCanceled);
    SetToComplete();
}

void JobStartNATSession::StepReady()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        SetStep(Step(static_cast<Step::Function>(&JobStartNATSession::StepCallCompletion), L""));
        return;
    }

    if (m_sessionContext.GetState() != CallContext::CallInProgress)
    {
        SetStep(Step(static_cast<Step::Function>(&JobStartNATSession::StepCallCompletion), L""));
        return;
    }

    SetToSuspended();
    ResumeOnCallCompletion(&m_sessionContext,
                           new Step(static_cast<Step::Function>(&JobStartNATSession::StepCallCompletion), L""));
}

void JobStartNATSession::StepCallCompletion()
{
    if (Network::GetInstance() == 0 || RootTransport::GetInstance() == 0)
    {
        CompleteJob(qResult(START_NAT_SESSION_ABORTED_RESULT));
        return;
    }

    if (m_relayInfoContext.GetState() == CallContext::CallInProgress)
    {
        SetToSuspended();
        ResumeOnCallCompletion(&m_relayInfoContext,
                               new Step(static_cast<Step::Function>(&JobStartNATSession::StepCallCompletion), L""));
        return;
    }

    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(START_NAT_SESSION_ABORTED_RESULT));
        return;
    }

    if (m_pRelayInfoJob == 0
        || m_pRelayInfoJob->m_key.m_relayAddressType != JobResolveRelayInfo::RELAY_ADDRESS_HOST_NAME
        || RootTransport::GetInstance()->m_bRelayAddressSet)
    {
        SetStep(Step(static_cast<Step::Function>(&JobStartNATSession::StepFinish), L""));
        return;
    }

    RootTransport::GetInstance()->SetRelayNameAddress(m_pRelayInfoJob->m_key.m_relayAddress);
    m_nameResolveContext.Reset();
    if (!m_nameResolveContext.InitiateCall())
    {
        CompleteJob(qResult(START_NAT_SESSION_ABORTED_RESULT));
        return;
    }

    JobNameResolve* pNameResolve = new JobNameResolve(g_nameResolveThreadPriority, g_nameResolveThreadAffinity,
                                                      &RootTransport::SetUpRelayServer);
    pNameResolve->Initialize(m_nameResolveContext.GetCallID());
    Scheduler::GetInstance()->Queue(pNameResolve, true);
    Core::GetInstance()->GetCallContextRegister()->AssociateCallContext(m_uiCallId, m_nameResolveContext.GetCallID());
    SetToSuspended();
    ResumeOnCallCompletion(&m_nameResolveContext,
                           new Step(static_cast<Step::Function>(&JobStartNATSession::StepDNSCallCompletion), L""));
}

void JobStartNATSession::StepDNSCallCompletion()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        SetStep(Step(static_cast<Step::Function>(&JobCallContextCallback::StepFinish), L""));
        return;
    }

    switch (m_nameResolveContext.GetState())
    {
    default:
        SetStep(Step(static_cast<Step::Function>(&JobStartNATSession::StepFinish), L""));
        return;
    case CallContext::CallInProgress:
        SetToWaiting(START_NAT_SESSION_WAIT_CONTEXT);
        return;
    case CallContext::CallFailed:
        CompleteJob(qResult(START_NAT_SESSION_NAME_RESOLVE_FAILED_RESULT));
        return;
    case CallContext::CallSucceeded:
        SetStep(Step(static_cast<Step::Function>(&JobStartNATSession::StepFinish), L""));
        return;
    case CallContext::Ready:
        SetStep(Step(static_cast<Step::Function>(&JobStartNATSession::StepFinish), L""));
        return;
    }
}

void JobStartNATSession::StepFinish()
{
    if (m_pRelayInfoJob != 0 && m_relayInfoContext.GetResult())
        m_pRelayInfoJob->ResolvedRelayInfo();

    CompleteJob(m_sessionContext.GetResult());
}

}
}
