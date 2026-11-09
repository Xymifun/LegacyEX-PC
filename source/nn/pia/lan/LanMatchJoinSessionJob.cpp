#include "nn/pia/lan/LanMatchJoinSessionJob.hpp"

#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/lan/LanFacade.hpp"
#include "nn/pia/lan/LanMatchmakeSession.hpp"
#include "nn/pia/lan/LanSessionInfo.hpp"
#include "nn/pia/session/JoinSessionSetting.hpp"
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
    cResultSessionDisconnected = 0xc406,
    cResultJoinFailed = 0x6439,
    cResultCancelled = 0x6c05,
    cResultUnknown6838 = 0x6838,
    cResultUnknown6c3d = 0x6c3d,
    cResultUnknown6c4c = 0x6c4c,
    cResultRemoteVersionOlder = 0x6470,
    cResultRemoteVersionNewer = 0x646f,
    cResultInvalidArgument = 0x10c07,
    cResultInvalidState = 0x10c08
};

enum
{
    cStatusDisconnectedSession = 5,
    cJoinStateMatchmakeJoining = 2,
    cJoinStateRetryWait = 8,
    cRetryCountMax = 2,
    cRetryMarginMsec = 9000,
    cSystemCommunicationVersionMin = 5,
    cStationNumRetryThreshold = 2
};

template <class T>
inline void setStep(common::StepSequenceJob::Step& slot, common::Job::ExecuteResult (T::*function)(), const char* name)
{
    const common::StepSequenceJob::Step step(
        static_cast<common::Job::ExecuteResult (common::StepSequenceJob::*)()>(function), name);
    slot = step;
}

inline common::Time makeRetryTime()
{
    const int waitMsec = session::g_piaMesh->GetMeshProtocol()->GetUnknown24() + cRetryMarginMsec;
    common::Time time;
    time.m_tick = common::g_piaJobScheduler->m_dispatchTime.m_tick
                  + common::TimeSpan::GetTicksPerMSec().m_tick * waitMsec;
    return time;
}
}

LanMatchJoinSessionJob::LanMatchJoinSessionJob() : session::JoinSessionJob(), m_retryTime(), m_startupResult(0)
{
}

LanMatchJoinSessionJob::~LanMatchJoinSessionJob()
{
}

