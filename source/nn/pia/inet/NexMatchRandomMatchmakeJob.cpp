#include "nn/pia/inet/NexMatchRandomMatchmakeJob.hpp"

#include "nn/nex/NgsBridgeInterface.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/inet/NexCreateSessionSetting.hpp"
#include "nn/pia/inet/NexFacade.hpp"
#include "nn/pia/inet/NexMatchMeshLayerController.hpp"
#include "nn/pia/inet/NexMatchmakeSession.hpp"
#include "nn/pia/inet/NexSessionInfo.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/JoinRandomSessionSetting.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/StationIdStatusTable.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

namespace
{
enum
{
    cResultNatTraversalFailed = 0x643f,
    cResultNatTraversalFailedEimToEdm = 0x6443,
    cResultNatTraversalFailedEdmToEim = 0x6444,
    cResultNatTraversalFailedEdmToEdm = 0x6445,
    cResultPortDetectionFailed = 0x6434,
    cResultNatServerAddressUnavailable = 0x6436,
    cResultCancelled = 0x6c05,
    cResultRequestFailed = 0xac5a,
    cResultNexError = 0xe437
};

enum
{
    cJoinStateLeaveJoinedSession = 1,
    cJoinStateRandomMatchmaking = 2,
    cJoinStateNatSession = 3,
    cJoinStateRetryWait = 8
};

enum
{
    cSearchCriteriaNumMax = 2,
    cRetryCountMax = 2,
    cJoinMeshStateHostMigrating = 3,
    cOwnerStationNumMax = 2,
    cMonitoringByteUnset = 0xff,
    cMonitoringShortUnset = 0xffff
};

enum
{
    cNotificationTimeoutMsec = 10000,
    cRetryTimeoutMsec = 30000,
    cRetryIntervalMsec = 10000,
    cRetryIntervalLongMsec = 60000
};

const unsigned long long cTraceFlagSession = 0x0000000008000000ULL;

const char* const cCompleteFailureStepName = "nn::pia::inet::NexMatchRandomMatchmakeJob::CompleteFailure";

inline NexMatchMeshLayerController* meshLayerController()
{
    return static_cast<NexMatchMeshLayerController*>(session::g_piaSession->m_meshLayerController);
}

inline unsigned int bufferSessionIndex()
{
    return session::g_piaSession->m_currentSessionIndex == 0 ? 1 : 0;
}

inline common::Time timeAfter(unsigned int msec)
{
    const common::TimeSpan span = common::TimeSpan::FromMSec(msec);
    return common::g_piaJobScheduler->m_dispatchTime + span;
}

inline void countRetry(unsigned char& count)
{
    if (count == cMonitoringByteUnset)
        count = 1;
    else
        ++count;
}

}

NexMatchRandomMatchmakeJob::NexMatchRandomMatchmakeJob()
    : session::RandomMatchmakeJob(),
      m_matchmakeSession(0),
      m_retryTimeoutTime(),
      m_retryTime(),
      m_notificationTimeoutTime(),
      m_isOpenSession(false),
      m_natStartTime()
{
}

NexMatchRandomMatchmakeJob::~NexMatchRandomMatchmakeJob()
{
}

