#include "nn/pia/session/LeaveSessionJob.hpp"

#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/SessionStateMonitoringContent.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/JointSessionJob.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/MeshLayerController.hpp"
#include "nn/pia/session/Session.hpp"
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
    cResultCancelled = 0x6c05,
    cResultInvalidState = 0x10c08
};

enum
{
    cReportReasonLeave = 1,
    cSendMonitoringDataWait = 50
};

enum
{
    cMatchmakeStatusMatched = 2,
    cMatchmakeStatusJointSessionProcessing = 3,
    cMatchmakeStatusJointSessionProcessed = 4
};

enum
{
    cHostMigrationRemainingStationNum = 2
};
}

LeaveSessionJob::LeaveSessionJob()
    : common::StepSequenceJob(),
      m_callerContext(0),
      m_callContext(),
      m_hostMigrationTimeoutTime(),
      m_startTime()
{
}

LeaveSessionJob::~LeaveSessionJob()
{
}

Result LeaveSessionJob::Startup(common::CallContext* callerContext)
{
    if (m_callerContext != 0)
        return cResultInvalidState;
    if (g_piaMesh->IsHost() && !g_piaMesh->IsHostMigrationEnabled())
        return cResultInvalidState;
    m_callerContext = callerContext;
    callerContext->InitiateCall();
    m_isSingleStation = false;
    Reset(true);
    m_startTime = common::g_piaJobScheduler->m_dispatchTime;
    JointSessionJob* jointSessionJob = g_piaSession->m_jointSessionJob;
    if (jointSessionJob != 0)
    {
        jointSessionJob->m_failurePending = 1;
        jointSessionJob->m_pendingFailureResult = cResultCancelled;
    }
    setStep(&LeaveSessionJob::WaitForcedTerminatingOfJointSessionJob,
            "LeaveSessionJob::WaitForcedTerminatingOfJointSessionJob");
    return 0;
}

LeaveSessionJob::ExecuteResult LeaveSessionJob::WaitForcedTerminatingOfJointSessionJob()
{
    const int status = g_piaSession->GetStatus();
    if (status == Session::cStatusDisconnectedNetwork)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::SendMonitoringData),
                 "nn::pia::session::LeaveSessionJob::SendMonitoringData"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    else if (status == Session::cStatusUnknown7)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::SendMonitoringData),
                 "nn::pia::session::LeaveSessionJob::SendMonitoringData"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    else if (status == Session::cStatusUnknown8)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::SendMonitoringData),
                 "nn::pia::session::LeaveSessionJob::SendMonitoringData"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (g_piaSession->m_jointSessionJob != 0 && g_piaSession->m_jointSessionJob->IsRunning())
        return MakeExecuteResult(cExecuteKindNextStep);
    if (g_piaSession->IsHostMigrationEnabled() && g_piaSession->IsJointSessionHost()
        && g_piaSession->m_matchmakeStatus == cMatchmakeStatusJointSessionProcessed)
        SetNextStepAtWaitForcedTerminatingOfJointSessionJob();
    else
        SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::LeaveMesh),
                            "LeaveSessionJob::LeaveMesh"));
    return MakeExecuteResult(cExecuteKindContinue);
}

