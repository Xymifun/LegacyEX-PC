#include "nn/pia/session/DestroySessionJob.hpp"

#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/SessionStateMonitoringContent.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/session/JointSessionJob.hpp"
#include "nn/pia/session/Mesh.hpp"
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
    cResultCancelled = 0x6c05,
    cResultInvalidState = 0x10c08
};

enum
{
    cReportReasonLeave = 1,
    cSendMonitoringDataWait = 50
};
}

DestroySessionJob::DestroySessionJob() : common::StepSequenceJob(), m_callerContext(0), m_callContext(), m_startTime()
{
}

DestroySessionJob::~DestroySessionJob()
{
}

Result DestroySessionJob::Startup(common::CallContext* callerContext)
{
    if (m_callerContext != 0)
        return cResultInvalidState;
    if (g_piaSession->IsHostMigrationEnabled())
        return cResultInvalidState;
    const Result result = StartupImpl();
    if (result != 0)
        return result;
    m_callerContext = callerContext;
    setStep(&DestroySessionJob::WaitForcedTerminatingOfJointSessionJob,
            "DestroySessionJob::WaitForcedTerminatingOfJointSessionJob");
    callerContext->InitiateCall();
    m_hasWaitedJointSessionJob = false;
    Reset(true);
    m_startTime = common::g_piaJobScheduler->m_dispatchTime;
    JointSessionJob* jointSessionJob = g_piaSession->m_jointSessionJob;
    if (jointSessionJob != 0)
    {
        jointSessionJob->m_failurePending = 1;
        jointSessionJob->m_pendingFailureResult = cResultCancelled;
    }
    return 0;
}

DestroySessionJob::ExecuteResult DestroySessionJob::WaitForcedTerminatingOfJointSessionJob()
{
    if (g_piaSession->GetStatus() == Session::cStatusDisconnectedNetwork)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        setStep(&DestroySessionJob::SendMonitoringData, "nn::pia::session::DestroySessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (g_piaSession->m_jointSessionJob != 0 && g_piaSession->m_jointSessionJob->IsRunning())
    {
        m_hasWaitedJointSessionJob = true;
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    setStep(&DestroySessionJob::DestroyMesh, "DestroySessionJob::DestroyMesh");
    return MakeExecuteResult(cExecuteKindContinue);
}

DestroySessionJob::ExecuteResult DestroySessionJob::DestroyMesh()
{
    if (g_piaSession->GetStatus() == Session::cStatusDisconnectedNetwork)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        setStep(&DestroySessionJob::SendMonitoringData, "nn::pia::session::DestroySessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    g_piaMesh->SetSkipReport(true);
    if (m_hasWaitedJointSessionJob)
    {
        SetNextStepAtWaitDestroyMesh();
        return MakeExecuteResult(cExecuteKindContinue);
    }
    const Result result = g_piaMesh->DestroyMesh(&m_callContext);
    if (result != 0)
    {
        SetNextStepAtWaitDestroyMesh();
        return MakeExecuteResult(cExecuteKindContinue);
    }
    setStep(&DestroySessionJob::WaitDestroyMesh, "DestroySessionJob::WaitDestroyMesh");
    return MakeExecuteResult(cExecuteKindNextStep);
}

DestroySessionJob::ExecuteResult DestroySessionJob::WaitDestroyMesh()
{
    if (g_piaSession->GetStatus() == Session::cStatusDisconnectedNetwork)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        setStep(&DestroySessionJob::SendMonitoringData, "nn::pia::session::DestroySessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_callContext.m_state - common::CallContext::cStateSuccess <= 2u)
    {
        SetNextStepAtWaitDestroyMesh();
        return MakeExecuteResult(cExecuteKindContinue);
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

DestroySessionJob::ExecuteResult DestroySessionJob::MeshCleanup()
{
    g_piaSession->m_meshLayerController->MeshCleanup();
    SetNextStepAtMeshCleanup();
    return MakeExecuteResult(cExecuteKindContinue);
}

void DestroySessionJob::CalcProcessingTimeOfLeave() const
{
    const common::TimeSpan elapsed(common::g_piaJobScheduler->m_dispatchTime.m_tick - m_startTime.m_tick);
    const int elapsedMsec = static_cast<int>(elapsed.m_tick / common::TimeSpan::GetTicksPerMSec().m_tick);
    if (elapsedMsec >= 0)
        common::g_sessionStateMonitoringContent.m_leaveProcessingTime = elapsedMsec;
}

DestroySessionJob::ExecuteResult DestroySessionJob::SendMonitoringData()
{
    CalcProcessingTimeOfLeave();
    g_piaMesh->SetSkipReport(false);
    g_piaMesh->SendReport(cReportReasonLeave);
    setStep(&DestroySessionJob::CompleteProcess, "DestroySessionJob::CompleteProcess");
    return MakeExecuteResult(cExecuteKindWait, cSendMonitoringDataWait);
}

DestroySessionJob::ExecuteResult DestroySessionJob::CompleteProcess()
{
    Session* piaSession = g_piaSession;
    piaSession->CleanupStatus(false);
    if (m_callerContext != 0)
        m_callerContext->SignalSuccess(0);
    m_callerContext = 0;
    return MakeExecuteResult(cExecuteKindFinished);
}

void DestroySessionJob::Cleanup()
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

void DestroySessionJob::Trace(unsigned long long) const
{
}

}
}
}
