#include "nn/pia/session/RandomMatchmakeJob.hpp"

#include "nn/pia/Transport.hpp"
#include "nn/pia/StationIdTable.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/common/SessionStateMonitoringContent.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/MeshLayerController.hpp"
#include "nn/pia/session/StationIdStatusTable.hpp"

namespace nn
{
namespace pia
{
namespace session
{

namespace
{
enum RandomMatchmakeResult
{
    cResultSessionDisconnected = 0xc406,
    cResultSessionStatus6 = 0xa471,
    cResultSessionStatus7 = 0xc420,
    cResultSessionStatus8 = 0xc421,
    cResultJoinFailed = 0x6439,
    cResultCancelled = 0x6c05,
    cResultCreateFailed = 0x10c08,
    cResultInvalidArgument = 0x10c07
};

enum SessionStatus
{
    cSessionStatusDisconnected = 5,
    cSessionStatus6 = 6,
    cSessionStatus7 = 7,
    cSessionStatus8 = 8
};

enum
{
    cJoinStateWaitJoinMesh = 4,
    cJoinStateComplete = 9,
    cJoinMeshStateUnknown3 = 3,
    cMonitoringByteUnset = 0xff
};

inline void SetCompleteFailureStep(RandomMatchmakeJob& job, int result)
{
    job.setStep(&RandomMatchmakeJob::CompleteFailure, "nn::pia::session::RandomMatchmakeJob::CompleteFailure");
    job.m_result = result;
}
}

RandomMatchmakeJob::RandomMatchmakeJob()
    : common::StepSequenceJob(),
      m_callerContext(0),
      m_joinState(0),
      m_sessionId(0),
      m_isNewSession(false),
      m_joined(false),
      m_result(0),
      m_stationConnectionInfo(),
      m_callContext(),
      m_startTime()
{
}

RandomMatchmakeJob::~RandomMatchmakeJob()
{
}

Result RandomMatchmakeJob::Startup(common::CallContext* callerContext, const JoinRandomSessionSetting* setting)
{
    if (callerContext == 0 || setting == 0)
        return ResultInvalidArgument();
    if (m_callerContext != 0)
        return ResultInvalidArgument();

    const Result result = StartupImpl(setting);
    if (result != 0)
        return result;

    m_callerContext = callerContext;
    callerContext->InitiateCall();
    m_sessionInvalidated = false;
    m_ownerChanged = false;
    m_retryCount = 0;
    m_connectionInfoRetry = false;
    m_result = 0;
    Reset(true);

    common::SessionBeginMonitoringContent& monitoring = common::g_sessionBeginMonitoringContent;
    monitoring.m_connectionInfoRetryCount = cMonitoringByteUnset;
    monitoring.m_joinRetryCount = cMonitoringByteUnset;
    monitoring.m_retryReasonBCount = cMonitoringByteUnset;
    monitoring.m_ownerChangeRetryCount = cMonitoringByteUnset;
    monitoring.m_sessionInfoRetryCount = cMonitoringByteUnset;
    monitoring.m_retryReasonACount = cMonitoringByteUnset;
    m_startTime = common::g_piaJobScheduler->m_dispatchTime;
    return 0;
}

RandomMatchmakeJob::ExecuteResult RandomMatchmakeJob::MeshStartup()
{
    const unsigned int status = g_piaSession->GetStatus();
    switch (status)
    {
    case cSessionStatusDisconnected:
        SetCompleteFailureStep(*this, cResultSessionDisconnected);
        return MakeExecuteResult(cExecuteKindContinue);
    case cSessionStatus6:
        SetCompleteFailureStep(*this, cResultSessionStatus6);
        return MakeExecuteResult(cExecuteKindContinue);
    case cSessionStatus7:
        SetCompleteFailureStep(*this, cResultSessionStatus7);
        return MakeExecuteResult(cExecuteKindContinue);
    case cSessionStatus8:
        SetCompleteFailureStep(*this, cResultSessionStatus8);
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }

    Session* session = g_piaSession;
    if (m_sessionInvalidated)
    {
        const bool createMesh = m_isNewSession;
        session->m_meshLayerController->MeshCleanup();
        if (createMesh)
            return SetNextStepAtFailedCreateMesh(cResultJoinFailed);
        return SetNextStepAtFailedJoinMesh(cResultJoinFailed);
    }
    if (m_ownerChanged && m_retryCount == 0)
    {
        session->m_meshLayerController->MeshCleanup();
        return SetNextStepAtFailedJoinMesh(cResultJoinFailed);
    }

    m_retryReasonA = false;
    m_retryReasonB = false;
    m_sessionInfoRetry = false;
    m_ownerChanged = false;
    const Result result = session->m_meshLayerController->MeshStartup(
        session->m_matchmakeSession[session->m_currentSessionIndex], false);
    if (result != 0)
    {
        const bool createMesh = m_isNewSession;
        session->m_meshLayerController->MeshCleanup();
        if (createMesh)
            return SetNextStepAtFailedCreateMesh(result);
        return SetNextStepAtFailedJoinMesh(result);
    }

    session->m_postponeNotice = true;
    SetNextStepAtMeshStartup();
    return MakeExecuteResult(cExecuteKindContinue);
}

RandomMatchmakeJob::ExecuteResult RandomMatchmakeJob::CreateMesh()
{
    const Result result = g_piaMesh->CreateMeshAsync();
    if (result != 0)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        return SetNextStepAtFailedCreateMesh(result);
    }
    g_piaMesh->SetSkipReport(true);
    setStep(&RandomMatchmakeJob::WaitCreateMesh, "RandomMatchmakeJob::WaitCreateMesh");
    return MakeExecuteResult(cExecuteKindNextStep);
}

RandomMatchmakeJob::ExecuteResult RandomMatchmakeJob::WaitCreateMesh()
{
    const unsigned int status = g_piaSession->GetStatus();
    switch (status)
    {
    case cSessionStatusDisconnected:
        SetCompleteFailureStep(*this, cResultSessionDisconnected);
        return MakeExecuteResult(cExecuteKindContinue);
    case cSessionStatus6:
        SetCompleteFailureStep(*this, cResultSessionStatus6);
        return MakeExecuteResult(cExecuteKindContinue);
    case cSessionStatus7:
        SetCompleteFailureStep(*this, cResultSessionStatus7);
        return MakeExecuteResult(cExecuteKindContinue);
    case cSessionStatus8:
        SetCompleteFailureStep(*this, cResultSessionStatus8);
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }

    const bool completed = g_piaMesh->IsCreateMeshCompleted();
    if (!completed)
        return MakeExecuteResult(cExecuteKindNextStep);

    Session* session = g_piaSession;
    const Result result = g_piaMesh->GetCreateMeshResult();
    if (result != 0)
    {
        session->m_meshLayerController->MeshCleanup();
        return SetNextStepAtFailedCreateMesh(result);
    }
    session->m_matchmakeStatus = Session::cMatchmakeStatusMatched;
    session->m_disconnectStatus = Session::cConnectionStatusConnected;
    if (!session->UpdateStationIdToBecomeHost())
    {
        session->m_meshLayerController->MeshCleanup();
        return SetNextStepAtFailedCreateMesh(cResultCreateFailed);
    }
    session->m_postponeNotice = false;
    session->m_stationIdStatusTable->CallJoinEventNotYetNotified();
    SetNextStepAtWaitCreateMesh();
    return MakeExecuteResult(cExecuteKindContinue);
}

RandomMatchmakeJob::ExecuteResult RandomMatchmakeJob::GetStationConnection()
{
    const unsigned int status = g_piaSession->GetStatus();
    switch (status)
    {
    case cSessionStatusDisconnected:
        SetCompleteFailureStep(*this, cResultSessionDisconnected);
        return MakeExecuteResult(cExecuteKindContinue);
    case cSessionStatus6:
        SetCompleteFailureStep(*this, cResultSessionStatus6);
        return MakeExecuteResult(cExecuteKindContinue);
    case cSessionStatus7:
        SetCompleteFailureStep(*this, cResultSessionStatus7);
        return MakeExecuteResult(cExecuteKindContinue);
    case cSessionStatus8:
        SetCompleteFailureStep(*this, cResultSessionStatus8);
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }

    const unsigned int index = g_piaSession->m_currentSessionIndex;
    IMatchmakeSession* matchmakeSession = g_piaSession->m_matchmakeSession[index];
    const unsigned int sessionId = g_piaSession->m_sessionId[index];
    const transport::StationConnectionInfo emptyInfo;
    m_stationConnectionInfo.SetStationConnectionInfo(emptyInfo);
    const Result result = matchmakeSession->GetStationConnectionInfo(&m_callContext, sessionId);
    if (result != 0)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        return SetNextStepAtFailedJoinMesh(result);
    }
    setStep(&RandomMatchmakeJob::WaitGetStationConnection, "RandomMatchmakeJob::WaitGetStationConnection");
    return MakeExecuteResult(cExecuteKindNextStep);
}

RandomMatchmakeJob::ExecuteResult RandomMatchmakeJob::WaitGetStationConnection()
{
    const unsigned int status = g_piaSession->GetStatus();
    switch (status)
    {
    case cSessionStatusDisconnected:
        SetCompleteFailureStep(*this, cResultSessionDisconnected);
        return MakeExecuteResult(cExecuteKindContinue);
    case cSessionStatus6:
        SetCompleteFailureStep(*this, cResultSessionStatus6);
        return MakeExecuteResult(cExecuteKindContinue);
    case cSessionStatus7:
        SetCompleteFailureStep(*this, cResultSessionStatus7);
        return MakeExecuteResult(cExecuteKindContinue);
    case cSessionStatus8:
        SetCompleteFailureStep(*this, cResultSessionStatus8);
        return MakeExecuteResult(cExecuteKindContinue);
    default:
    {
        IMatchmakeSession* matchmakeSession = g_piaSession->m_matchmakeSession[g_piaSession->m_currentSessionIndex];
        if (matchmakeSession->WaitGetStationConnectionInfo(&m_stationConnectionInfo))
        {
            if (m_callContext.m_state == common::CallContext::cStateFailure)
            {
                g_piaSession->m_meshLayerController->MeshCleanup();
                return SetNextStepAtFailedJoinMesh(m_callContext.m_result);
            }
            if (m_callContext.m_state == common::CallContext::cStateSuccess)
            {
                m_principalId = m_stationConnectionInfo.m_stationLocation.m_principalId;
                setStep(&RandomMatchmakeJob::JoinMesh, "RandomMatchmakeJob::JoinMesh");
                return MakeExecuteResult(cExecuteKindContinue);
            }
        }
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    }
}

RandomMatchmakeJob::ExecuteResult RandomMatchmakeJob::JoinMesh()
{
    const unsigned int status = g_piaSession->GetStatus();
    switch (status)
    {
    case cSessionStatusDisconnected:
        SetCompleteFailureStep(*this, cResultSessionDisconnected);
        return MakeExecuteResult(cExecuteKindContinue);
    case cSessionStatus6:
        SetCompleteFailureStep(*this, cResultSessionStatus6);
        return MakeExecuteResult(cExecuteKindContinue);
    case cSessionStatus7:
        SetCompleteFailureStep(*this, cResultSessionStatus7);
        return MakeExecuteResult(cExecuteKindContinue);
    case cSessionStatus8:
        SetCompleteFailureStep(*this, cResultSessionStatus8);
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }

    const Result result = g_piaMesh->JoinMeshAsync(m_stationConnectionInfo);
    if (result != 0)
    {
        common::g_sessionBeginMonitoringContent.m_joinMeshState = static_cast<unsigned char>(
            g_piaSession->GetJoinMeshState());
        g_piaSession->m_meshLayerController->MeshCleanup();
        return SetNextStepAtFailedJoinMesh(result);
    }
    g_piaMesh->SetSkipReport(true);
    setStep(&RandomMatchmakeJob::WaitJoinMesh, "RandomMatchmakeJob::WaitJoinMesh");
    m_joinState = cJoinStateWaitJoinMesh;
    return MakeExecuteResult(cExecuteKindNextStep);
}

bool RandomMatchmakeJob::CheckContextCallCancelled()
{
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
        return true;
    return false;
}

RandomMatchmakeJob::ExecuteResult RandomMatchmakeJob::WaitJoinMesh()
{
    const unsigned int status = g_piaSession->GetStatus();
    switch (status)
    {
    case cSessionStatusDisconnected:
        SetCompleteFailureStep(*this, cResultSessionDisconnected);
        return MakeExecuteResult(cExecuteKindContinue);
    case cSessionStatus6:
        SetCompleteFailureStep(*this, cResultSessionStatus6);
        return MakeExecuteResult(cExecuteKindContinue);
    case cSessionStatus7:
        SetCompleteFailureStep(*this, cResultSessionStatus7);
        return MakeExecuteResult(cExecuteKindContinue);
    case cSessionStatus8:
        SetCompleteFailureStep(*this, cResultSessionStatus8);
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }

    if (!g_piaMesh->IsJoinMeshCompleted())
    {
        if (!m_sessionInvalidated && !m_ownerChanged && !m_sessionInfoRetry && !m_retryReasonB && !m_retryReasonA
            && !CheckContextCallCancelled())
        {
            if (g_piaMesh->IsHostMigrationEnabled()
                && (g_piaMesh->IsHostMigrationStarted() || g_piaMesh->IsProcessingHostMigration())
                && g_piaMesh->GetJoinMeshState() != cJoinMeshStateUnknown3)
            {
                m_ownerChanged = true;
                g_piaMesh->CancelJoinMeshAsync();
            }
        }
        else
        {
            g_piaMesh->CancelJoinMeshAsync();
        }
        return MakeExecuteResult(cExecuteKindNextStep);
    }

    Result result = g_piaMesh->GetJoinMeshResult();
    if (result == 0)
    {
        const Result connectionError = g_piaMesh->CheckConnectionError();
        if (connectionError != 0)
        {
            m_result = cResultJoinFailed;
            g_piaSession->m_meshLayerController->MeshCleanup();
            return SetNextStepAtFailedJoinMesh(m_result);
        }
        SetNextStepAtWaitJoinMesh();
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (result == cResultCancelled
        && (m_sessionInvalidated || m_ownerChanged || m_sessionInfoRetry || m_retryReasonB || m_retryReasonA))
        result = cResultJoinFailed;
    g_piaSession->m_meshLayerController->MeshCleanup();
    return SetNextStepAtFailedJoinMesh(result);
}

RandomMatchmakeJob::ExecuteResult RandomMatchmakeJob::LeaveMesh()
{
    Result result;
    if (g_piaMesh->IsHost())
    {
        if (!g_piaMesh->IsHostMigrationEnabled())
            result = g_piaMesh->DestroyMesh(&m_callContext);
        else
            result = g_piaMesh->LeaveMeshWithHostMigration(&m_callContext);
    }
    else
    {
        result = g_piaMesh->LeaveMesh(&m_callContext);
    }
    if (result != 0)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        return SetNextStepAtWaitLeaveMesh();
    }
    setStep(&RandomMatchmakeJob::WaitLeaveMesh, "RandomMatchmakeJob::WaitLeaveMesh");
    return MakeExecuteResult(cExecuteKindNextStep);
}

RandomMatchmakeJob::ExecuteResult RandomMatchmakeJob::WaitLeaveMesh()
{
    if (m_callContext.m_state != common::CallContext::cStateInProgress)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        return SetNextStepAtWaitLeaveMesh();
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

void RandomMatchmakeJob::CalcProcessingTimeOfJoin() const
{
    const int elapsedMsec = static_cast<int>(
        static_cast<long long>(common::g_piaJobScheduler->m_dispatchTime.m_tick - m_startTime.m_tick)
        / common::TimeSpan::GetTicksPerMSec().m_tick);
    if (elapsedMsec >= 0)
        common::g_sessionBeginMonitoringContent.m_joinProcessingTime = elapsedMsec;
}

RandomMatchmakeJob::ExecuteResult RandomMatchmakeJob::CompleteProcess()
{
    if (CheckContextCallCancelled())
    {
        setStep(&RandomMatchmakeJob::LeaveMesh, "RandomMatchmakeJob::LeaveMesh");
        m_result = cResultCancelled;
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_sessionInvalidated || m_ownerChanged || m_sessionInfoRetry
        || (g_piaMesh->IsHostMigrationEnabled()
            && (g_piaMesh->IsHostMigrationStarted() || g_piaMesh->IsProcessingHostMigration())))
    {
        setStep(&RandomMatchmakeJob::LeaveMesh, "RandomMatchmakeJob::LeaveMesh");
        m_result = cResultJoinFailed;
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (g_piaMesh->GetMeshState() == 0)
    {
        m_result = cResultJoinFailed;
        g_piaSession->m_meshLayerController->MeshCleanup();
        return SetNextStepAtWaitLeaveMesh();
    }

    m_joined = true;
    Session* session = g_piaSession;
    session->m_matchmakeStatus = Session::cMatchmakeStatusMatched;
    transport::g_piaTransport->m_stationIdTable->SetAvailableEntryNum(
        session->m_fieldc2[session->m_currentSessionIndex]);
    session->m_disconnectStatus = Session::cConnectionStatusConnected;
    if (!session->UpdateStationIdToBecomeClient())
    {
        m_result = cResultJoinFailed;
        common::g_sessionBeginMonitoringContent.m_joinMeshState = static_cast<unsigned char>(
            session->GetJoinMeshState());
        setStep(&RandomMatchmakeJob::LeaveMesh, "RandomMatchmakeJob::LeaveMesh");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    session->m_postponeNotice = false;
    session->m_stationIdStatusTable->ClearDisconnectedStationId();
    session->m_stationIdStatusTable->CallJoinEventNotYetNotified();
    CalcProcessingTimeOfJoin();
    common::g_sessionBeginMonitoringContent.m_joinMeshState = cMonitoringByteUnset;
    if (m_isNewSession)
        common::g_sessionBeginMonitoringContent.m_joinResult = 0xffffffff;
    else
        common::g_sessionBeginMonitoringContent.m_joinResult = 0;
    common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = 0xffffffff;
    g_piaMesh->SetSkipReport(false);
    g_piaMesh->SendReport(0);
    m_joinState = cJoinStateComplete;
    m_callerContext->SignalSuccess(0);
    m_callerContext = 0;
    return MakeExecuteResult(cExecuteKindFinished);
}

RandomMatchmakeJob::ExecuteResult RandomMatchmakeJob::CompleteFailure()
{
    g_piaSession->m_meshLayerController->MeshCleanup();
    if (CheckContextCallCancelled() || m_result == cResultCancelled)
    {
        m_callerContext->SignalCancel();
        m_result = cResultCancelled;
    }
    else if (m_result == cResultCreateFailed || m_result == cResultSessionDisconnected)
    {
        g_piaSession->ChangeStatusToDisconnectedNetwork();
        m_callerContext->SignalFailure(cResultSessionDisconnected);
        m_result = cResultSessionDisconnected;
    }
    else
    {
        m_callerContext->SignalFailure(m_result);
    }
    m_callerContext = 0;
    CalcProcessingTimeOfJoin();
    common::g_sessionBeginMonitoringContent.m_metric130 = static_cast<unsigned char>(
        g_piaSession->GetJoinRandomSessionStateForDebug());
    common::g_sessionBeginMonitoringContent.m_joinMeshState = static_cast<unsigned char>(
        g_piaSession->GetJoinMeshState());
    common::g_sessionBeginMonitoringContent.m_joinResult = m_result;
    m_joinState = cJoinStateComplete;
    g_piaMesh->SetSkipReport(false);
    g_piaMesh->SendReport(0);
    common::g_sessionBeginMonitoringContent.Cleanup();
    common::g_sessionStateMonitoringContent.Cleanup();
    Cleanup();
    return MakeExecuteResult(cExecuteKindFinished);
}

void RandomMatchmakeJob::SetNextStepAtMeshStartup()
{
    if (m_isNewSession)
    {
        transport::g_piaTransport->m_stationIdTable->SetAvailableEntryNum(
            g_piaSession->m_fieldc2[g_piaSession->m_currentSessionIndex]);
        setStep(&RandomMatchmakeJob::CreateMesh, "RandomMatchmakeJob::CreateMesh");
    }
    else
    {
        setStep(&RandomMatchmakeJob::GetStationConnection, "RandomMatchmakeJob::GetStationConnection");
    }
}

void RandomMatchmakeJob::Cleanup()
{
    CleanupImpl();
    if (m_callerContext != 0)
    {
        if (m_callerContext->m_state == common::CallContext::cStateInProgress)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
    }
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        m_callContext.SignalCancel();
    m_callContext.Reset();
    m_joinState = 0;
    m_retryReasonA = false;
    m_principalId = 0;
    m_connectionInfoRetry = false;
    m_result = 0;
    m_joined = false;
    m_retryReasonB = false;
    m_sessionInvalidated = false;
    m_isNewSession = false;
    m_sessionId = 0;
    m_sessionInfoRetry = false;
    m_ownerChanged = false;
    const transport::StationConnectionInfo emptyInfo;
    m_stationConnectionInfo.SetStationConnectionInfo(emptyInfo);
}

void RandomMatchmakeJob::CleanupInternalContexts()
{
    m_callContext.SignalCancel();
    m_callContext.Reset();
}

void RandomMatchmakeJob::Trace(unsigned long long value) const
{
}

void RandomMatchmakeJob::SetMatchmakeStatus(Session::MatchmakeStatus status)
{
    g_piaSession->m_matchmakeStatus = status;
}

void RandomMatchmakeJob::SetConnectionStatus(Session::ConnectionStatus status)
{
    g_piaSession->m_disconnectStatus = status;
}

unsigned int RandomMatchmakeJob::GetJoinSessionStateForDebug() const
{
    return m_joinState;
}

void RandomMatchmakeJob::NoticeOwnerChangeEvent(unsigned int sessionId, unsigned long long ownerId)
{
}

void RandomMatchmakeJob::NoticeUnregisterEvent(unsigned int sessionId)
{
}

void RandomMatchmakeJob::CleanupImpl()
{
}

}
}
}