LeaveSessionJob::ExecuteResult LeaveSessionJob::LeaveMesh()
{
    const int status = g_piaSession->GetStatus();
    if (status == Session::cStatusDisconnectedNetwork)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::SendMonitoringData),
                 "nn::pia::session::LeaveSessionJob::SendMonitoringData"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    else if (status == Session::cStatusUnknown7)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::SendMonitoringData),
                 "nn::pia::session::LeaveSessionJob::SendMonitoringData"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    else if (status == Session::cStatusUnknown8)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::SendMonitoringData),
                 "nn::pia::session::LeaveSessionJob::SendMonitoringData"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    g_piaMesh->SetSkipReport(true);
    if (g_piaMesh->IsHost() && g_piaSession->IsHostMigrationEnabled())
    {
        if (g_piaSession->GetStationNum() == 1)
            m_isSingleStation = true;
        const Result result = g_piaMesh->LeaveMeshWithHostMigration(&m_callContext);
        if (result != 0)
        {
            SetNextStepAtWaitLeaveMesh();
            return MakeExecuteResult(cExecuteKindContinue);
        }
        SetCurrentStep(Step(
            static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::WaitLeaveMeshWithHostMigration),
            "LeaveSessionJob::WaitLeaveMeshWithHostMigration"));
    }
    else
    {
        const Result result = g_piaMesh->LeaveMesh(&m_callContext);
        if (g_piaSession->m_postponeNotice)
            g_piaSession->m_postponeNotice = false;
        if (result != 0)
        {
            SetNextStepAtWaitLeaveMesh();
            return MakeExecuteResult(cExecuteKindContinue);
        }
        SetCurrentStep(Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::WaitLeaveMesh),
                            "LeaveSessionJob::WaitLeaveMesh"));
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

LeaveSessionJob::ExecuteResult LeaveSessionJob::WaitLeaveMesh()
{
    const int status = g_piaSession->GetStatus();
    if (status == Session::cStatusDisconnectedNetwork)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::SendMonitoringData),
                 "nn::pia::session::LeaveSessionJob::SendMonitoringData"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    else if (status == Session::cStatusUnknown7)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::SendMonitoringData),
                 "nn::pia::session::LeaveSessionJob::SendMonitoringData"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    else if (status == Session::cStatusUnknown8)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::SendMonitoringData),
                 "nn::pia::session::LeaveSessionJob::SendMonitoringData"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (g_piaSession->GetStatus() == Session::cStatusUnknown6)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::SendMonitoringData),
                 "nn::pia::session::LeaveSessionJob::SendMonitoringData"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_callContext.m_state == common::CallContext::cStateSuccess
        || m_callContext.m_state == common::CallContext::cStateFailure
        || m_callContext.m_state == common::CallContext::cStateCancelled)
    {
        SetNextStepAtWaitLeaveMesh();
        return MakeExecuteResult(cExecuteKindContinue);
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

LeaveSessionJob::ExecuteResult LeaveSessionJob::WaitLeaveMeshWithHostMigration()
{
    const int status = g_piaSession->GetStatus();
    if (status == Session::cStatusDisconnectedNetwork)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        setStep(&LeaveSessionJob::SendMonitoringData, "nn::pia::session::LeaveSessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    else if (status == Session::cStatusUnknown7)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        setStep(&LeaveSessionJob::SendMonitoringData, "nn::pia::session::LeaveSessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    else if (status == Session::cStatusUnknown8)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        setStep(&LeaveSessionJob::SendMonitoringData, "nn::pia::session::LeaveSessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_callContext.m_state == common::CallContext::cStateSuccess
        || m_callContext.m_state == common::CallContext::cStateFailure
        || m_callContext.m_state == common::CallContext::cStateCancelled)
    {
        if (m_isSingleStation)
        {
            SetNextStepAtWaitLeaveMesh();
            return MakeExecuteResult(cExecuteKindContinue);
        }
        const int timeoutMsec = GetWaitHostMigrationTimeout();
        const common::TimeSpan timeout(common::TimeSpan::GetTicksPerMSec().m_tick * timeoutMsec);
        m_hostMigrationTimeoutTime = common::g_piaJobScheduler->m_dispatchTime + timeout;
        setStep(&LeaveSessionJob::WaitHostMigrated, "LeaveSessionJob::WaitHostMigrated");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

LeaveSessionJob::ExecuteResult LeaveSessionJob::WaitHostMigrated()
{
    const int status = g_piaSession->GetStatus();
    if (status == Session::cStatusDisconnectedNetwork)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::SendMonitoringData),
                 "nn::pia::session::LeaveSessionJob::SendMonitoringData"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    else if (status == Session::cStatusUnknown7)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::SendMonitoringData),
                 "nn::pia::session::LeaveSessionJob::SendMonitoringData"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    else if (status == Session::cStatusUnknown8)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::SendMonitoringData),
                 "nn::pia::session::LeaveSessionJob::SendMonitoringData"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (g_piaSession->GetStatus() == Session::cStatusUnknown6)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::SendMonitoringData),
                 "nn::pia::session::LeaveSessionJob::SendMonitoringData"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_hostMigrationTimeoutTime <= common::g_piaJobScheduler->m_dispatchTime)
    {
        SetNextStepAtWaitHostMigratedTimeout();
        return MakeExecuteResult(cExecuteKindContinue);
    }
    Session* session = g_piaSession;
    const unsigned int bufferIndex = session->m_currentSessionIndex == 0;
    if (session->m_sessionId[bufferIndex] != 0)
    {
        if (session->m_matchmakeSession[bufferIndex]->IsMatchmakeOwner())
        {
            if (session->m_matchmakeStatus == cMatchmakeStatusJointSessionProcessed
                && session->GetMatchmakeSessionStationNum() <= cHostMigrationRemainingStationNum)
            {
                SetNextStepAtWaitLeaveMesh();
                return MakeExecuteResult(cExecuteKindContinue);
            }
            if (session->m_matchmakeStatus == cMatchmakeStatusJointSessionProcessing)
            {
                SetNextStepAtWaitLeaveMesh();
                return MakeExecuteResult(cExecuteKindContinue);
            }
            return MakeExecuteResult(cExecuteKindNextStep);
        }
        if (session->m_matchmakeSession[session->m_currentSessionIndex]->IsMatchmakeOwner())
            session->m_stationIdStatusTable->GetStationIdNum(session->m_sessionId[session->m_currentSessionIndex]);
    }
    else if (session->m_sessionId[session->m_currentSessionIndex] != 0
             && session->m_matchmakeSession[session->m_currentSessionIndex]->IsMatchmakeOwner())
    {
        if (session->m_matchmakeStatus == cMatchmakeStatusMatched
            && session->GetMatchmakeSessionStationNum() <= cHostMigrationRemainingStationNum)
        {
            SetNextStepAtWaitLeaveMesh();
            return MakeExecuteResult(cExecuteKindContinue);
        }
        if (session->m_matchmakeStatus == cMatchmakeStatusJointSessionProcessing)
        {
            SetNextStepAtWaitLeaveMesh();
            return MakeExecuteResult(cExecuteKindContinue);
        }
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    SetNextStepAtWaitLeaveMesh();
    return MakeExecuteResult(cExecuteKindContinue);
}

