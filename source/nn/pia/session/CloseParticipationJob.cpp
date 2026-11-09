#include "nn/pia/session/CloseParticipationJob.hpp"

#include "nn/pia/Transport.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/MeshLayerController.hpp"
#include "nn/pia/session/Session.hpp"

namespace nn
{
namespace pia
{
namespace session
{

namespace
{
enum
{
    cResultWaitP2pStableTimeout = 0x2c18
};

const int cWaitP2pStableMinMsec = 1000;
const int cWaitP2pStableTimeoutMsec = 15000;

inline void SelectStep(common::StepSequenceJob::Step& slot,
                       common::Job::ExecuteResult (CloseParticipationJob::*function)(), const char* name)
{
    const common::StepSequenceJob::Step step(
        static_cast<common::Job::ExecuteResult (common::StepSequenceJob::*)()>(function), name);
    slot = step;
}

inline int GetElapsedMsec(const common::Time& startTime)
{
    common::Time now;
    now.SetNow();
    return static_cast<int>((now - startTime).m_tick / common::TimeSpan::GetTicksPerMSec().m_tick);
}
}

CloseParticipationJob::CloseParticipationJob()
    : common::StepSequenceJob(),
      m_callerContext(0),
      m_callContext(),
      m_sessionId(0),
      m_matchmakeSession(0),
      m_startTime()
{
}

CloseParticipationJob::~CloseParticipationJob()
{
}

Result CloseParticipationJob::Startup(common::CallContext* callerContext, unsigned int sessionId,
                                      IMatchmakeSession* matchmakeSession)
{
    if (callerContext == 0 || matchmakeSession == 0)
        return static_cast<Result>(transport::cResultInvalidArgument);
    if (!matchmakeSession->IsMatchmakeOwner())
        return static_cast<Result>(transport::cResultInvalidState);

    m_callerContext = callerContext;
    callerContext->InitiateCall();
    m_sessionId = sessionId;
    m_matchmakeSession = matchmakeSession;
    Reset(true);
    SelectStep(m_currentStep, &CloseParticipationJob::CloseParticipation, "CloseParticipationJob::CloseParticipation");
    return 0;
}

CloseParticipationJob::ExecuteResult CloseParticipationJob::CloseParticipation()
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

    const Result result = m_matchmakeSession->CloseParticipate(&m_callContext, m_sessionId);
    if (result != 0)
    {
        if (result == static_cast<Result>(transport::cResultTransportUnavailable))
            return MakeExecuteResult(cExecuteKindNextStep);
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &CloseParticipationJob::WaitCloseParticipation,
               "CloseParticipationJob::WaitCloseParticipation");
    return MakeExecuteResult(cExecuteKindNextStep);
}

CloseParticipationJob::ExecuteResult CloseParticipationJob::WaitCloseParticipation()
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
    if (m_matchmakeSession->WaitCloseParticipate())
    {
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
            SelectStep(m_currentStep, &CloseParticipationJob::WaitP2pStable, "CloseParticipationJob::WaitP2pStable");
            m_startTime.SetNow();
            return MakeExecuteResult(cExecuteKindContinue);
        }
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

CloseParticipationJob::ExecuteResult CloseParticipationJob::WaitP2pStable()
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

    if (GetElapsedMsec(m_startTime) > cWaitP2pStableMinMsec
        && g_piaSession->m_meshLayerController->IsAllStationJoined(m_sessionId))
    {
        if (g_piaSession->m_sessionId[g_piaSession->m_currentSessionIndex] == m_sessionId)
        {
            g_piaSession->SetCurrentSessionOpenStatus(Session::cSessionOpenStatusParticipationClosed);
            m_callerContext->SignalSuccess(0);
        }
        else
        {
            if (g_piaSession->m_sessionId[g_piaSession->m_currentSessionIndex == 0 ? 1 : 0] == m_sessionId)
                g_piaSession->SetBufferSessionOpenStatus(Session::cSessionOpenStatusParticipationClosed);
            m_callerContext->SignalSuccess(0);
        }
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (GetElapsedMsec(m_startTime) > cWaitP2pStableTimeoutMsec)
    {
        m_callerContext->SignalFailure(static_cast<Result>(cResultWaitP2pStableTimeout));
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

void CloseParticipationJob::Cleanup()
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

void CloseParticipationJob::Trace(unsigned long long) const
{
}

}
}
}
