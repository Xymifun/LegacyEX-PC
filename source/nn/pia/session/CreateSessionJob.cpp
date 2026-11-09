#include "nn/pia/session/CreateSessionJob.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/common/SessionStateMonitoringContent.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/MeshLayerController.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/session/StationIdStatusTable.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/StationIdTable.hpp"

namespace nn
{
namespace pia
{
namespace session
{
namespace
{
inline void SelectStep(common::StepSequenceJob::Step& slot,
                       CreateSessionJob::ExecuteResult (CreateSessionJob::*function)(), const char* name)
{
    const common::StepSequenceJob::Step step(
        static_cast<CreateSessionJob::ExecuteResult (common::StepSequenceJob::*)()>(function), name);
    slot = step;
}
}

CreateSessionJob::CreateSessionJob() : common::StepSequenceJob(), m_callerContext(0), m_callContext(), m_result(0)
{
}

CreateSessionJob::~CreateSessionJob()
{
}

Result CreateSessionJob::Startup(common::CallContext* callerContext, const CreateSessionSetting* setting)
{
    if (callerContext == 0 || setting == 0)
        return static_cast<Result>(transport::cResultInvalidArgument);
    const Result result = StartupImpl(setting);
    if (result != 0)
        return result;
    m_callerContext = callerContext;
    callerContext->InitiateCall();
    Reset(true);
    return 0;
}

CreateSessionJob::ExecuteResult CreateSessionJob::MeshStartup()
{
    unsigned int status = g_piaSession->GetStatus();
    switch (status)
    {
    case Session::cStatusDisconnectedNetwork:
        SelectStep(m_currentStep, &CreateSessionJob::CompleteFailure,
                   "nn::pia::session::CreateSessionJob::CompleteFailure");
        m_result = Session::cResultStatusDisconnectedNetwork;
        return MakeExecuteResult(cExecuteKindContinue);
    case Session::cStatusUnknown6:
        SelectStep(m_currentStep, &CreateSessionJob::CompleteFailure,
                   "nn::pia::session::CreateSessionJob::CompleteFailure");
        m_result = Session::cResultStatusUnknown6;
        return MakeExecuteResult(cExecuteKindContinue);
    case Session::cStatusUnknown7:
        SelectStep(m_currentStep, &CreateSessionJob::CompleteFailure,
                   "nn::pia::session::CreateSessionJob::CompleteFailure");
        m_result = Session::cResultStatusUnknown7;
        return MakeExecuteResult(cExecuteKindContinue);
    case Session::cStatusUnknown8:
        SelectStep(m_currentStep, &CreateSessionJob::CompleteFailure,
                   "nn::pia::session::CreateSessionJob::CompleteFailure");
        m_result = Session::cResultStatusUnknown8;
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    Session* session = g_piaSession;
    const Result result = session->m_meshLayerController->MeshStartup(
        session->m_matchmakeSession[session->m_currentSessionIndex], false);
    if (result != 0)
        return SetNextStepAtFailedCreateMesh(result);
    session->m_postponeNotice = true;
    transport::g_piaTransport->m_stationIdTable->SetAvailableEntryNum(
        session->m_fieldc2[session->m_currentSessionIndex]);
    SelectStep(m_currentStep, &CreateSessionJob::CreateMesh, "CreateSessionJob::CreateMesh");
    return MakeExecuteResult(cExecuteKindContinue);
}

CreateSessionJob::ExecuteResult CreateSessionJob::CreateMesh()
{
    const Result result = g_piaMesh->CreateMesh(&m_callContext);
    if (result != 0)
    {
        g_piaSession->m_meshLayerController->MeshCleanup();
        return SetNextStepAtFailedCreateMesh(result);
    }
    g_piaMesh->SetSkipReport(true);
    SelectStep(m_currentStep, &CreateSessionJob::WaitCreateMesh, "CreateSessionJob::WaitCreateMesh");
    return MakeExecuteResult(cExecuteKindNextStep);
}

CreateSessionJob::ExecuteResult CreateSessionJob::WaitCreateMesh()
{
    unsigned int status = g_piaSession->GetStatus();
    switch (status)
    {
    case Session::cStatusDisconnectedNetwork:
        SelectStep(m_currentStep, &CreateSessionJob::CompleteFailure,
                   "nn::pia::session::CreateSessionJob::CompleteFailure");
        m_result = Session::cResultStatusDisconnectedNetwork;
        return MakeExecuteResult(cExecuteKindContinue);
    case Session::cStatusUnknown6:
        SelectStep(m_currentStep, &CreateSessionJob::CompleteFailure,
                   "nn::pia::session::CreateSessionJob::CompleteFailure");
        m_result = Session::cResultStatusUnknown6;
        return MakeExecuteResult(cExecuteKindContinue);
    case Session::cStatusUnknown7:
        SelectStep(m_currentStep, &CreateSessionJob::CompleteFailure,
                   "nn::pia::session::CreateSessionJob::CompleteFailure");
        m_result = Session::cResultStatusUnknown7;
        return MakeExecuteResult(cExecuteKindContinue);
    case Session::cStatusUnknown8:
        SelectStep(m_currentStep, &CreateSessionJob::CompleteFailure,
                   "nn::pia::session::CreateSessionJob::CompleteFailure");
        m_result = Session::cResultStatusUnknown8;
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    if (m_callContext.m_state != common::CallContext::cStateFailure)
    {
        if (m_callContext.m_state == common::CallContext::cStateSuccess
            || m_callContext.m_state == common::CallContext::cStateCancelled)
        {
            const Result result = m_callContext.m_result;
            Session* session = g_piaSession;
            if (result != 0)
            {
                session->m_meshLayerController->MeshCleanup();
                return SetNextStepAtFailedCreateMesh(result);
            }
            session->m_matchmakeStatus = Session::cMatchmakeStatusMatched;
            g_piaSession->m_disconnectStatus = Session::cConnectionStatusConnected;
            if (!session->UpdateStationIdToBecomeHost())
            {
                session->m_meshLayerController->MeshCleanup();
                return SetNextStepAtFailedCreateMesh(transport::cResultInvalidState);
            }
            session->m_postponeNotice = false;
            session->m_stationIdStatusTable->CallJoinEventNotYetNotified();
            SetNextStepAtWaitCreateMesh();
            return MakeExecuteResult(cExecuteKindContinue);
        }
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    g_piaSession->m_meshLayerController->MeshCleanup();
    return SetNextStepAtFailedCreateMesh(m_callContext.m_result);
}

CreateSessionJob::ExecuteResult CreateSessionJob::CompleteFailure()
{
    g_piaSession->m_meshLayerController->MeshCleanup();
    if (m_result == transport::cResultInvalidState || m_result == Session::cResultStatusDisconnectedNetwork)
    {
        g_piaSession->ChangeStatusToDisconnectedNetwork();
        m_callerContext->SignalFailure(Session::cResultStatusDisconnectedNetwork);
    }
    else
    {
        m_callerContext->SignalFailure(m_result);
    }
    m_callerContext = 0;
    g_piaMesh->SetSkipReport(false);
    g_piaMesh->SendReport(0);
    common::g_sessionBeginMonitoringContent.Cleanup();
    common::g_sessionStateMonitoringContent.Cleanup();
    Cleanup();
    return MakeExecuteResult(cExecuteKindFinished);
}

CreateSessionJob::ExecuteResult CreateSessionJob::CompleteProcess()
{
    m_callerContext->SignalSuccess(0);
    m_callerContext = 0;
    g_piaMesh->SetSkipReport(false);
    g_piaMesh->SendReport(0);
    return MakeExecuteResult(cExecuteKindFinished);
}

void CreateSessionJob::Cleanup()
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
    m_result = 0;
}

void CreateSessionJob::Trace(unsigned long long) const
{
}

void CreateSessionJob::SetMatchmakeStatus(Session::MatchmakeStatus status)
{
    g_piaSession->m_matchmakeStatus = status;
}

void CreateSessionJob::SetConnectionStatus(Session::ConnectionStatus status)
{
    g_piaSession->m_disconnectStatus = status;
}

}
}
}