LanMatchJoinSessionJob::ExecuteResult LanMatchJoinSessionJob::JoinMatchmakeSession()
{
    if (session::g_piaSession->GetStatus() == cStatusDisconnectedSession)
    {
        setStep(m_currentStep, &session::JoinSessionJob::CompleteFailure,
                "nn::pia::lan::LanMatchJoinSessionJob::CompleteFailure");
        m_result = cResultSessionDisconnected;
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        m_callerContext->SignalFailure(cResultCancelled);
        m_callerContext = 0;
        Cleanup();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    const Result startupResult = m_startupResult;
    if (startupResult != 0)
    {
        m_callerContext->SignalFailure(startupResult);
        m_callerContext = 0;
        Cleanup();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    const Result result = m_matchmakeSession->JoinMatchmakeSession(&m_callContext, m_sessionId);
    if (result != 0)
    {
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        Cleanup();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    m_joinState = cJoinStateMatchmakeJoining;
    setStep(m_currentStep, &LanMatchJoinSessionJob::WaitJoinMatchmake, "LanMatchJoinSessionJob::WaitJoinMatchmake");
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchJoinSessionJob::ExecuteResult LanMatchJoinSessionJob::WaitJoinMatchmake()
{
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        m_callerContext->SignalFailure(cResultCancelled);
        m_callerContext = 0;
        Cleanup();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (m_matchmakeSession->WaitJoinMatchmakeSession())
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            m_callerContext->SignalFailure(m_callContext.m_result);
            m_callerContext = 0;
            Cleanup();
            return MakeExecuteResult(cExecuteKindFinished);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex] = m_sessionId;
            SetMatchmakeStatus(session::Session::cMatchmakeStatusSearching);
            m_callContext.Reset();
            setStep(m_currentStep, &LanMatchJoinSessionJob::GetStationConnectionInfo,
                    "LanMatchJoinSessionJob::GetStationConnectionInfo");
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchJoinSessionJob::ExecuteResult LanMatchJoinSessionJob::GetStationConnectionInfo()
{
    if (session::g_piaSession->GetStatus() == cStatusDisconnectedSession)
    {
        setStep(m_currentStep, &session::JoinSessionJob::CompleteFailure,
                "nn::pia::lan::LanMatchJoinSessionJob::CompleteFailure");
        m_result = cResultSessionDisconnected;
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        m_result = cResultCancelled;
        setStep(m_currentStep, &LanMatchJoinSessionJob::LeaveMatchmakeSession,
                "LanMatchJoinSessionJob::LeaveMatchmakeSession");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    session::Session* session = session::g_piaSession;
    const Result result = m_matchmakeSession->GetStationConnectionInfo(
        &m_callContext, session->m_sessionId[session->m_currentSessionIndex]);
    if (result != 0)
    {
        setStep(m_currentStep, &LanMatchJoinSessionJob::LeaveMatchmakeSession,
                "LanMatchJoinSessionJob::LeaveMatchmakeSession");
        m_result = m_callContext.m_result;
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    setStep(m_currentStep, &LanMatchJoinSessionJob::WaitGetStationConnectionInfo,
            "LanMatchJoinSessionJob::WaitGetStationConnectionInfo");
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchJoinSessionJob::ExecuteResult LanMatchJoinSessionJob::WaitGetStationConnectionInfo()
{
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        setStep(m_currentStep, &LanMatchJoinSessionJob::LeaveMatchmakeSession,
                "LanMatchJoinSessionJob::LeaveMatchmakeSession");
        m_result = cResultCancelled;
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    if (m_matchmakeSession->WaitGetStationConnectionInfo(&m_stationConnectionInfo))
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            setStep(m_currentStep, &LanMatchJoinSessionJob::LeaveMatchmakeSession,
                    "LanMatchJoinSessionJob::LeaveMatchmakeSession");
            m_result = m_callContext.m_result;
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            setStep(m_currentStep, &session::JoinSessionJob::MeshStartup, "JoinSessionJob::MeshStartup");
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchJoinSessionJob::ExecuteResult LanMatchJoinSessionJob::LeaveMatchmakeSession()
{
    if (session::g_piaSession->GetStatus() == cStatusDisconnectedSession)
    {
        m_result = cResultSessionDisconnected;
        setStep(m_currentStep, &session::JoinSessionJob::CompleteFailure,
                "nn::pia::lan::LanMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    session::Session* session = session::g_piaSession;
    const Result result = m_matchmakeSession->LeaveMatchmakeSession(
        &m_callContext, session->m_sessionId[session->m_currentSessionIndex]);
    if (result == 0)
    {
        setStep(m_currentStep, &LanMatchJoinSessionJob::WaitLeaveMatchmakeSession,
                "LanMatchJoinSessionJob::WaitLeaveMatchmakeSession");
    }
    else
    {
        setStep(m_currentStep, &session::JoinSessionJob::CompleteFailure, "LanMatchJoinSessionJob::CompleteFailure");
        session->m_sessionId[session->m_currentSessionIndex] = 0;
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchJoinSessionJob::ExecuteResult LanMatchJoinSessionJob::WaitLeaveMatchmakeSession()
{
    if (m_matchmakeSession->WaitLeaveMatchmakeSession())
    {
        session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex] = 0;
        if (session::g_piaSession->GetStatus() == cStatusDisconnectedSession)
            m_result = cResultSessionDisconnected;
        setStep(m_currentStep, &session::JoinSessionJob::CompleteFailure, "LanMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchJoinSessionJob::ExecuteResult LanMatchJoinSessionJob::WaitForRetryJoinMatchmakeSession()
{
    if (session::g_piaSession->GetStatus() == cStatusDisconnectedSession)
    {
        setStep(m_currentStep, &session::JoinSessionJob::CompleteFailure,
                "nn::pia::lan::LanMatchJoinSessionJob::CompleteFailure");
        m_result = cResultSessionDisconnected;
        return MakeExecuteResult(cExecuteKindContinue);
    }
    m_joinState = cJoinStateRetryWait;
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        setStep(m_currentStep, &session::JoinSessionJob::MeshCleanup, "LanMatchJoinSessionJob::MeshCleanup");
        m_result = cResultCancelled;
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_retryTime < common::g_piaJobScheduler->m_dispatchTime)
    {
        transport::StationConnectionInfo connectionInfo;
        m_stationConnectionInfo.SetStationConnectionInfo(connectionInfo);
        m_result = 0;
        setStep(m_currentStep, &LanMatchJoinSessionJob::JoinMatchmakeSession,
                "LanMatchJoinSessionJob::JoinMatchmakeSession");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchJoinSessionJob::ExecuteResult LanMatchJoinSessionJob::WaitRequestSessionInfo()
{
    if (!m_matchmakeSession->WaitRequestSessionInfo())
        return MakeExecuteResult(cExecuteKindNextStep);
    if (m_callContext.m_state == common::CallContext::cStateFailure)
    {
        m_result = cResultJoinFailed;
        if (session::g_piaMesh->GetStationNum() > cStationNumRetryThreshold)
            m_sessionInfoRetry = true;
        setStep(m_currentStep, &session::JoinSessionJob::LeaveMesh, "LanMatchJoinSessionJob::LeaveMesh");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    setStep(m_currentStep, &session::JoinSessionJob::CompleteProcess, "LanMatchJoinSessionJob::CompleteProcess");
    return MakeExecuteResult(cExecuteKindContinue);
}

void LanMatchJoinSessionJob::Cleanup()
{
    session::JoinSessionJob::Cleanup();
    m_matchmakeSession = 0;
    m_startupResult = 0;
}

void LanMatchJoinSessionJob::NoticeOwnerChangeEvent(unsigned int sessionId, unsigned long long ownerId)
{
    if (m_sessionId == sessionId)
    {
        if (LanFacade::GetInstance()->GetLocalPrincipalId() == ownerId)
        {
            m_sessionInvalidated = true;
            return;
        }
        if (m_joinState == cJoinStateRetryWait)
        {
            m_retryTime = makeRetryTime();
            return;
        }
        if (session::g_piaMesh->GetJoinMeshState() > 0)
        {
            m_ownerChanged = true;
            return;
        }
        m_ownerChanged = true;
        m_sessionInvalidated = true;
    }
}

void LanMatchJoinSessionJob::Trace(unsigned long long) const
{
}

void LanMatchJoinSessionJob::SetNextStepAtWaitJoinMesh()
{
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
    {
        m_result = cResultJoinFailed;
        setStep(m_currentStep, &session::JoinSessionJob::LeaveMesh, "LanMatchJoinSessionJob::LeaveMesh");
    }
    else if (session::g_piaSession->GetStatus() == cStatusDisconnectedSession)
    {
        m_result = cResultSessionDisconnected;
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        setStep(m_currentStep, &session::JoinSessionJob::CompleteFailure,
                "lan::LanMatchJoinSessionJob::CompleteFailure");
    }
    else
    {
        const Result result = m_matchmakeSession->RequestSessionInfo(&m_callContext, m_sessionId);
        if (result != 0)
        {
            m_result = cResultJoinFailed;
            setStep(m_currentStep, &session::JoinSessionJob::LeaveMesh, "LanMatchJoinSessionJob::LeaveMesh");
        }
        else
        {
            setStep(m_currentStep, &LanMatchJoinSessionJob::WaitRequestSessionInfo,
                    "LanMatchJoinSessionJob::WaitRequestSessionInfo");
        }
    }
}

LanMatchJoinSessionJob::ExecuteResult LanMatchJoinSessionJob::SetNextStepAtFailedJoinMesh(Result result)
{
    if (session::g_piaSession->GetStatus() == cStatusDisconnectedSession)
    {
        m_result = cResultSessionDisconnected;
        setStep(m_currentStep, &session::JoinSessionJob::CompleteFailure,
                "nn::pia::lan::LanMatchJoinSessionJob::CompleteFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (result == cResultUnknown6838)
    {
        setStep(m_currentStep, &session::JoinSessionJob::MeshCleanup, "LanMatchJoinSessionJob::MeshCleanup");
        m_result = result;
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    if (result == cResultUnknown6c3d)
    {
        m_result = cResultUnknown6c4c;
        setStep(m_currentStep, &session::JoinSessionJob::MeshCleanup, "LanMatchJoinSessionJob::MeshCleanup");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    m_result = result;
    if (result != cResultCancelled)
    {
        m_retryCount = m_retryCount + 1;
        if (m_retryCount < cRetryCountMax)
        {
            session::g_piaSession->m_postponeNotice = false;
            session::g_piaSession->m_stationIdStatusTable->ClearDisconnectedStationId();
            session::g_piaSession->m_meshLayerController->MeshCleanup();
            m_retryTime = makeRetryTime();
            setStep(m_currentStep, &LanMatchJoinSessionJob::WaitForRetryJoinMatchmakeSession,
                    "LanMatchJoinSessionJob::WaitForRetryJoinMatchmakeSession");
            return MakeExecuteResult(cExecuteKindNextStep);
        }
    }
    setStep(m_currentStep, &session::JoinSessionJob::MeshCleanup, "LanMatchJoinSessionJob::MeshCleanup");
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchJoinSessionJob::ExecuteResult LanMatchJoinSessionJob::SetNextStepAtWaitLeaveMesh()
{
    if (session::g_piaSession->GetStatus() == cStatusDisconnectedSession)
    {
        setStep(m_currentStep, &session::JoinSessionJob::CompleteFailure,
                "nn::pia::lan::LanMatchJoinSessionJob::CompleteFailure");
        m_result = cResultSessionDisconnected;
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_result != cResultCancelled && (m_sessionInfoRetry || m_retryReasonB || m_ownerChanged))
    {
        m_retryCount = m_retryCount + 1;
        if (m_retryCount < cRetryCountMax)
        {
            session::g_piaSession->m_postponeNotice = false;
            session::g_piaSession->m_stationIdStatusTable->ClearDisconnectedStationId();
            session::g_piaSession->m_meshLayerController->MeshCleanup();
            m_retryTime = makeRetryTime();
            setStep(m_currentStep, &LanMatchJoinSessionJob::WaitForRetryJoinMatchmakeSession,
                    "LanMatchJoinSessionJob::WaitForRetryJoinMatchmakeSession");
            return MakeExecuteResult(cExecuteKindNextStep);
        }
    }
    setStep(m_currentStep, &session::JoinSessionJob::MeshCleanup, "LanMatchJoinSessionJob::MeshCleanup");
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchJoinSessionJob::ExecuteResult LanMatchJoinSessionJob::SetNextStepAtMeshCleanup()
{
    setStep(m_currentStep, &LanMatchJoinSessionJob::LeaveMatchmakeSession,
            "LanMatchJoinSessionJob::LeaveMatchmakeSession");
    return MakeExecuteResult(cExecuteKindContinue);
}

Result LanMatchJoinSessionJob::StartupImpl(const session::JoinSessionSetting* setting)
{
    session::Session* session = session::g_piaSession;
    m_matchmakeSession = static_cast<LanMatchmakeSession*>(session->m_matchmakeSession[session->m_currentSessionIndex]);
    if (m_matchmakeSession == 0)
        return cResultInvalidState;
    if (setting == 0)
        return cResultInvalidArgument;
    const LanSessionInfo* sessionInfo = static_cast<const LanSessionInfo*>(setting->GetSessionInfoPtr());
    if (sessionInfo->GetSystemCommunicationVersion() < cSystemCommunicationVersionMin)
    {
        m_startupResult = cResultRemoteVersionOlder;
    }
    else if (sessionInfo->GetSystemCommunicationVersion() > cSystemCommunicationVersionMin)
    {
        m_startupResult = cResultRemoteVersionNewer;
    }
    else if (sessionInfo->GetApplicationCommunicationVersion()
             < LanFacade::GetInstance()->GetApplicationCommunicationVersion())
    {
        m_startupResult = cResultRemoteVersionOlder;
    }
    else if (sessionInfo->GetApplicationCommunicationVersion()
             > LanFacade::GetInstance()->GetApplicationCommunicationVersion())
    {
        m_startupResult = cResultRemoteVersionNewer;
    }
    else
    {
        m_startupResult = 0;
    }
    m_matchmakeSession->ReflectSettingToMatchmakeSession(setting);
    m_sessionId = setting->GetSessionId();
    setStep(m_currentStep, &LanMatchJoinSessionJob::JoinMatchmakeSession,
            "LanMatchJoinSessionJob::JoinMatchmakeSession");
    return 0;
}

}
}
}
