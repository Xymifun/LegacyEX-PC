#include "nn/nex/JobNameResolve.hpp"

#include "nn/nex/CallContext.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/Network.hpp"
#include "nn/nex/PseudoGlobalVariable.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/TransportBuffer.hpp"

namespace nn
{
namespace nex
{

namespace
{

enum NameResolveResult
{
    NAME_RESOLVE_PENDING_RESULT = static_cast<int>(0x80010001u),
    NAME_RESOLVE_ABORTED_RESULT = static_cast<int>(0x80010004u),
    NAME_RESOLVE_FAILED_RESULT = static_cast<int>(0x80050014u),
    NAME_RESOLVE_SUCCEEDED_RESULT = 0x10001
};

enum
{
    NAME_RESOLVE_CRITICAL_SECTION_TRACE_MASK = 1,
    NAME_RESOLVE_WAIT_CONTEXT = 10,
    NAME_RESOLVE_THREAD_WAIT_CONTEXT = 50
};

const unsigned int NAME_RESOLVE_THREAD_WAIT_FOREVER = 0xFFFFFFFFu;

PseudoGlobalVariable<bool> g_bNameResolveThreadStarted;

}

unsigned int g_nameResolveThreadPriority = 10;
int g_nameResolveThreadAffinity = static_cast<int>(INHERIT_THREAD_AFFINITY);

JobNameResolve::JobNameResolve(unsigned int uiThreadPriority, int iThreadAffinity, ResolveMethod method)
    : StepSequenceJob(JOB_TYPE_0, DebugString(L"")),
      m_uiCallId(0),
      m_result(NAME_RESOLVE_PENDING_RESULT),
      m_pThread(0),
      m_criticalSection(NAME_RESOLVE_CRITICAL_SECTION_TRACE_MASK),
      m_uiThreadPriority(uiThreadPriority),
      m_iThreadAffinity(iThreadAffinity),
      m_method(method)
{
    Network::AcquireInstance();
}

JobNameResolve::~JobNameResolve()
{
    if (m_pThread != 0)
    {
        m_pThread->Wait(NAME_RESOLVE_THREAD_WAIT_FOREVER);
        delete m_pThread;
        g_bNameResolveThreadStarted.Get() = false;
    }

    Network::ReleaseInstance();
}

void JobNameResolve::Initialize(unsigned int uiCallId)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    m_uiCallId = uiCallId;
    SetStep(Step(static_cast<Step::Function>(&JobNameResolve::StepRunThread), L""));
}

void JobNameResolve::CompleteJob(const qResult& rResult)
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
    }

    SetToComplete();
}

void JobNameResolve::StepRunThread()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(NAME_RESOLVE_ABORTED_RESULT));
        return;
    }

    if (g_bNameResolveThreadStarted.Get())
    {
        SetToWaiting(NAME_RESOLVE_THREAD_WAIT_CONTEXT);
        return;
    }

    m_pThread = new Thread(String(L"DNS Check Thread"), m_uiThreadPriority);
    m_pThread->SetAffinity(m_iThreadAffinity);
    g_bNameResolveThreadStarted.Get() = true;
    m_pThread->SetObjectMethod(&JobNameResolve::CheckThread, this, 0);
    m_pThread->Launch(0);
    SetStep(Step(static_cast<Step::Function>(&JobNameResolve::StepCheck), L""));
}

void JobNameResolve::StepCheck()
{
    ScopedCS lock(&m_criticalSection);
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext != 0)
    {
        if (m_result == qResult(NAME_RESOLVE_PENDING_RESULT))
        {
            SetToWaiting(NAME_RESOLVE_WAIT_CONTEXT);
            return;
        }

        CompleteJob(m_result);
        return;
    }

    if (m_result == qResult(NAME_RESOLVE_PENDING_RESULT))
    {
        SetStep(Step(static_cast<Step::Function>(&JobNameResolve::StepCancelWait), L""));
        return;
    }

    CompleteJob(qResult(NAME_RESOLVE_ABORTED_RESULT));
}

void JobNameResolve::StepCancelWait()
{
    ScopedCS lock(&m_criticalSection);
    if (m_result == qResult(NAME_RESOLVE_PENDING_RESULT))
        SetToWaiting(NAME_RESOLVE_WAIT_CONTEXT);
    else
        CompleteJob(qResult(NAME_RESOLVE_ABORTED_RESULT));
}

void JobNameResolve::CheckThread(int)
{
    RootTransport* pTransport = RootTransport::GetInstance();
    bool bResolved = (pTransport->*m_method)();
    ScopedCS lock(&m_criticalSection);
    if (bResolved)
        m_result = qResult(NAME_RESOLVE_SUCCEEDED_RESULT);
    else
        m_result = qResult(NAME_RESOLVE_FAILED_RESULT);
}

}
}
