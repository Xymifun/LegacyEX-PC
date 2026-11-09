#include "nn/pia/session/OpenParticipationJob.hpp"

#include "nn/pia/Transport.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/Session.hpp"

namespace nn
{
namespace pia
{
namespace session
{

OpenParticipationJob::OpenParticipationJob()
    : common::StepSequenceJob(),
      m_callerContext(0),
      m_callContext(),
      m_sessionId(0),
      m_matchmakeSession(0)
{
}

OpenParticipationJob::~OpenParticipationJob()
{
}

Result OpenParticipationJob::Startup(common::CallContext* callerContext, unsigned int sessionId,
                                     IMatchmakeSession* matchmakeSession)
{
    if (!matchmakeSession->IsMatchmakeOwner())
        return static_cast<Result>(transport::cResultInvalidState);

    m_callerContext = callerContext;
    callerContext->InitiateCall();
    m_sessionId = sessionId;
    m_matchmakeSession = matchmakeSession;
    Reset(true);
    SetCurrentStep(
        Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&OpenParticipationJob::OpenParticipation),
             "OpenParticipationJob::OpenParticipation"));
    return 0;
}

OpenParticipationJob::ExecuteResult OpenParticipationJob::OpenParticipation()
{
    unsigned int status = g_piaSession->GetStatus();
    switch (status)
    {
    case Session::cStatusDisconnectedNetwork:
    {
        m_callerContext->SignalFailure(static_cast<Result>(Session::cResultStatusDisconnectedNetwork));
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case Session::cStatusUnknown6:
    {
        m_callerContext->SignalFailure(static_cast<Result>(Session::cResultStatusUnknown6));
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case Session::cStatusUnknown7:
    {
        m_callerContext->SignalFailure(static_cast<Result>(Session::cResultStatusUnknown7));
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case Session::cStatusUnknown8:
    {
        m_callerContext->SignalFailure(static_cast<Result>(Session::cResultStatusUnknown8));
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    default:
        break;
    }
    if (g_piaSession->GetStatus() == Session::cStatusDisconnectedSession)
    {
        m_callerContext->SignalFailure(static_cast<Result>(Session::cResultStatusDisconnectedSession));
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        m_callerContext->SignalCancel();
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }

    const Result result = m_matchmakeSession->OpenParticipate(&m_callContext, m_sessionId);
    if (result != 0)
    {
        if (result == static_cast<Result>(transport::cResultTransportUnavailable))
            return MakeExecuteResult(cExecuteKindNextStep);
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SetCurrentStep(
        Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&OpenParticipationJob::WaitOpenParticipation),
             "OpenParticipationJob::WaitOpenParticipation"));
    return MakeExecuteResult(cExecuteKindNextStep);
}

OpenParticipationJob::ExecuteResult OpenParticipationJob::WaitOpenParticipation()
{
    unsigned int status = g_piaSession->GetStatus();
    switch (status)
    {
    case Session::cStatusDisconnectedNetwork:
    {
        m_callerContext->SignalFailure(static_cast<Result>(Session::cResultStatusDisconnectedNetwork));
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case Session::cStatusUnknown6:
    {
        m_callerContext->SignalFailure(static_cast<Result>(Session::cResultStatusUnknown6));
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case Session::cStatusUnknown7:
    {
        m_callerContext->SignalFailure(static_cast<Result>(Session::cResultStatusUnknown7));
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case Session::cStatusUnknown8:
    {
        m_callerContext->SignalFailure(static_cast<Result>(Session::cResultStatusUnknown8));
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    default:
        break;
    }
    if (!m_matchmakeSession->WaitOpenParticipate())
        return MakeExecuteResult(cExecuteKindNextStep);

    const common::CallContext::State state = m_callContext.m_state;
    if (state == common::CallContext::cStateFailure)
    {
        if (m_callContext.m_result == static_cast<Result>(transport::cResultInvalidState))
        {
            g_piaSession->ChangeStatusToDisconnectedSession();
            m_callerContext->SignalFailure(static_cast<Result>(Session::cResultStatusDisconnectedSession));
        }
        else
        {
            m_callerContext->SignalFailure(m_callContext.m_result);
        }
    }
    if (state == common::CallContext::cStateSuccess)
    {
        Session* session = g_piaSession;
        if (session->m_sessionId[session->m_currentSessionIndex] == m_sessionId)
        {
            session->SetCurrentSessionOpenStatus(Session::cSessionOpenStatusOpen);
            m_callerContext->SignalSuccess(0);
        }
        else
        {
            if (session->m_sessionId[session->m_currentSessionIndex == 0 ? 1 : 0] == m_sessionId)
                session->SetBufferSessionOpenStatus(Session::cSessionOpenStatusOpen);
            m_callerContext->SignalSuccess(0);
        }
    }
    m_callerContext = 0;
    return MakeExecuteResult(cExecuteKindFinished);
}

void OpenParticipationJob::Cleanup()
{
    if (m_callerContext != 0)
    {
        if (m_callerContext->m_state == common::CallContext::cStateInProgress)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
    }
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        m_callContext.SignalCancel();
    m_callContext.Reset();
}

void OpenParticipationJob::Trace(unsigned long long) const
{
}

}
}
}