NexMatchRandomMatchmakeJob::ExecuteResult NexMatchRandomMatchmakeJob::LeaveJoinedMatchmakeSession()
{
    const unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        m_result = session::Session::cResultStatusUnknown6;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        m_result = session::Session::cResultStatusUnknown7;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        m_result = session::Session::cResultStatusUnknown8;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    m_joinState = cJoinStateLeaveJoinedSession;
    if (m_joinedSessionIdNum == 0)
    {
        setStep(&NexMatchRandomMatchmakeJob::RandomMatchmake, "NexMatchRandomMatchmakeJob::RandomMatchmake");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    const unsigned int sessionId = m_joinedSessionId[m_joinedSessionIdNum - 1];
    session::IMatchmakeSession* matchmakeSession =
        session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex];
    m_callContext.Reset();
    const Result result = matchmakeSession->LeaveMatchmakeSession(&m_callContext, sessionId);
    if (result != 0)
    {
        if (result == cResultRequestFailed)
        {
            m_result = result;
            setStep(&session::RandomMatchmakeJob::CompleteFailure, "NexMatchRandomMatchmakeJob::CompleteFailure");
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        if (m_joinedSessionIdNum != 0)
            --m_joinedSessionIdNum;
        return MakeExecuteResult(cExecuteKindContinue);
    }
    setStep(&NexMatchRandomMatchmakeJob::WaitLeaveJoinedMatchmakeSession,
            "NexMatchRandomMatchmakeJob::WaitLeaveJoinedMatchmakeSession");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchRandomMatchmakeJob::ExecuteResult NexMatchRandomMatchmakeJob::WaitLeaveJoinedMatchmakeSession()
{
    const unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        m_result = session::Session::cResultStatusUnknown6;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        m_result = session::Session::cResultStatusUnknown7;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        m_result = session::Session::cResultStatusUnknown8;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    session::Session* session = session::g_piaSession;
    if (session->m_matchmakeSession[session->m_currentSessionIndex]->WaitLeaveMatchmakeSession())
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            const Result result = m_callContext.m_result;
            if (result == session::Session::cResultStatusDisconnectedNetwork || result == cResultNexError)
                m_result = result;
        }
        if (m_joinedSessionIdNum != 0)
            --m_joinedSessionIdNum;
        setStep(&NexMatchRandomMatchmakeJob::LeaveJoinedMatchmakeSession,
                "NexMatchRandomMatchmakeJob::LeaveJoinedMatchmakeSession");
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchRandomMatchmakeJob::ExecuteResult NexMatchRandomMatchmakeJob::RandomMatchmake()
{
    const unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        m_result = session::Session::cResultStatusUnknown6;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        m_result = session::Session::cResultStatusUnknown7;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        m_result = session::Session::cResultStatusUnknown8;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    if (CheckContextCallCancelled())
    {
        setStep(&session::RandomMatchmakeJob::CompleteFailure, "NexMatchRandomMatchmakeJob::CompleteFailure");
        m_result = cResultCancelled;
        return MakeExecuteResult(cExecuteKindContinue);
    }
    m_ownerPrincipalId = 0;
    m_natStartTime = common::g_piaJobScheduler->m_dispatchTime;
    meshLayerController()->CleanupEventHandler();
    m_ngsInterfaceAvailable = false;
    const Result result = m_matchmakeSession->RandomMatchmakeSession(&m_callContext);
    if (result != 0)
    {
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(result);
            m_callerContext = 0;
        }
        m_matchmakeSession->UnregisterSearchCriteria();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    m_joinState = cJoinStateRandomMatchmaking;
    setStep(&NexMatchRandomMatchmakeJob::WaitRandomMatchmake, "NexMatchRandomMatchmakeJob::WaitRandomMatchmake");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchRandomMatchmakeJob::ExecuteResult NexMatchRandomMatchmakeJob::WaitRandomMatchmake()
{
    const unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        m_result = session::Session::cResultStatusUnknown6;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        m_result = session::Session::cResultStatusUnknown7;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        m_result = session::Session::cResultStatusUnknown8;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    if (m_matchmakeSession->WaitRandomMatchmakeSession(&m_sessionId, &m_isNewSession))
    {
        m_matchmakeSession->UnregisterSearchCriteria();
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            m_result = m_callContext.m_result;
            setStep(&NexMatchRandomMatchmakeJob::LeaveMatchmakeSession,
                    "NexMatchRandomMatchmakeJob::LeaveMatchmakeSession");
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex] = m_sessionId;
            if (!m_isNewSession)
            {
                if (m_matchmakeSession->IsRandomMatchmakeScoreAvailable())
                {
                    const unsigned int score = m_matchmakeSession->GetRandomMatchmakeScore();
                    session::g_piaSession->m_field140 = true;
                    session::g_piaSession->m_field13c = score;
                }
            }
            else
            {
                session::g_piaSession->SetCurrentSessionOpenStatus(
                    m_isOpenSession ? session::Session::cSessionOpenStatusOpen
                                    : session::Session::cSessionOpenStatusParticipationClosed);
            }
            SetMatchmakeStatus(session::Session::cMatchmakeStatusSearching);
            m_ownerPrincipalId = m_matchmakeSession->GetMatchmakeOwnerUniqueId();
            if (meshLayerController()->GetNgsInterface(0) != 0)
                m_ngsInterfaceAvailable = true;
            m_notificationTimeoutTime = timeAfter(cNotificationTimeoutMsec);
            setStep(&NexMatchRandomMatchmakeJob::WaitNotification, "NexMatchRandomMatchmakeJob::WaitNotification");
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchRandomMatchmakeJob::ExecuteResult NexMatchRandomMatchmakeJob::WaitNotification()
{
    const unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        m_result = session::Session::cResultStatusUnknown6;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        m_result = session::Session::cResultStatusUnknown7;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        m_result = session::Session::cResultStatusUnknown8;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    if (CheckContextCallCancelled())
    {
        setStep(&NexMatchRandomMatchmakeJob::LeaveMatchmakeSession,
                "NexMatchRandomMatchmakeJob::LeaveMatchmakeSession");
        m_result = cResultCancelled;
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    if (m_notificationTimeoutTime < common::g_piaJobScheduler->m_dispatchTime)
    {
        setStep(&NexMatchRandomMatchmakeJob::LeaveMatchmakeSession,
                "NexMatchRandomMatchmakeJob::LeaveMatchmakeSession");
        m_result = cResultRequestFailed;
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    NexMatchMeshLayerController* controller = meshLayerController();
    nex::NgsBridgeInterface* ngsInterface = controller->GetNgsInterface(0);
    if (ngsInterface != 0 && !controller->IsParticipated(m_sessionId, ngsInterface->GetPrincipalID()))
    {
        controller->Trace(cTraceFlagSession);
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    setStep(&NexMatchRandomMatchmakeJob::StartNatSession, "NexMatchRandomMatchmakeJob::StartNatSession");
    common::g_sessionBeginMonitoringContent.m_metric80 = static_cast<unsigned int>(
        static_cast<long long>(common::g_piaJobScheduler->m_dispatchTime.m_tick - m_natStartTime.m_tick)
        / static_cast<long long>(common::TimeSpan::GetTicksPerMSec().m_tick));
    return MakeExecuteResult(cExecuteKindContinue);
}

NexMatchRandomMatchmakeJob::ExecuteResult NexMatchRandomMatchmakeJob::StartNatSession()
{
    if (CheckContextCallCancelled())
    {
        setStep(&NexMatchRandomMatchmakeJob::LeaveMatchmakeSession,
                "NexMatchRandomMatchmakeJob::LeaveMatchmakeSession");
        m_result = cResultCancelled;
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    NexMatchMeshLayerController* controller = meshLayerController();
    const Result result = controller->StartNatSession();
    if (result != 0)
    {
        m_result = result;
        setStep(&NexMatchRandomMatchmakeJob::LeaveMatchmakeSession,
                "NexMatchRandomMatchmakeJob::LeaveMatchmakeSession");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    m_joinState = cJoinStateNatSession;
    setStep(&NexMatchRandomMatchmakeJob::WaitStartNatSession, "NexMatchRandomMatchmakeJob::WaitStartNatSession");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchRandomMatchmakeJob::ExecuteResult NexMatchRandomMatchmakeJob::WaitStartNatSession()
{
    const unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        m_result = session::Session::cResultStatusUnknown6;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        m_result = session::Session::cResultStatusUnknown7;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        m_result = session::Session::cResultStatusUnknown8;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    NexMatchMeshLayerController* controller = meshLayerController();
    Result result = 0;
    const ExecuteKind kind = controller->WaitStartNatSession(&result);
    if (CheckContextCallCancelled())
        controller->CancelStartNatSession();
    if (kind == cExecuteKindContinue)
    {
        setStep(&session::RandomMatchmakeJob::MeshStartup, "RandomMatchmakeJob::MeshStartup");
    }
    else if (kind == cExecuteKindFinished)
    {
        if (result == cResultNatServerAddressUnavailable || result == cResultPortDetectionFailed
            || result == cResultCancelled || result == transport::cResultInvalidState)
        {
            m_result = result;
            setStep(&NexMatchRandomMatchmakeJob::LeaveMatchmakeSession,
                    "NexMatchRandomMatchmakeJob::LeaveMatchmakeSession");
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        if (result == session::Session::cResultStatusDisconnectedNetwork)
        {
            SetConnectionStatus(session::Session::cConnectionStatusDisconnectedNetwork);
            setStep(&session::RandomMatchmakeJob::CompleteFailure, "NexMatchRandomMatchmakeJob::CompleteFailure");
            m_result = session::Session::cResultStatusDisconnectedNetwork;
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        if (result == session::Session::cResultStatusUnknown6)
        {
            SetConnectionStatus(session::Session::cConnectionStatusUnknown4);
            m_result = session::Session::cResultStatusUnknown6;
            setStep(&session::RandomMatchmakeJob::CompleteFailure, "NexMatchRandomMatchmakeJob::CompleteFailure");
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        if (result == session::Session::cResultStatusUnknown7)
        {
            SetConnectionStatus(session::Session::cConnectionStatusUnknown5);
            setStep(&session::RandomMatchmakeJob::CompleteFailure, "NexMatchRandomMatchmakeJob::CompleteFailure");
            m_result = session::Session::cResultStatusUnknown7;
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        if (result == session::Session::cResultStatusUnknown8)
        {
            SetConnectionStatus(session::Session::cConnectionStatusUnknown6);
            setStep(&session::RandomMatchmakeJob::CompleteFailure, "NexMatchRandomMatchmakeJob::CompleteFailure");
            m_result = session::Session::cResultStatusUnknown8;
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        m_result = transport::cResultInvalidState;
        setStep(&NexMatchRandomMatchmakeJob::LeaveMatchmakeSession,
                "NexMatchRandomMatchmakeJob::LeaveMatchmakeSession");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    return MakeExecuteResult(kind);
}

NexMatchRandomMatchmakeJob::ExecuteResult NexMatchRandomMatchmakeJob::LeaveMatchmakeSession()
{
    const unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        m_result = session::Session::cResultStatusUnknown6;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        m_result = session::Session::cResultStatusUnknown7;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        m_result = session::Session::cResultStatusUnknown8;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    session::Session* session = session::g_piaSession;
    NexMatchMeshLayerController* controller = meshLayerController();
    const unsigned int bufferSessionId = session->m_sessionId[bufferSessionIndex()];
    if (bufferSessionId != 0)
    {
        m_callContext.Reset();
        const Result result = session->m_matchmakeSession[bufferSessionIndex()]->LeaveMatchmakeSession(&m_callContext,
                                                                                                       bufferSessionId);
        if (result == 0)
        {
            setStep(&NexMatchRandomMatchmakeJob::WaitLeaveBufferMatchmakeSession,
                    "NexMatchRandomMatchmakeJob::WaitLeaveBufferMatchmakeSession");
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        if (result == cResultRequestFailed)
            m_result = result;
        session::g_piaSession->m_sessionId[bufferSessionIndex()] = 0;
    }
    if (controller != 0)
    {
        for (unsigned int i = 0; i < controller->GetSessionIdArraySize(); ++i)
        {
            const unsigned int sessionId = controller->GetSessionId(i);
            if (sessionId == 0)
                continue;
            if (!controller->IsParticipated(sessionId, NexFacade::GetInstance()->GetLocalPrincipalId()))
            {
                controller->ClearSessionId(sessionId);
                continue;
            }
            if (sessionId != session->m_sessionId[session->m_currentSessionIndex])
            {
                session::g_piaSession->m_sessionId[bufferSessionIndex()] = sessionId;
                m_callContext.Reset();
                const Result result = session->m_matchmakeSession[bufferSessionIndex()]->LeaveMatchmakeSession(
                    &m_callContext, sessionId);
                if (result == 0)
                {
                    setStep(&NexMatchRandomMatchmakeJob::WaitLeaveBufferMatchmakeSession,
                            "NexMatchRandomMatchmakeJob::WaitLeaveBufferMatchmakeSession");
                    return MakeExecuteResult(cExecuteKindNextStep);
                }
                if (result == cResultRequestFailed)
                    m_result = result;
                session::g_piaSession->m_sessionId[bufferSessionIndex()] = 0;
            }
        }
    }
    const unsigned int index = session->m_currentSessionIndex;
    const unsigned int currentSessionId = session->m_sessionId[index];
    if (currentSessionId != 0)
    {
        session::IMatchmakeSession* matchmakeSession = session->m_matchmakeSession[index];
        const Result result = m_isNewSession
                                  ? matchmakeSession->UnregisterMatchmakeSession(&m_callContext, currentSessionId)
                                  : matchmakeSession->LeaveMatchmakeSession(&m_callContext, currentSessionId);
        if (result == 0)
        {
            setStep(&NexMatchRandomMatchmakeJob::WaitLeaveCurrentMatchmakeSession,
                    "NexMatchRandomMatchmakeJob::WaitLeaveCurrentMatchmakeSession");
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        if (result == cResultRequestFailed)
            m_result = result;
        session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex] = 0;
    }
    setStep(&session::RandomMatchmakeJob::CompleteFailure, "NexMatchRandomMatchmakeJob::CompleteFailure");
    return MakeExecuteResult(cExecuteKindContinue);
}

NexMatchRandomMatchmakeJob::ExecuteResult NexMatchRandomMatchmakeJob::WaitLeaveCurrentMatchmakeSession()
{
    const unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        m_result = session::Session::cResultStatusUnknown6;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        m_result = session::Session::cResultStatusUnknown7;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        m_result = session::Session::cResultStatusUnknown8;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    session::Session* session = session::g_piaSession;
    session::IMatchmakeSession* matchmakeSession = session->m_matchmakeSession[session->m_currentSessionIndex];
    const bool completed = m_isNewSession ? matchmakeSession->WaitUnregisterMatchmakeSession()
                                          : matchmakeSession->WaitLeaveMatchmakeSession();
    if (!completed)
        return MakeExecuteResult(cExecuteKindNextStep);
    if (m_callContext.m_state == common::CallContext::cStateFailure)
    {
        const Result result = m_callContext.m_result;
        if (result == session::Session::cResultStatusDisconnectedNetwork)
            m_result = result;
        else if (result == cResultNexError)
            m_result = cResultNexError;
    }
    session->m_sessionId[session->m_currentSessionIndex] = 0;
    setStep(&NexMatchRandomMatchmakeJob::LeaveMatchmakeSession, "NexMatchRandomMatchmakeJob::LeaveMatchmakeSession");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchRandomMatchmakeJob::ExecuteResult NexMatchRandomMatchmakeJob::WaitLeaveBufferMatchmakeSession()
{
    const unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        m_result = session::Session::cResultStatusUnknown6;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        m_result = session::Session::cResultStatusUnknown7;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        m_result = session::Session::cResultStatusUnknown8;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    if (!session::g_piaSession->m_matchmakeSession[bufferSessionIndex()]->WaitLeaveMatchmakeSession())
        return MakeExecuteResult(cExecuteKindNextStep);
    if (m_callContext.m_state == common::CallContext::cStateFailure)
    {
        const Result result = m_callContext.m_result;
        if (result == session::Session::cResultStatusDisconnectedNetwork)
            m_result = result;
        else if (result == cResultNexError)
            m_result = cResultNexError;
    }
    session::g_piaSession->m_sessionId[bufferSessionIndex()] = 0;
    setStep(&NexMatchRandomMatchmakeJob::LeaveMatchmakeSession, "NexMatchRandomMatchmakeJob::LeaveMatchmakeSession");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchRandomMatchmakeJob::ExecuteResult NexMatchRandomMatchmakeJob::ChangeParticipation()
{
    const unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        m_result = session::Session::cResultStatusUnknown6;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        m_result = session::Session::cResultStatusUnknown7;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        m_result = session::Session::cResultStatusUnknown8;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    if (CheckContextCallCancelled())
    {
        setStep(&NexMatchRandomMatchmakeJob::LeaveMatchmakeSession,
                "NexMatchRandomMatchmakeJob::LeaveMatchmakeSession");
        m_result = cResultCancelled;
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    session::IMatchmakeSession* matchmakeSession =
        session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex];
    const Result result = m_isOpenSession ? matchmakeSession->OpenParticipate(&m_callContext, m_sessionId)
                                          : matchmakeSession->CloseParticipate(&m_callContext, m_sessionId);
    if (result != 0)
    {
        m_result = result;
        setStep(&NexMatchRandomMatchmakeJob::LeaveMatchmakeSession,
                "NexMatchRandomMatchmakeJob::LeaveMatchmakeSession");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    setStep(&NexMatchRandomMatchmakeJob::WaitChangeParticipation,
            "NexMatchRandomMatchmakeJob::WaitChangeParticipation");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchRandomMatchmakeJob::ExecuteResult NexMatchRandomMatchmakeJob::WaitChangeParticipation()
{
    const unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        m_result = session::Session::cResultStatusUnknown6;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        m_result = session::Session::cResultStatusUnknown7;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        m_result = session::Session::cResultStatusUnknown8;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    session::IMatchmakeSession* matchmakeSession =
        session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex];
    const bool completed = m_isOpenSession ? matchmakeSession->WaitOpenParticipate()
                                           : matchmakeSession->WaitCloseParticipate();
    if (completed)
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            m_result = m_callContext.m_result;
            setStep(&NexMatchRandomMatchmakeJob::LeaveMatchmakeSession,
                    "NexMatchRandomMatchmakeJob::LeaveMatchmakeSession");
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            session::g_piaSession->SetCurrentSessionOpenStatus(
                m_isOpenSession ? session::Session::cSessionOpenStatusOpen
                                : session::Session::cSessionOpenStatusParticipationClosed);
            matchmakeSession->UpdateMatchmakeSessionHost(m_sessionId);
            setStep(&NexMatchRandomMatchmakeJob::StartNatSession, "NexMatchRandomMatchmakeJob::StartNatSession");
            m_isNewSession = true;
            return MakeExecuteResult(cExecuteKindNextStep);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchRandomMatchmakeJob::ExecuteResult NexMatchRandomMatchmakeJob::CleanupForRetryJoinMesh()
{
    countRetry(common::g_sessionBeginMonitoringContent.m_joinRetryCount);
    session::g_piaSession->m_postponeNotice = false;
    session::g_piaSession->m_stationIdStatusTable->ClearDisconnectedStationId();
    if (m_isNewSession)
    {
        setStep(&NexMatchRandomMatchmakeJob::StartNatSession, "NexMatchRandomMatchmakeJob::StartNatSession");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_sessionInfoRetry || m_retryReasonB || m_retryReasonA || m_ownerChanged)
    {
        m_retryTimeoutTime = timeAfter(cRetryTimeoutMsec);
        if (m_sessionInfoRetry && !m_ownerChanged)
            m_retryTime = timeAfter(cRetryIntervalLongMsec);
        else
            m_retryTime = timeAfter(cRetryIntervalMsec);
        m_waitingForRetryJoinMesh = true;
        setStep(&NexMatchRandomMatchmakeJob::WaitForRetryJoinMesh, "NexMatchRandomMatchmakeJob::WaitForRetryJoinMesh");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    if (m_connectionInfoRetry)
    {
        setStep(&NexMatchRandomMatchmakeJob::StartNatSession, "NexMatchRandomMatchmakeJob::StartNatSession");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    setStep(&NexMatchRandomMatchmakeJob::LeaveMatchmakeSession, "NexMatchRandomMatchmakeJob::LeaveMatchmakeSession");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchRandomMatchmakeJob::ExecuteResult NexMatchRandomMatchmakeJob::WaitForRetryJoinMesh()
{
    switch (session::g_piaSession->GetStatus())
    {
    case session::Session::cStatusDisconnectedNetwork:
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        m_result = session::Session::cResultStatusUnknown6;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        m_result = session::Session::cResultStatusUnknown7;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        m_result = session::Session::cResultStatusUnknown8;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, cCompleteFailureStepName);
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    m_joinState = cJoinStateRetryWait;
    if (CheckContextCallCancelled())
    {
        m_waitingForRetryJoinMesh = false;
        m_result = cResultCancelled;
        setStep(&NexMatchRandomMatchmakeJob::LeaveMatchmakeSession,
                "NexMatchRandomMatchmakeJob::LeaveMatchmakeSession");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    const common::Time& now = common::g_piaJobScheduler->m_dispatchTime;
    if (m_retryTime < now)
    {
        m_waitingForRetryJoinMesh = false;
        setStep(&NexMatchRandomMatchmakeJob::StartNatSession, "NexMatchRandomMatchmakeJob::StartNatSession");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_retryTimeoutTime < now)
    {
        setStep(&NexMatchRandomMatchmakeJob::LeaveMatchmakeSession,
                "NexMatchRandomMatchmakeJob::LeaveMatchmakeSession");
        m_waitingForRetryJoinMesh = false;
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_ownerPrincipalId != 0 && NexFacade::GetInstance()->GetLocalPrincipalId() == m_ownerPrincipalId)
    {
        const unsigned int stationNum = session::g_piaSession->GetMatchmakeSessionStationNum();
        m_waitingForRetryJoinMesh = false;
        if (stationNum == 1)
        {
            m_ownerChanged = false;
            setStep(&NexMatchRandomMatchmakeJob::ChangeParticipation,
                    "NexMatchRandomMatchmakeJob::ChangeParticipation");
            return MakeExecuteResult(cExecuteKindContinue);
        }
        setStep(&NexMatchRandomMatchmakeJob::LeaveMatchmakeSession,
                "NexMatchRandomMatchmakeJob::LeaveMatchmakeSession");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

void NexMatchRandomMatchmakeJob::NoticeOwnerChangeEvent(unsigned int sessionId, unsigned long long ownerId)
{
    if (m_sessionId != sessionId)
        return;
    if (NexFacade::GetInstance()->GetLocalPrincipalId() == ownerId)
    {
        if (session::g_piaSession->GetMatchmakeSessionStationNum() <= cOwnerStationNumMax)
        {
            NexMatchMeshLayerController* controller = meshLayerController();
            if (session::g_piaSession->GetMatchmakeSessionStationNum() != cOwnerStationNumMax
                || controller->IsParticipated(m_sessionId, m_ownerPrincipalId))
            {
                m_ownerPrincipalId = ownerId;
                m_ownerChanged = true;
                return;
            }
        }
        m_ownerPrincipalId = ownerId;
        m_sessionInvalidated = true;
        return;
    }
    if (!session::g_piaMesh->IsHostMigrationEnabled()
        || (!session::g_piaMesh->IsHostMigrationStarted() && !session::g_piaMesh->IsProcessingHostMigration())
        || session::g_piaMesh->GetJoinMeshState() != cJoinMeshStateHostMigrating)
    {
        m_ownerChanged = true;
        if (m_waitingForRetryJoinMesh)
        {
            m_retryTime = timeAfter(cRetryIntervalMsec);
            if (m_retryTimeoutTime < m_retryTime)
                m_retryTime = m_retryTimeoutTime;
        }
    }
    m_ownerPrincipalId = ownerId;
}

void NexMatchRandomMatchmakeJob::NoticeUnregisterEvent(unsigned int sessionId)
{
    if (m_sessionId == sessionId)
        m_sessionInvalidated = true;
}

void NexMatchRandomMatchmakeJob::Trace(unsigned long long) const
{
}

Result NexMatchRandomMatchmakeJob::StartupImpl(const session::JoinRandomSessionSetting* setting)
{
    session::Session* session = session::g_piaSession;
    if (session->GetLocalPlayerCount() == 0)
        return transport::cResultInvalidState;
    if (setting->GetSessionSearchCriteriaSize() > cSearchCriteriaNumMax)
        return transport::cResultInvalidArgument;
    m_matchmakeSession = static_cast<NexMatchmakeSession*>(session->m_matchmakeSession[session->m_currentSessionIndex]);
    if (m_matchmakeSession == 0)
        return transport::cResultInvalidState;
    const NexCreateSessionSetting* createSetting = static_cast<const NexCreateSessionSetting*>(
        setting->GetCreateSessionSetting());
    if (createSetting->GetMaxParticipantNum() == 0)
        return transport::cResultInvalidArgument;
    const unsigned short participantNum = createSetting->GetMaxParticipantNum()
                                          + createSetting->GetExtraParticipantNum();
    if (transport::g_piaTransport->m_stationNum < participantNum)
        return transport::cResultInvalidArgument;
    if (m_matchmakeSession->ReflectJoinRandomSetting(setting, session->IsHostMigrationEnabled()) != 0)
        return transport::cResultInvalidArgument;
    m_isOpenSession = createSetting->IsOpenSession();
    session::g_piaSession->m_fieldc2[session::g_piaSession->m_currentSessionIndex] = participantNum;
    m_waitingForRetryJoinMesh = false;
    NexMatchMeshLayerController* controller = static_cast<NexMatchMeshLayerController*>(session->m_meshLayerController);
    m_joinedSessionIdNum = 0;
    for (unsigned int i = 0; i < controller->GetSessionIdArraySize(); ++i)
    {
        if (controller->GetSessionId(i) != 0)
        {
            m_joinedSessionId[m_joinedSessionIdNum] = controller->GetSessionId(i);
            ++m_joinedSessionIdNum;
        }
    }
    setStep(&NexMatchRandomMatchmakeJob::LeaveJoinedMatchmakeSession,
            "NexMatchRandomMatchmakeJob::LeaveJoinedMatchmakeSession");
    common::g_sessionBeginMonitoringContent.m_metric137 = cMonitoringShortUnset;
    common::g_sessionBeginMonitoringContent.m_metric138 = cMonitoringShortUnset;
    return 0;
}

void NexMatchRandomMatchmakeJob::CleanupImpl()
{
    m_ownerPrincipalId = 0;
    m_waitingForRetryJoinMesh = false;
    if (m_matchmakeSession != 0)
    {
        m_matchmakeSession->UnregisterSearchCriteria();
        m_matchmakeSession = 0;
    }
    m_isOpenSession = false;
}

void NexMatchRandomMatchmakeJob::SetNextStepAtWaitCreateMesh()
{
    setStep(&session::RandomMatchmakeJob::CompleteProcess, "RandomMatchmakeJob::CompleteProcess");
}

void NexMatchRandomMatchmakeJob::SetNextStepAtWaitJoinMesh()
{
    session::Session* session = session::g_piaSession;
    NexSessionInfo* sessionInfo = static_cast<NexSessionInfo*>(
        session->m_matchmakeSession[session->m_currentSessionIndex]->GetSessionInfo());
    sessionInfo->SetHostPrincipalId(
        session->m_matchmakeSession[session->m_currentSessionIndex]->GetMatchmakeOwnerUniqueId());
    sessionInfo->SetCurrentParticipantNum(session->GetMatchmakeSessionParticipantNum());
    setStep(&session::RandomMatchmakeJob::CompleteProcess, "RandomMatchmakeJob::CompleteProcess");
}

NexMatchRandomMatchmakeJob::ExecuteResult NexMatchRandomMatchmakeJob::SetNextStepAtFailedCreateMesh(Result result)
{
    m_result = result;
    setStep(&NexMatchRandomMatchmakeJob::LeaveMatchmakeSession, "NexMatchRandomMatchmakeJob::LeaveMatchmakeSession");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchRandomMatchmakeJob::ExecuteResult NexMatchRandomMatchmakeJob::SetNextStepAtFailedJoinMesh(Result result)
{
    m_result = result;
    if (!m_sessionInvalidated)
    {
        if (m_isNewSession)
        {
            setStep(&NexMatchRandomMatchmakeJob::CleanupForRetryJoinMesh,
                    "NexMatchRandomMatchmakeJob::CleanupForRetryJoinMesh");
            return MakeExecuteResult(cExecuteKindContinue);
        }
        ++m_retryCount;
        if (m_retryCount < cRetryCountMax)
        {
            common::SessionBeginMonitoringContent& monitoring = common::g_sessionBeginMonitoringContent;
            bool retry = true;
            if (m_ownerChanged)
            {
                countRetry(monitoring.m_ownerChangeRetryCount);
            }
            else if (m_sessionInfoRetry)
            {
                countRetry(monitoring.m_sessionInfoRetryCount);
            }
            else if (m_retryReasonB)
            {
                countRetry(monitoring.m_retryReasonBCount);
            }
            else if (m_retryReasonA)
            {
                countRetry(monitoring.m_retryReasonACount);
            }
            else if (!m_connectionInfoRetry
                     && (result == cResultNatTraversalFailed || result == cResultNatTraversalFailedEdmToEdm
                         || result == cResultNatTraversalFailedEdmToEim || result == cResultNatTraversalFailedEimToEdm))
            {
                m_connectionInfoRetry = true;
                countRetry(monitoring.m_connectionInfoRetryCount);
            }
            else
            {
                retry = false;
            }
            if (retry)
            {
                setStep(&NexMatchRandomMatchmakeJob::CleanupForRetryJoinMesh,
                        "NexMatchRandomMatchmakeJob::CleanupForRetryJoinMesh");
                return MakeExecuteResult(cExecuteKindContinue);
            }
        }
    }
    session::g_piaSession->m_postponeNotice = false;
    session::g_piaSession->m_stationIdStatusTable->ClearDisconnectedStationId();
    setStep(&NexMatchRandomMatchmakeJob::LeaveMatchmakeSession, "NexMatchRandomMatchmakeJob::LeaveMatchmakeSession");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchRandomMatchmakeJob::ExecuteResult NexMatchRandomMatchmakeJob::SetNextStepAtWaitLeaveMesh()
{
    setStep(&NexMatchRandomMatchmakeJob::LeaveMatchmakeSession, "NexMatchRandomMatchmakeJob::LeaveMatchmakeSession");
    return MakeExecuteResult(cExecuteKindNextStep);
}

}
}
}
