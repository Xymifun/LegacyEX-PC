#include "nn/pia/lan/LanMatchRandomMatchmakeJob.hpp"

#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/lan/LanCreateSessionSetting.hpp"
#include "nn/pia/lan/LanFacade.hpp"
#include "nn/pia/lan/LanMatchmakeSession.hpp"
#include "nn/pia/session/CreateSessionSetting.hpp"
#include "nn/pia/session/JoinRandomSessionSetting.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/MeshLayerController.hpp"
#include "nn/pia/session/MeshProtocol.hpp"
#include "nn/pia/session/StationIdStatusTable.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

namespace
{
enum
{
    cResultJoinFailed = 0x6439,
    cResultCancelled = 0x6c05,
    cResultSessionIgnored = 0x6c3d,
    cResultSessionIgnoredRetryOver = 0x6c4c,
    cResultInvalidArgument = 0x10c07,
    cResultInvalidState = 0x10c08
};

enum
{
    cJoinStateRandomMatchmaking = 2,
    cJoinStateRetryWait = 8
};

enum
{
    cRetryCountMax = 3,
    cRetryMarginMsec = 9000,
    cStationNumRetryThreshold = 2,
    cWaitIntervalMsec = 500
};

inline common::Time makeRetryTime()
{
    const int waitMsec = session::g_piaMesh->GetMeshProtocol()->GetUnknown24() + cRetryMarginMsec;
    common::Time time;
    time.m_tick = common::TimeSpan::GetTicksPerMSec().m_tick * waitMsec
                  + common::g_piaJobScheduler->m_dispatchTime.m_tick;
    return time;
}

inline void cleanupForRetry()
{
    session::Session* session = session::g_piaSession;
    session->m_postponeNotice = false;
    session->m_stationIdStatusTable->ClearDisconnectedStationId();
    session->m_meshLayerController->MeshCleanup();
}
}

LanMatchRandomMatchmakeJob::LanMatchRandomMatchmakeJob()
    : session::RandomMatchmakeJob(),
      m_subCallContext(),
      m_matchmakeSession(0),
      m_retryTime(),
      m_isOpenSession(true)
{
}

LanMatchRandomMatchmakeJob::~LanMatchRandomMatchmakeJob()
{
}

