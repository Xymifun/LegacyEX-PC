#include "nn/pia/inet/NexMatchGenerateSystemPasswordJob.hpp"

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

inline void SelectStep(common::StepSequenceJob::Step& slot,
                       ExecuteResult (NexMatchGenerateSystemPasswordJob::*function)(), const char* name)
{
    const common::StepSequenceJob::Step step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(function), name);
    slot = step;
}
}

NexMatchGenerateSystemPasswordJob::NexMatchGenerateSystemPasswordJob()
    : session::GenerateMatchmakeSystemPasswordJob(),
      m_matchmakeSession(0)
{
}

NexMatchGenerateSystemPasswordJob::~NexMatchGenerateSystemPasswordJob()
{
}

Result NexMatchGenerateSystemPasswordJob::StartupImpl()
{
    m_matchmakeSession = static_cast<NexMatchmakeSession*>(
        session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex]);
    SelectStep(m_currentStep, &NexMatchGenerateSystemPasswordJob::GenerateMatchmakeSystemPassword,
               "NexMatchGenerateSystemPasswordJob::GenerateMatchmakeSystemPassword");
    return 0;
}

NexMatchGenerateSystemPasswordJob::ExecuteResult NexMatchGenerateSystemPasswordJob::GenerateMatchmakeSystemPassword()
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
    const Result result = m_matchmakeSession->GenerateSystemPassword(&m_callContext, m_sessionId);
    if (result != 0)
    {
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &NexMatchGenerateSystemPasswordJob::WaitGenerateMatchmakeSystemPassword,
               "NexMatchGenerateSystemPasswordJob::WaitGenerateMatchmakeSystemPassword");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchGenerateSystemPasswordJob::ExecuteResult NexMatchGenerateSystemPasswordJob::
    WaitGenerateMatchmakeSystemPassword()
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
    if (m_matchmakeSession->WaitGenerateSystemPassword())
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            const Result result = m_callContext.m_result;
            if (result == transport::cResultInvalidState)
                m_callerContext->SignalFailure(session::Session::cResultStatusDisconnectedSession);
            else
                m_callerContext->SignalFailure(result);
            SelectStep(m_currentStep, &NexMatchGenerateSystemPasswordJob::FailureProcess,
                       "GenerateMatchmakeSystemPasswordJob::FailureProcess");
            return MakeExecuteResult(cExecuteKindContinue);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            SelectStep(m_currentStep, &NexMatchGenerateSystemPasswordJob::CompleteProcess,
                       "GenerateMatchmakeSystemPasswordJob::CompleteProcess");
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

void NexMatchGenerateSystemPasswordJob::Cleanup()
{
    session::GenerateMatchmakeSystemPasswordJob::Cleanup();
    m_matchmakeSession = 0;
}

void NexMatchGenerateSystemPasswordJob::Trace(unsigned long long) const
{
}

}
}
}
