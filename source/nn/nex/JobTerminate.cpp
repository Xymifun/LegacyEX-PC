#include "nn/nex/JobTerminate.hpp"

#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/NgsFacade.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint64_t TRACE_TERMINATE = 0x1000;
const int WAIT_CONTEXT_PENDING_JOB = 100;

enum TerminateResult
{
    TERMINATE_SUCCEEDED_RESULT = 0x10001
};

}

JobTerminate::JobTerminate(unsigned int uiCallId, NgsFacade* pNgsFacade)
    : StepSequenceJob(JOB_TYPE_2, DebugString(L"")),
      m_callContext()
{
    m_pNgsFacade = pNgsFacade;
    m_uiCallId = uiCallId;
    m_traceData = TRACE_TERMINATE;
    SetStep(Step(static_cast<Step::Function>(&JobTerminate::StepWaitOnPendingJobs), L""));
}

JobTerminate::~JobTerminate()
{
}

void JobTerminate::StepWaitOnPendingJobs()
{
    if (m_pNgsFacade->LoginJobIsInProgress())
    {
        SetToWaiting(WAIT_CONTEXT_PENDING_JOB);
        return;
    }

    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext != 0)
        pCallContext->SignalSuccess(qResult(TERMINATE_SUCCEEDED_RESULT));

    SetToComplete();
}

}
}