LanMatchRandomMatchmakeJob::ExecuteResult LanMatchRandomMatchmakeJob::RandomMatchmake()
{
    if (session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedNetwork)
    {
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        setStep(&session::RandomMatchmakeJob::CompleteFailure,
                "nn::pia::lan::LanMatchRandomMatchmakeJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        m_callerContext->SignalFailure(cResultCancelled);
        m_callerContext = 0;
        Cleanup();
        return MakeExecuteResult(cExecuteKindFinished);
    }
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
    setStep(&LanMatchRandomMatchmakeJob::WaitRandomMatchmake, "LanMatchRandomMatchmakeJob::WaitRandomMatchmake");
    return MakeExecuteResult(cExecuteKindWait, cWaitIntervalMsec);
}

LanMatchRandomMatchmakeJob::ExecuteResult LanMatchRandomMatchmakeJob::WaitRandomMatchmake()
{
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        m_callerContext->SignalFailure(cResultCancelled);
        m_callerContext = 0;
        Cleanup();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (m_matchmakeSession->WaitRandomMatchmakeSession(&m_sessionId, &m_isNewSession))
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            m_matchmakeSession->UnregisterSearchCriteria();
            m_callerContext->SignalFailure(m_callContext.m_result);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex] = m_sessionId;
            SetMatchmakeStatus(session::Session::cMatchmakeStatusSearching);
            if (m_isNewSession)
            {
                session::g_piaSession->SetCurrentSessionOpenStatus(
                    m_isOpenSession ? session::Session::cSessionOpenStatusOpen
                                    : session::Session::cSessionOpenStatusParticipationClosed);
            }
            setStep(&session::RandomMatchmakeJob::MeshStartup, "RandomMatchmakeJob::MeshStartup");
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchRandomMatchmakeJob::ExecuteResult LanMatchRandomMatchmakeJob::UnregisterGathering()
{
    if (session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedNetwork)
    {
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        setStep(&session::RandomMatchmakeJob::CompleteFailure,
                "nn::pia::lan::LanMatchRandomMatchmakeJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    session::Session* session = session::g_piaSession;
    const unsigned int index = session->m_currentSessionIndex;
    const Result result = session->m_matchmakeSession[index]->UnregisterMatchmakeSession(&m_callContext,
                                                                                         session->m_sessionId[index]);
    if (result != 0)
    {
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        m_callerContext->SignalFailure(m_result);
        m_callerContext = 0;
        Cleanup();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    setStep(&LanMatchRandomMatchmakeJob::WaitUnregisterGathering,
            "LanMatchRandomMatchmakeJob::WaitUnregisterGathering");
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchRandomMatchmakeJob::ExecuteResult LanMatchRandomMatchmakeJob::WaitUnregisterGathering()
{
    session::Session* session = session::g_piaSession;
    if (session->m_matchmakeSession[session->m_currentSessionIndex]->WaitUnregisterMatchmakeSession())
    {
        session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex] = 0;
        if (session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedNetwork)
            m_result = session::Session::cResultStatusDisconnectedNetwork;
        m_callerContext->SignalFailure(m_result);
        m_callerContext = 0;
        Cleanup();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchRandomMatchmakeJob::ExecuteResult LanMatchRandomMatchmakeJob::LeaveMatchmakeSession()
{
    if (session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedNetwork)
    {
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        setStep(&session::RandomMatchmakeJob::CompleteFailure,
                "nn::pia::lan::LanMatchRandomMatchmakeJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    session::Session* session = session::g_piaSession;
    const unsigned int index = session->m_currentSessionIndex;
    const Result result = session->m_matchmakeSession[index]->LeaveMatchmakeSession(&m_callContext,
                                                                                    session->m_sessionId[index]);
    if (result != 0)
    {
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        m_callerContext->SignalFailure(m_result);
        m_callerContext = 0;
        Cleanup();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    setStep(&LanMatchRandomMatchmakeJob::WaitLeaveMatchmakeSession,
            "LanMatchRandomMatchmakeJob::WaitLeaveMatchmakeSession");
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchRandomMatchmakeJob::ExecuteResult LanMatchRandomMatchmakeJob::WaitLeaveMatchmakeSession()
{
    session::Session* session = session::g_piaSession;
    if (session->m_matchmakeSession[session->m_currentSessionIndex]->WaitLeaveMatchmakeSession())
    {
        session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex] = 0;
        if (session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedNetwork)
            m_result = session::Session::cResultStatusDisconnectedNetwork;
        m_callerContext->SignalFailure(m_result);
        m_callerContext = 0;
        Cleanup();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchRandomMatchmakeJob::ExecuteResult LanMatchRandomMatchmakeJob::WaitForRetryRandomMatchmake()
{
    if (session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedNetwork)
    {
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        setStep(&session::RandomMatchmakeJob::CompleteFailure,
                "nn::pia::lan::LanMatchRandomMatchmakeJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    m_joinState = cJoinStateRetryWait;
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        m_result = cResultCancelled;
        m_matchmakeSession->UnregisterSearchCriteria();
        setStep(&LanMatchRandomMatchmakeJob::LeaveMatchmakeSession,
                "LanMatchRandomMatchmakeJob::LeaveMatchmakeSession");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_retryTime < common::g_piaJobScheduler->m_dispatchTime)
    {
        setStep(&LanMatchRandomMatchmakeJob::RandomMatchmake, "LanMatchRandomMatchmakeJob::RandomMatchmake");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchRandomMatchmakeJob::ExecuteResult LanMatchRandomMatchmakeJob::WaitRequestSessionInfo()
{
    if (!m_matchmakeSession->WaitRequestSessionInfo())
        return MakeExecuteResult(cExecuteKindNextStep);
    if (m_callContext.m_state == common::CallContext::cStateFailure)
    {
        if (session::g_piaMesh->GetStationNum() > cStationNumRetryThreshold)
            m_sessionInfoRetry = true;
        else if (session::g_piaMesh->GetStationNum() > 0)
        {
            cleanupForRetry();
            m_matchmakeSession->SetIgnoredSessionId(m_sessionId);
            setStep(&LanMatchRandomMatchmakeJob::RandomMatchmake, "LanMatchRandomMatchmakeJob::RandomMatchmake");
            return MakeExecuteResult(cExecuteKindContinue);
        }
        setStep(&session::RandomMatchmakeJob::LeaveMesh, "LanMatchRandomMatchmakeJob::LeaveMesh");
        m_result = cResultJoinFailed;
        return MakeExecuteResult(cExecuteKindContinue);
    }
    setStep(&session::RandomMatchmakeJob::CompleteProcess, "LanMatchRandomMatchmakeJob::CompleteProcess");
    return MakeExecuteResult(cExecuteKindContinue);
}

void LanMatchRandomMatchmakeJob::NoticeOwnerChangeEvent(unsigned int sessionId, unsigned long long ownerId)
{
    if (m_sessionId == sessionId && !m_isNewSession)
    {
        if (LanFacade::GetInstance()->GetLocalPrincipalId() == ownerId)
        {
            m_sessionInvalidated = true;
            return;
        }
        if (m_joinState == cJoinStateRetryWait)
        {
            const int waitMsec = session::g_piaMesh->GetMeshProtocol()->GetUnknown24() + cRetryMarginMsec;
            const common::TimeSpan span = common::TimeSpan::FromMSec(waitMsec);
            common::Time retryTime;
            retryTime.m_tick = common::g_piaJobScheduler->m_dispatchTime.m_tick + span.m_tick;
            m_retryTime = retryTime;
            return;
        }
        if (session::g_piaMesh->GetJoinMeshState() > 0)
        {
            m_ownerChanged = true;
            return;
        }
        m_sessionInvalidated = true;
        m_ownerChanged = true;
    }
}

void LanMatchRandomMatchmakeJob::Cleanup()
{
    if (m_matchmakeSession != 0)
        m_matchmakeSession->UnregisterSearchCriteria();
    session::RandomMatchmakeJob::Cleanup();
}

void LanMatchRandomMatchmakeJob::Trace(unsigned long long) const
{
}

Result LanMatchRandomMatchmakeJob::StartupImpl(const session::JoinRandomSessionSetting* setting)
{
    const session::CreateSessionSetting* createSetting = setting->GetCreateSessionSetting();
    session::Session* session = session::g_piaSession;
    m_matchmakeSession = static_cast<LanMatchmakeSession*>(session->m_matchmakeSession[session->m_currentSessionIndex]);
    if (m_matchmakeSession == 0)
        return cResultInvalidState;
    if (m_matchmakeSession->RegisterSearchCriteria(setting->GetSessionSearchCriteria(0),
                                                   setting->GetSessionSearchCriteriaSize())
        == 0)
        return cResultInvalidArgument;
    if (createSetting->GetMaxParticipantNum() == 0)
        return cResultInvalidArgument;
    m_matchmakeSession->ReflectSettingToMatchmakeSession(createSetting);
    session::g_piaSession->m_fieldc2[session::g_piaSession->m_currentSessionIndex] =
        createSetting->GetMaxParticipantNum();
    m_isOpenSession = static_cast<const LanCreateSessionSetting*>(createSetting)->IsOpenSession();
    setStep(&LanMatchRandomMatchmakeJob::RandomMatchmake, "LanMatchRandomMatchmakeJob::RandomMatchmake");
    return 0;
}

void LanMatchRandomMatchmakeJob::SetNextStepAtWaitCreateMesh()
{
    if (session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedNetwork)
    {
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, "LanMatchRandomMatchmakeJob::CompleteFailure");
    }
    else
    {
        m_matchmakeSession->UpdateStationInfoList();
        m_matchmakeSession->CopySessionInfo();
        setStep(&session::RandomMatchmakeJob::CompleteProcess, "RandomMatchmakeJob::CompleteProcess");
    }
}

void LanMatchRandomMatchmakeJob::SetNextStepAtWaitJoinMesh()
{
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
    {
        m_result = cResultJoinFailed;
        setStep(&session::RandomMatchmakeJob::LeaveMesh, "LanMatchRandomMatchmakeJob::LeaveMesh");
    }
    else if (session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedNetwork)
    {
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        setStep(&session::RandomMatchmakeJob::CompleteFailure, "LanMatchRandomMatchmakeJob::CompleteFailure");
    }
    else
    {
        const Result result = m_matchmakeSession->RequestSessionInfo(&m_callContext, m_sessionId);
        if (result == 0)
        {
            setStep(&LanMatchRandomMatchmakeJob::WaitRequestSessionInfo,
                    "LanMatchRandomMatchmakeJob::WaitRequestSessionInfo");
        }
        else
        {
            m_result = cResultJoinFailed;
            setStep(&session::RandomMatchmakeJob::LeaveMesh, "LanMatchRandomMatchmakeJob::LeaveMesh");
        }
    }
}

LanMatchRandomMatchmakeJob::ExecuteResult LanMatchRandomMatchmakeJob::SetNextStepAtFailedCreateMesh(Result result)
{
    if (session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedNetwork)
    {
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        setStep(&session::RandomMatchmakeJob::CompleteFailure,
                "nn::pia::lan::LanMatchRandomMatchmakeJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    m_result = result;
    m_matchmakeSession->UnregisterSearchCriteria();
    setStep(&LanMatchRandomMatchmakeJob::UnregisterGathering, "LanMatchRandomMatchmakeJob::UnregisterGathering");
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchRandomMatchmakeJob::ExecuteResult LanMatchRandomMatchmakeJob::SetNextStepAtFailedJoinMesh(Result result)
{
    if (session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedNetwork)
    {
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        setStep(&session::RandomMatchmakeJob::CompleteFailure,
                "nn::pia::lan::LanMatchRandomMatchmakeJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    m_result = result;
    if (result != cResultCancelled)
    {
        ++m_retryCount;
        if (m_retryCount < cRetryCountMax)
        {
            cleanupForRetry();
            if (result == cResultSessionIgnored)
            {
                m_matchmakeSession->SetIgnoredSessionId(m_sessionId);
                setStep(&LanMatchRandomMatchmakeJob::RandomMatchmake, "LanMatchRandomMatchmakeJob::RandomMatchmake");
                return MakeExecuteResult(cExecuteKindNextStep);
            }
            if (m_retryCount == 1)
            {
                m_retryTime = makeRetryTime();
                m_matchmakeSession->SetPreferredSessionId(m_sessionId);
                setStep(&LanMatchRandomMatchmakeJob::WaitForRetryRandomMatchmake,
                        "LanMatchRandomMatchmakeJob::WaitForRetryRandomMatchmake");
                return MakeExecuteResult(cExecuteKindNextStep);
            }
            m_matchmakeSession->SetIgnoredSessionId(m_sessionId);
            setStep(&LanMatchRandomMatchmakeJob::RandomMatchmake, "LanMatchRandomMatchmakeJob::RandomMatchmake");
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        if (m_result == cResultSessionIgnored)
            m_result = cResultSessionIgnoredRetryOver;
    }
    m_matchmakeSession->UnregisterSearchCriteria();
    setStep(&LanMatchRandomMatchmakeJob::LeaveMatchmakeSession, "LanMatchRandomMatchmakeJob::LeaveMatchmakeSession");
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchRandomMatchmakeJob::ExecuteResult LanMatchRandomMatchmakeJob::SetNextStepAtWaitLeaveMesh()
{
    if (session::g_piaSession->GetStatus() == session::Session::cStatusDisconnectedNetwork)
    {
        m_result = session::Session::cResultStatusDisconnectedNetwork;
        setStep(&session::RandomMatchmakeJob::CompleteFailure,
                "nn::pia::lan::LanMatchRandomMatchmakeJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_result != cResultCancelled && (m_sessionInfoRetry || m_retryReasonB || m_ownerChanged))
    {
        ++m_retryCount;
        if (m_retryCount < cRetryCountMax)
        {
            cleanupForRetry();
            if (m_retryCount == 1)
            {
                m_retryTime = makeRetryTime();
                m_matchmakeSession->SetPreferredSessionId(m_sessionId);
                setStep(&LanMatchRandomMatchmakeJob::WaitForRetryRandomMatchmake,
                        "LanMatchRandomMatchmakeJob::WaitForRetryRandomMatchmake");
                return MakeExecuteResult(cExecuteKindNextStep);
            }
            m_matchmakeSession->SetIgnoredSessionId(m_sessionId);
            setStep(&LanMatchRandomMatchmakeJob::RandomMatchmake, "LanMatchRandomMatchmakeJob::RandomMatchmake");
            return MakeExecuteResult(cExecuteKindNextStep);
        }
    }
    m_matchmakeSession->UnregisterSearchCriteria();
    setStep(&LanMatchRandomMatchmakeJob::LeaveMatchmakeSession, "LanMatchRandomMatchmakeJob::LeaveMatchmakeSession");
    return MakeExecuteResult(cExecuteKindNextStep);
}

}
}
}
