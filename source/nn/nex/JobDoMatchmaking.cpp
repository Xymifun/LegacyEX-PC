#include "nn/nex/JobDoMatchmaking.hpp"

#include <stdlib.h>

#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/Credentials.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/JobWaitForNotification.hpp"
#include "nn/nex/NotificationEventFilterInterface.hpp"
#include "nn/nex/Reconstruction.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

namespace
{

enum
{
    MATCHMAKING_NOTIFICATION_TYPE = 125,
    MATCHMAKING_NOTIFICATION_SUB_TYPE = 0,
    MATCHMAKING_NOTIFICATION_TIMEOUT = 0x7fffffff,
    GATHERING_ID_NUMBER_BASE = 10
};

enum NotificationReason
{
    REASON_GATHERING_FOUND = 1,
    REASON_UNAVAILABLE = 3,
    REASON_TIMED_OUT = 4,
    REASON_FAILED = 5
};

enum MatchmakingResult
{
    MATCHMAKING_CONNECTION_LOST_RESULT = static_cast<int>(0x80010004u),
    MATCHMAKING_CALL_FAILED_RESULT = static_cast<int>(0x8001000Du),
    MATCHMAKING_INVALID_RESULT = static_cast<int>(0x80010001u),
    MATCHMAKING_UNAVAILABLE_RESULT = static_cast<int>(0x800300DEu),
    MATCHMAKING_TIMED_OUT_RESULT = static_cast<int>(0x8001000Au),
    MATCHMAKING_FAILED_RESULT = static_cast<int>(0x80030070u),
    MATCHMAKING_SUCCEEDED_RESULT = 0x10001
};

}

JobDoMatchmaking::JobDoMatchmaking()
    : StepSequenceJob(JOB_TYPE_2, DebugString(L"")),
      m_uiCallId(0),
      m_pMatchmakeExtensionClient(0),
      m_autoMatchmakeParam(),
      m_pJoinedSession(0),
      m_requestContext(0),
      m_notificationContext(0),
      m_pNotificationFilter(0),
      m_notificationEvent(),
      m_uiRequestID(0),
      m_uiGatheringID(0),
      m_matchmakeSession()
{
}

JobDoMatchmaking::~JobDoMatchmaking()
{
}

qResult JobDoMatchmaking::Initialize(unsigned int uiCallId, MatchmakeExtensionClient* pMatchmakeExtensionClient,
                                     const AutoMatchmakeParam& rParam, MatchmakeSession* pJoinedSession)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());

    m_uiCallId = uiCallId;
    m_pMatchmakeExtensionClient = pMatchmakeExtensionClient;
    m_autoMatchmakeParam = rParam;
    m_pJoinedSession = pJoinedSession;

    ProtocolCallContext* pContext = static_cast<ProtocolCallContext*>(
        Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId));
    Credentials* pCredentials = pContext->GetCredentials();
    if (pCredentials == 0)
        pCredentials = m_pMatchmakeExtensionClient->GetCredentials();

    m_requestContext.SetCredentials(pCredentials);
    m_notificationContext.SetCredentials(pCredentials);
    SetStep(Step(static_cast<Step::Function>(&JobDoMatchmaking::StepRequestMatchmaking), L""));
    return qResult(true);
}

void JobDoMatchmaking::CompleteJob(const qResult& rResult)
{
    CallContext* pContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pContext == 0)
    {
        m_requestContext.Cancel(CallContext::CallCanceled);
        m_notificationContext.Cancel(CallContext::CallCanceled);
        SetToComplete();
        return;
    }

    if (rResult)
    {
        *m_pJoinedSession = m_matchmakeSession;
        pContext->SignalSuccess(rResult);
        SetToComplete();
        return;
    }

    pContext->SignalFailure(rResult);
    SetToComplete();
}

void JobDoMatchmaking::StepRequestMatchmaking()
{
    CallContext* pContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pContext == 0)
    {
        CompleteJob(qResult(MATCHMAKING_CONNECTION_LOST_RESULT));
        return;
    }

    m_requestContext.Reset();
    if (!m_pMatchmakeExtensionClient->RequestMatchmaking(&m_requestContext, m_autoMatchmakeParam, &m_uiRequestID))
    {
        CompleteJob(qResult(MATCHMAKING_CALL_FAILED_RESULT));
        return;
    }

    Core::GetInstance()->GetCallContextRegister()->AssociateCallContext(m_uiCallId, m_requestContext.GetCallID());
    m_notificationContext.Reset();
    if (!JobWaitForNotification::Start(&m_notificationContext, MATCHMAKING_NOTIFICATION_TYPE,
                                       MATCHMAKING_NOTIFICATION_SUB_TYPE, MATCHMAKING_NOTIFICATION_TIMEOUT,
                                       &m_pNotificationFilter, &m_notificationEvent))
    {
        CompleteJob(qResult(MATCHMAKING_CALL_FAILED_RESULT));
        return;
    }

    Core::GetInstance()->GetCallContextRegister()->AssociateCallContext(m_uiCallId, m_notificationContext.GetCallID());
    SetToSuspended();
    ResumeOnCallCompletion(
        &m_requestContext,
        new Step(static_cast<Step::Function>(&JobDoMatchmaking::StepWaitingRequestMatchmaking), L""));
}

