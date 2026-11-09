#include "nn/pia/inet/NexMatchClearSystemPasswordJob.hpp"

#include "nn/pia/Transport.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/inet/NexMatchmakeSession.hpp"
#include "nn/pia/session/Session.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

namespace
{
typedef common::Job::ExecuteResult ExecuteResult;

enum
{
    cResultCancelled = 0x6c05
};

inline void SelectStep(common::StepSequenceJob::Step& slot, ExecuteResult (NexMatchClearSystemPasswordJob::*function)(),
                       const char* name)
{
    slot = common::StepSequenceJob::Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(function), name);
}
}

NexMatchClearSystemPasswordJob::NexMatchClearSystemPasswordJob()
    : session::ClearMatchmakeSystemPasswordJob(),
      m_matchmakeSession(0)
{
}

NexMatchClearSystemPasswordJob::~NexMatchClearSystemPasswordJob()
{
}

Result NexMatchClearSystemPasswordJob::StartupImpl()
{
    m_matchmakeSession = static_cast<NexMatchmakeSession*>(
        session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex]);
    SelectStep(m_currentStep, &NexMatchClearSystemPasswordJob::ClearMatchmakeSystemPassword,
               "NexMatchClearSystemPasswordJob::ClearMatchmakeSystemPassword");
    return 0;
}

NexMatchClearSystemPasswordJob::ExecuteResult NexMatchClearSystemPasswordJob::ClearMatchmakeSystemPassword()
{
    const unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
    {
        const Result result = session::Session::cResultStatusDisconnectedNetwork;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown6:
    {
        const Result result = session::Session::cResultStatusUnknown6;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown7:
    {
        const Result result = session::Session::cResultStatusUnknown7;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown8:
    {
        const Result result = session::Session::cResultStatusUnknown8;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    default:
        break;
    }
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        const Result result = cResultCancelled;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    const Result result = m_matchmakeSession->ClearSystemPassword(&m_callContext, m_sessionId);
    if (result != 0)
    {
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &NexMatchClearSystemPasswordJob::WaitClearMatchmakeSystemPassword,
               "NexMatchClearSystemPasswordJob::WaitClearMatchmakeSystemPassword");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchClearSystemPasswordJob::ExecuteResult NexMatchClearSystemPasswordJob::WaitClearMatchmakeSystemPassword()
{
    const unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
    {
        const Result result = session::Session::cResultStatusDisconnectedNetwork;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown6:
    {
        const Result result = session::Session::cResultStatusUnknown6;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown7:
    {
        const Result result = session::Session::cResultStatusUnknown7;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown8:
    {
        const Result result = session::Session::cResultStatusUnknown8;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    default:
        break;
    }
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        const Result result = cResultCancelled;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (m_matchmakeSession->WaitClearSystemPassword())
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            const Result result = m_callContext.m_result;
            if (result == transport::cResultInvalidState)
                m_callerContext->SignalFailure(session::Session::cResultStatusDisconnectedSession);
            else
                m_callerContext->SignalFailure(result);
            SelectStep(m_currentStep, &NexMatchClearSystemPasswordJob::FailureProcess,
                       "ClearMatchmakeSystemPasswordJob::FailureProcess");
            return MakeExecuteResult(cExecuteKindContinue);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            SelectStep(m_currentStep, &NexMatchClearSystemPasswordJob::CompleteProcess,
                       "ClearMatchmakeSystemPasswordJob::CompleteProcess");
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

void NexMatchClearSystemPasswordJob::Cleanup()
{
    session::ClearMatchmakeSystemPasswordJob::Cleanup();
    m_matchmakeSession = 0;
}

void NexMatchClearSystemPasswordJob::Trace(unsigned long long) const
{
}

}
}
}