LeaveSessionJob::ExecuteResult LeaveSessionJob::LeaveCurrentMatchmakeSession()
{
    const int status = g_piaSession->GetStatus();
    if (status == Session::cStatusDisconnectedNetwork)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::SendMonitoringData),
                 "nn::pia::session::LeaveSessionJob::SendMonitoringData"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    else if (status == Session::cStatusUnknown7)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::SendMonitoringData),
                 "nn::pia::session::LeaveSessionJob::SendMonitoringData"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    else if (status == Session::cStatusUnknown8)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::SendMonitoringData),
                 "nn::pia::session::LeaveSessionJob::SendMonitoringData"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (g_piaSession->GetStatus() == Session::cStatusUnknown6)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::SendMonitoringData),
                 "nn::pia::session::LeaveSessionJob::SendMonitoringData"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    IMatchmakeSession* matchmakeSession = g_piaSession->m_matchmakeSession[g_piaSession->m_currentSessionIndex];
    if (IsNeedSkipLeaveCurrentMatchmakeSession())
    {
        SetNextStepAtWaitLeaveCurrentMatchmakeSession();
        return MakeExecuteResult(cExecuteKindContinue);
    }
    const Result result = matchmakeSession->LeaveMatchmakeSession(
        &m_callContext, g_piaSession->m_sessionId[g_piaSession->m_currentSessionIndex]);
    if (result != 0)
    {
        SetNextStepAtLeaveCurrentMatchmakeSessionFailure();
        return MakeExecuteResult(cExecuteKindContinue);
    }
    SetCurrentStep(Step(
        static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::WaitLeaveCurrentMatchmakeSession),
        "LeaveSessionJob::WaitLeaveCurrentMatchmakeSession"));
    return MakeExecuteResult(cExecuteKindNextStep);
}

