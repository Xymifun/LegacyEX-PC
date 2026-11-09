#include "nn/pia/session/JoinSessionJob.hpp"

#include "nn/pia/StationIdTable.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/common/SessionStateMonitoringContent.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/session/JoinSessionSetting.hpp"
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
enum
{
    cResultSessionDisconnected = 0xc406,
    cResultNetworkDisconnected = 0xa471,
    cResultSessionFull = 0xc420,
    cResultSessionClosed = 0xc421,
    cResultJoinFailed = 0x6439,
    cResultCancelled = 0x6c05,
    cResultInvalidArgument = 0x10c07,
    cResultInvalidState = 0x10c08
};

enum
{
    cStatusDisconnectedSession = 5,
    cStatusDisconnectedNetwork = 6,
    cStatusSessionFull = 7,
    cStatusSessionClosed = 8,
    cJoinMeshStateCompleted = 3,
    cJoinStateMeshJoining = 4,
    cJoinStateFinished = 9
};

const unsigned char cMonitoringUnset = 0xff;
const unsigned int cMonitoringSessionIdUnset = 0xffffffff;

inline void setStep(common::StepSequenceJob::Step& slot, JoinSessionJob::ExecuteResult (JoinSessionJob::*function)(),
                    const char* name)
{
    const common::StepSequenceJob::Step step(
        static_cast<JoinSessionJob::ExecuteResult (common::StepSequenceJob::*)()>(function), name);
    slot = step;
}
}

JoinSessionJob::JoinSessionJob()
    : common::StepSequenceJob(),
      m_callerContext(0),
      m_callContext(),
      m_stationConnectionInfo(),
      m_joinState(0),
      m_sessionId(0),
      m_joined(false),
      m_result(0),
      m_startTime()
{
}

JoinSessionJob::~JoinSessionJob()
{
}

void JoinSessionJob::Trace(unsigned long long value) const
{
}

Result JoinSessionJob::Startup(common::CallContext* callerContext, const JoinSessionSetting* setting)
{
    if (callerContext == 0 || setting == 0)
        return cResultInvalidArgument;
    const Result result = StartupImpl(setting);
    if (result != 0)
        return result;
    m_callerContext = callerContext;
    callerContext->InitiateCall();
    m_connectionInfoRetry = false;
    m_sessionInvalidated = false;
    m_sessionId = setting->GetSessionId();
    m_retryCount = 0;
    m_result = 0;
    Reset(true);
    m_startTime = common::g_piaJobScheduler->m_dispatchTime;
    common::SessionBeginMonitoringContent& monitoring = common::g_sessionBeginMonitoringContent;
    monitoring.m_joinRetryCount = cMonitoringUnset;
    monitoring.m_retryReasonACount = cMonitoringUnset;
    monitoring.m_connectionInfoRetryCount = cMonitoringUnset;
    monitoring.m_ownerChangeRetryCount = cMonitoringUnset;
    monitoring.m_sessionInfoRetryCount = cMonitoringUnset;
    monitoring.m_retryReasonBCount = cMonitoringUnset;
    return 0;
}

void JoinSessionJob::Cleanup()
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
    m_retryReasonB = false;
    m_sessionId = 0;
    m_result = 0;
    m_connectionInfoRetry = false;
    m_sessionInfoRetry = false;
    m_joined = false;
    m_retryReasonA = false;
    m_joinState = 0;
    m_sessionInvalidated = false;
    m_ownerChanged = false;
    transport::StationConnectionInfo connectionInfo;
    m_stationConnectionInfo.SetStationConnectionInfo(connectionInfo);
}

void JoinSessionJob::CleanupInternalContexts()
{
    m_callContext.SignalCancel();
    m_callContext.Reset();
}

bool JoinSessionJob::CheckContextCallCancelled()
{
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
        return true;
    return false;
}

unsigned int JoinSessionJob::GetJoinSessionStateForDebug() const
{
    return m_joinState;
}

void JoinSessionJob::NoticeOwnerChangeEvent(unsigned int, unsigned long long)
{
}

void JoinSessionJob::NoticeUnregisterEvent(unsigned int)
{
}