void JobDoMatchmaking::StepWaitingRequestMatchmaking()
{
    CallContext* pContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pContext == 0)
    {
        CompleteJob(qResult(MATCHMAKING_CONNECTION_LOST_RESULT));
        return;
    }

    if (m_requestContext.GetState() != CallContext::CallSucceeded)
    {
        CompleteJob(m_requestContext.GetResult());
        return;
    }

    m_pNotificationFilter->SetEventUInt64Param1(m_uiRequestID);
    SetToSuspended();
    ResumeOnCallCompletionOrAlreadyDone(
        &m_notificationContext, new Step(static_cast<Step::Function>(&JobDoMatchmaking::StepWaitingNotify), L""));
}

void JobDoMatchmaking::StepWaitingNotify()
{
    CallContext* pContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pContext == 0)
    {
        CompleteJob(qResult(MATCHMAKING_CONNECTION_LOST_RESULT));
        return;
    }

    if (m_notificationContext.GetState() != CallContext::CallSucceeded)
    {
        CompleteJob(m_notificationContext.GetResult());
        return;
    }

    switch (m_notificationEvent.GetParam3())
    {
    case REASON_GATHERING_FOUND:
    {
        char* pEnd = 0;
        char* pText;
        m_notificationEvent.m_StringParam.CreateCopy(&pText);
        m_uiGatheringID = strtoul(pText, &pEnd, GATHERING_ID_NUMBER_BASE);
        String::ReleaseCopy(pText);
        if (m_uiGatheringID == 0)
        {
            CompleteJob(qResult(MATCHMAKING_INVALID_RESULT));
            return;
        }

        SetStep(Step(static_cast<Step::Function>(&JobDoMatchmaking::StepFindMatchmakeSessionByGatheringIdDetail), L""));
        return;
    }
    case REASON_UNAVAILABLE:
        CompleteJob(qResult(MATCHMAKING_UNAVAILABLE_RESULT));
        return;
    case REASON_TIMED_OUT:
        CompleteJob(qResult(MATCHMAKING_TIMED_OUT_RESULT));
        return;
    case REASON_FAILED:
        CompleteJob(qResult(MATCHMAKING_FAILED_RESULT));
        return;
    default:
        CompleteJob(qResult(MATCHMAKING_INVALID_RESULT));
        return;
    }
}

void JobDoMatchmaking::StepFindMatchmakeSessionByGatheringIdDetail()
{
    CallContext* pContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pContext == 0)
    {
        CompleteJob(qResult(MATCHMAKING_CONNECTION_LOST_RESULT));
        return;
    }

    m_requestContext.Reset();
    if (!m_pMatchmakeExtensionClient->FindMatchmakeSessionByGatheringIdDetail(&m_requestContext, m_uiGatheringID,
                                                                              &m_matchmakeSession))
    {
        CompleteJob(qResult(MATCHMAKING_CALL_FAILED_RESULT));
        return;
    }

    Core::GetInstance()->GetCallContextRegister()->AssociateCallContext(m_uiCallId, m_requestContext.GetCallID());
    SetToSuspended();
    ResumeOnCallCompletion(
        &m_requestContext,
        new Step(static_cast<Step::Function>(&JobDoMatchmaking::StepWaitingFindMatchmakeSessionByGatheringIdDetail),
                 L""));
}

void JobDoMatchmaking::StepWaitingFindMatchmakeSessionByGatheringIdDetail()
{
    CallContext* pContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pContext == 0)
    {
        CompleteJob(qResult(MATCHMAKING_CONNECTION_LOST_RESULT));
        return;
    }

    if (m_requestContext.GetState() != CallContext::CallSucceeded)
    {
        CompleteJob(m_requestContext.GetResult());
        return;
    }

    CompleteJob(qResult(MATCHMAKING_SUCCEEDED_RESULT));
}

}
}