LeaveSessionJob::ExecuteResult LeaveSessionJob::WaitLeaveCurrentMatchmakeSession()
{
    const int status = g_piaSession->GetStatus();
    if (status == Session::cStatusDisconnectedNetwork)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::SendMonitoringData),
                 "nn::pia::session::LeaveSessionJob::SendMonitoringData"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    else if (status == Session::cStatusUnknown7)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::SendMonitoringData),
                 "nn::pia::session::LeaveSessionJob::SendMonitoringData"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    else if (status == Session::cStatusUnknown8)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::SendMonitoringData),
                 "nn::pia::session::LeaveSessionJob::SendMonitoringData"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (g_piaSession->GetStatus() == Session::cStatusUnknown6)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::SendMonitoringData),
                 "nn::pia::session::LeaveSessionJob::SendMonitoringData"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (g_piaSession->m_matchmakeSession[g_piaSession->m_currentSessionIndex]->WaitLeaveMatchmakeSession())
    {
        g_piaSession->m_sessionId[g_piaSession->m_currentSessionIndex] = 0;
        SetNextStepAtWaitLeaveCurrentMatchmakeSession();
        return MakeExecuteResult(cExecuteKindContinue);
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

void LeaveSessionJob::CalcProcessingTimeOfLeave() const
{
    const common::TimeSpan elapsed = common::g_piaJobScheduler->m_dispatchTime - m_startTime;
    const int elapsedMsec = static_cast<int>(elapsed.m_tick / common::TimeSpan::GetTicksPerMSec().m_tick);
    if (elapsedMsec >= 0)
        common::g_sessionStateMonitoringContent.m_leaveProcessingTime = elapsedMsec;
}

LeaveSessionJob::ExecuteResult LeaveSessionJob::SendMonitoringData()
{
    CalcProcessingTimeOfLeave();
    g_piaMesh->SetSkipReport(false);
    g_piaMesh->SendReport(cReportReasonLeave);
    setStep(&LeaveSessionJob::CompleteProcess, "LeaveSessionJob::CompleteProcess");
    return MakeExecuteResult(cExecuteKindWait, cSendMonitoringDataWait);
}

LeaveSessionJob::ExecuteResult LeaveSessionJob::MeshCleanup()
{
    g_piaSession->m_meshLayerController->MeshCleanup();
    SetNextStepAtMeshCleanup();
    return MakeExecuteResult(cExecuteKindContinue);
}

LeaveSessionJob::ExecuteResult LeaveSessionJob::CompleteProcess()
{
    g_piaSession->CleanupStatus(false);
    if (m_callerContext != 0)
    {
        Result result = 0;
        m_callerContext->SignalSuccess(result);
    }
    m_callerContext = 0;
    return MakeExecuteResult(cExecuteKindFinished);
}

LeaveSessionJob::ExecuteResult LeaveSessionJob::LeaveBufferMatchmakeSession()
{
    const int status = g_piaSession->GetStatus();
    if (status == Session::cStatusDisconnectedNetwork)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::SendMonitoringData),
                 "nn::pia::session::LeaveSessionJob::SendMonitoringData"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    else if (status == Session::cStatusUnknown7)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::SendMonitoringData),
                 "nn::pia::session::LeaveSessionJob::SendMonitoringData"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    else if (status == Session::cStatusUnknown8)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::SendMonitoringData),
                 "nn::pia::session::LeaveSessionJob::SendMonitoringData"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (g_piaSession->GetStatus() == Session::cStatusUnknown6)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::SendMonitoringData),
                 "nn::pia::session::LeaveSessionJob::SendMonitoringData"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    IMatchmakeSession* matchmakeSession = g_piaSession->m_matchmakeSession[g_piaSession->m_currentSessionIndex == 0];
    const unsigned int sessionId = g_piaSession->m_sessionId[g_piaSession->m_currentSessionIndex == 0];
    if (sessionId == 0)
    {
        SetNextStepAtWaitLeaveBufferMatchmakeSession();
        return MakeExecuteResult(cExecuteKindContinue);
    }
    const Result result = matchmakeSession->LeaveMatchmakeSession(&m_callContext, sessionId);
    if (result != 0)
    {
        g_piaSession->m_sessionId[g_piaSession->m_currentSessionIndex == 0] = 0;
        SetNextStepAtWaitLeaveBufferMatchmakeSession();
        return MakeExecuteResult(cExecuteKindContinue);
    }
    SetCurrentStep(Step(
        static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::WaitLeaveBufferMatchmakeSession),
        "LeaveSessionJob::WaitLeaveBufferMatchmakeSession"));
    return MakeExecuteResult(cExecuteKindNextStep);
}