void JoinSessionJob::CleanupImpl()
{
}

void JoinSessionJob::SetMatchmakeStatus(Session::MatchmakeStatus status)
{
    g_piaSession->m_matchmakeStatus = status;
}

void JoinSessionJob::SetConnectionStatus(Session::ConnectionStatus status)
{
    g_piaSession->m_disconnectStatus = status;
}

void JoinSessionJob::SetNextStepAtWaitJoinMesh()
{
    setStep(m_currentStep, &JoinSessionJob::CompleteProcess, "JoinSessionJob::CompleteProcess");
}

JoinSessionJob::ExecuteResult JoinSessionJob::MeshStartup()
{
    const unsigned int status = g_piaSession->GetStatus();
    switch (status)
    {
    case cStatusDisconnectedSession:
        setStep(m_currentStep, &JoinSessionJob::CompleteFailure, "nn::pia::session::JoinSessionJob::CompleteFailure");
        m_result = cResultSessionDisconnected;
        return MakeExecuteResult(cExecuteKindContinue);
    case cStatusDisconnectedNetwork:
        setStep(m_currentStep, &JoinSessionJob::CompleteFailure, "nn::pia::session::JoinSessionJob::CompleteFailure");
        m_result = cResultNetworkDisconnected;
        return MakeExecuteResult(cExecuteKindContinue);
    case cStatusSessionFull:
        setStep(m_currentStep, &JoinSessionJob::CompleteFailure, "nn::pia::session::JoinSessionJob::CompleteFailure");
        m_result = cResultSessionFull;
        return MakeExecuteResult(cExecuteKindContinue);
    case cStatusSessionClosed:
        setStep(m_currentStep, &JoinSessionJob::CompleteFailure, "nn::pia::session::JoinSessionJob::CompleteFailure");
        m_result = cResultSessionClosed;
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    Session* session = g_piaSession;
    if (m_sessionInvalidated)
        return SetNextStepAtFailedJoinMesh(cResultJoinFailed);
    m_ownerChanged = false;
    m_retryReasonB = false;
    m_retryReasonA = false;
    m_sessionInfoRetry = false;
    const Result result = session->m_meshLayerController->MeshStartup(
        session->m_matchmakeSession[session->m_currentSessionIndex], false);
    if (result != 0)
        return SetNextStepAtFailedJoinMesh(result);
    session->m_postponeNotice = true;
    setStep(m_currentStep, &JoinSessionJob::JoinMesh, "JoinSessionJob::JoinMesh");
    return MakeExecuteResult(cExecuteKindContinue);
}

JoinSessionJob::ExecuteResult JoinSessionJob::JoinMesh()
{
    if (CheckContextCallCancelled())
    {
        SetNextStepAtFailedJoinMesh(cResultCancelled);
        return MakeExecuteResult(cExecuteKindContinue);
    }
    const Result result = g_piaMesh->JoinMeshAsync(m_stationConnectionInfo);
    if (result != 0)
    {
        common::g_sessionBeginMonitoringContent.m_joinMeshState = g_piaSession->GetJoinMeshState();
        g_piaSession->m_meshLayerController->MeshCleanup();
        return SetNextStepAtFailedJoinMesh(result);
    }
    m_joinState = cJoinStateMeshJoining;
    g_piaMesh->SetSkipReport(true);
    setStep(m_currentStep, &JoinSessionJob::WaitJoinMesh, "JoinSessionJob::WaitJoinMesh");
    return MakeExecuteResult(cExecuteKindNextStep);
}

JoinSessionJob::ExecuteResult JoinSessionJob::WaitJoinMesh()
{
    const unsigned int status = g_piaSession->GetStatus();
    switch (status)
    {
    case cStatusDisconnectedSession:
        setStep(m_currentStep, &JoinSessionJob::CompleteFailure, "nn::pia::session::JoinSessionJob::CompleteFailure");
        m_result = cResultSessionDisconnected;
        return MakeExecuteResult(cExecuteKindContinue);
    case cStatusDisconnectedNetwork:
        setStep(m_currentStep, &JoinSessionJob::CompleteFailure, "nn::pia::session::JoinSessionJob::CompleteFailure");
        m_result = cResultNetworkDisconnected;
        return MakeExecuteResult(cExecuteKindContinue);
    case cStatusSessionFull:
        setStep(m_currentStep, &JoinSessionJob::CompleteFailure, "nn::pia::session::JoinSessionJob::CompleteFailure");
        m_result = cResultSessionFull;
        return MakeExecuteResult(cExecuteKindContinue);
    case cStatusSessionClosed:
        setStep(m_currentStep, &JoinSessionJob::CompleteFailure, "nn::pia::session::JoinSessionJob::CompleteFailure");
        m_result = cResultSessionClosed;
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    if (g_piaMesh->IsJoinMeshCompleted())
    {
        Result joinResult = g_piaMesh->GetJoinMeshResult();
        if (joinResult != 0)
        {
            if (joinResult == cResultCancelled
                && (m_sessionInvalidated || m_ownerChanged || m_sessionInfoRetry || m_retryReasonB || m_retryReasonA))
                joinResult = cResultJoinFailed;
            return SetNextStepAtFailedJoinMesh(joinResult);
        }
        if (g_piaMesh->CheckConnectionError() != 0)
        {
            m_result = cResultJoinFailed;
            return SetNextStepAtFailedJoinMesh(cResultJoinFailed);
        }
        SetNextStepAtWaitJoinMesh();
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_sessionInvalidated || m_ownerChanged || m_sessionInfoRetry || m_retryReasonB || m_retryReasonA
        || CheckContextCallCancelled())
    {
        g_piaMesh->CancelJoinMeshAsync();
    }
    else if (g_piaMesh->IsHostMigrationEnabled()
             && (g_piaMesh->IsHostMigrationStarted() || g_piaMesh->IsProcessingHostMigration())
             && g_piaMesh->GetJoinMeshState() != cJoinMeshStateCompleted)
    {
        m_ownerChanged = true;
        g_piaMesh->CancelJoinMeshAsync();
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

JoinSessionJob::ExecuteResult JoinSessionJob::LeaveMesh()
{
    Result result;
    if (g_piaMesh->IsHost())
    {
        if (!g_piaMesh->IsHostMigrationEnabled())
        {
            result = g_piaMesh->DestroyMeshAsync();
            setStep(m_currentStep, &JoinSessionJob::WaitDestroyMesh, "JoinSessionJob::WaitDestroyMesh");
        }
        else
        {
            result = g_piaMesh->LeaveMeshWithHostMigrationAsync();
            setStep(m_currentStep, &JoinSessionJob::WaitLeaveMeshWithHostMigration,
                    "JoinSessionJob::WaitLeaveMeshWithHostMigration");
        }
    }
    else
    {
        result = g_piaMesh->LeaveMeshAsync();
        setStep(m_currentStep, &JoinSessionJob::WaitLeaveMesh, "JoinSessionJob::WaitLeaveMesh");
    }
    if (result != 0)
        return SetNextStepAtWaitLeaveMesh();
    return MakeExecuteResult(cExecuteKindNextStep);
}

JoinSessionJob::ExecuteResult JoinSessionJob::WaitLeaveMesh()
{
    if (g_piaMesh->IsLeaveMeshCompleted())
    {
        const Result result = g_piaMesh->GetLeaveMeshResult();
        return SetNextStepAtWaitLeaveMesh();
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

JoinSessionJob::ExecuteResult JoinSessionJob::WaitLeaveMeshWithHostMigration()
{
    if (g_piaMesh->IsLeaveMeshWithHostMigrationCompleted())
    {
        g_piaMesh->GetLeaveMeshWithHostMigrationResult();
        return SetNextStepAtWaitLeaveMesh();
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

JoinSessionJob::ExecuteResult JoinSessionJob::WaitDestroyMesh()
{
    if (g_piaMesh->IsDestroyMeshCompleted())
    {
        g_piaMesh->GetDestroyMeshResult();
        return SetNextStepAtWaitLeaveMesh();
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

JoinSessionJob::ExecuteResult JoinSessionJob::MeshCleanup()
{
    g_piaSession->m_meshLayerController->MeshCleanup();
    return SetNextStepAtMeshCleanup();
}

void JoinSessionJob::CalcProcessingTimeOfJoin() const
{
    const long long elapsed = static_cast<long long>(common::g_piaJobScheduler->m_dispatchTime.m_tick
                                                     - m_startTime.m_tick);
    const int processingTime = static_cast<int>(elapsed / common::TimeSpan::GetTicksPerMSec().m_tick);
    if (processingTime >= 0)
        common::g_sessionBeginMonitoringContent.m_joinProcessingTime = processingTime;
}

JoinSessionJob::ExecuteResult JoinSessionJob::CompleteProcess()
{
    if (CheckContextCallCancelled())
    {
        setStep(m_currentStep, &JoinSessionJob::LeaveMesh, "JoinSessionJob::LeaveMesh");
        m_result = cResultCancelled;
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_sessionInvalidated || m_ownerChanged || m_sessionInfoRetry
        || (g_piaMesh->IsHostMigrationEnabled()
            && (g_piaMesh->IsHostMigrationStarted() || g_piaMesh->IsProcessingHostMigration())))
    {
        setStep(m_currentStep, &JoinSessionJob::LeaveMesh, "JoinSessionJob::LeaveMesh");
        m_result = cResultJoinFailed;
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (g_piaMesh->GetMeshState() == Mesh::cStateNone)
    {
        m_result = cResultJoinFailed;
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
        common::g_sessionBeginMonitoringContent.m_joinMeshState = g_piaSession->GetJoinMeshState();
        setStep(m_currentStep, &JoinSessionJob::LeaveMesh, "JoinSessionJob::LeaveMesh");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    session->m_postponeNotice = false;
    session->m_stationIdStatusTable->ClearDisconnectedStationId();
    session->m_stationIdStatusTable->CallJoinEventNotYetNotified();
    m_callerContext->SignalSuccess(0);
    m_callerContext = 0;
    CalcProcessingTimeOfJoin();
    common::g_sessionBeginMonitoringContent.m_joinResult = 0;
    common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = cMonitoringSessionIdUnset;
    common::g_sessionBeginMonitoringContent.m_joinMeshState = cMonitoringUnset;
    m_joinState = cJoinStateFinished;
    g_piaMesh->SetSkipReport(false);
    g_piaMesh->SendReport(0);
    return MakeExecuteResult(cExecuteKindFinished);
}

JoinSessionJob::ExecuteResult JoinSessionJob::CompleteFailure()
{
    g_piaSession->m_meshLayerController->MeshCleanup();
    if (!CheckContextCallCancelled() && m_result != cResultCancelled)
    {
        if (m_result == cResultInvalidState || m_result == cResultSessionDisconnected)
        {
            g_piaSession->ChangeStatusToDisconnectedNetwork();
            m_callerContext->SignalFailure(cResultSessionDisconnected);
            m_result = cResultSessionDisconnected;
            m_callerContext = 0;
        }
        else if (m_result == cResultNetworkDisconnected)
        {
            g_piaSession->ChangeStatusToDisconnectedNetwork();
            m_callerContext->SignalFailure(m_result);
            m_callerContext = 0;
        }
        else
        {
            m_callerContext->SignalFailure(m_result);
            m_callerContext = 0;
        }
    }
    else
    {
        m_callerContext->SignalCancel();
        m_result = cResultCancelled;
        m_callerContext = 0;
    }
    CalcProcessingTimeOfJoin();
    common::g_sessionBeginMonitoringContent.m_joinSessionState = g_piaSession->GetJoinSessionStateForDebug();
    common::g_sessionBeginMonitoringContent.m_joinMeshState = g_piaSession->GetJoinMeshState();
    common::g_sessionBeginMonitoringContent.m_joinResult = m_result;
    m_joinState = cJoinStateFinished;
    g_piaMesh->SetSkipReport(false);
    g_piaMesh->SendReport(0);
    common::g_sessionBeginMonitoringContent.Cleanup();
    common::g_sessionStateMonitoringContent.Cleanup();
    Cleanup();
    return MakeExecuteResult(cExecuteKindFinished);
}

}
}
}