LeaveSessionJob::ExecuteResult LeaveSessionJob::WaitLeaveBufferMatchmakeSession()
{
    const int status = g_piaSession->GetStatus();
    if (status == Session::cStatusDisconnectedNetwork)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::SendMonitoringData),
                 "nn::pia::session::LeaveSessionJob::SendMonitoringData"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    else if (status == Session::cStatusUnknown7)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::SendMonitoringData),
                 "nn::pia::session::LeaveSessionJob::SendMonitoringData"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    else if (status == Session::cStatusUnknown8)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::SendMonitoringData),
                 "nn::pia::session::LeaveSessionJob::SendMonitoringData"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (g_piaSession->GetStatus() == Session::cStatusUnknown6)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        SetCurrentStep(
            Step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(&LeaveSessionJob::SendMonitoringData),
                 "nn::pia::session::LeaveSessionJob::SendMonitoringData"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (g_piaSession->m_matchmakeSession[g_piaSession->m_currentSessionIndex == 0]->WaitLeaveMatchmakeSession())
    {
        g_piaSession->m_sessionId[g_piaSession->m_currentSessionIndex == 0] = 0;
        SetNextStepAtWaitLeaveBufferMatchmakeSession();
        return MakeExecuteResult(cExecuteKindContinue);
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

void LeaveSessionJob::SetNextStepAtWaitForcedTerminatingOfJointSessionJob()
{
    setStep(&LeaveSessionJob::LeaveMesh, "LeaveSessionJob::LeaveMesh");
}

void LeaveSessionJob::SetNextStepAtWaitLeaveMesh()
{
    setStep(&LeaveSessionJob::MeshCleanup, "LeaveSessionJob::MeshCleanup");
}

void LeaveSessionJob::SetNextStepAtMeshCleanup()
{
    setStep(&LeaveSessionJob::LeaveCurrentMatchmakeSession, "LeaveSessionJob::LeaveCurrentMatchmakeSession");
}

void LeaveSessionJob::SetNextStepAtWaitLeaveCurrentMatchmakeSession()
{
    setStep(&LeaveSessionJob::SendMonitoringData, "LeaveSessionJob::SendMonitoringData");
}

void LeaveSessionJob::SetNextStepAtLeaveCurrentMatchmakeSessionFailure()
{
    setStep(&LeaveSessionJob::SendMonitoringData, "LeaveSessionJob::SendMonitoringData");
}

void LeaveSessionJob::SetNextStepAtWaitHostMigratedTimeout()
{
    SetNextStepAtWaitLeaveMesh();
}

void LeaveSessionJob::SetNextStepAtWaitLeaveBufferMatchmakeSession()
{
    setStep(&LeaveSessionJob::LeaveCurrentMatchmakeSession, "LeaveSessionJob::LeaveCurrentMatchmakeSession");
}

void LeaveSessionJob::Cleanup()
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
    m_isSingleStation = false;
}

void LeaveSessionJob::Trace(unsigned long long) const
{
}

}
}
}
