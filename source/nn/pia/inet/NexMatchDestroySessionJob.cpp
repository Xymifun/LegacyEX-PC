#include "nn/pia/inet/NexMatchDestroySessionJob.hpp"

#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/MeshLayerController.hpp"
#include "nn/pia/session/Session.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

namespace
{
typedef common::Job::ExecuteResult ExecuteResult;

inline void SelectStep(common::StepSequenceJob::Step& slot, ExecuteResult (session::DestroySessionJob::*function)(),
                       const char* name)
{
    const common::StepSequenceJob::Step step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(function), name);
    slot = step;
}
}

NexMatchDestroySessionJob::NexMatchDestroySessionJob() : session::DestroySessionJob()
{
}

NexMatchDestroySessionJob::~NexMatchDestroySessionJob()
{
}

NexMatchDestroySessionJob::ExecuteResult NexMatchDestroySessionJob::UnregisterCurrentMatchmakeSession()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchDestroySessionJob::SendMonitoringData,
                   "nn::pia::inet::NexMatchDestroySessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchDestroySessionJob::SendMonitoringData,
                   "nn::pia::inet::NexMatchDestroySessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchDestroySessionJob::SendMonitoringData,
                   "nn::pia::inet::NexMatchDestroySessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchDestroySessionJob::SendMonitoringData,
                   "nn::pia::inet::NexMatchDestroySessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    const unsigned int index = session::g_piaSession->m_currentSessionIndex;
    const unsigned int sessionId = session::g_piaSession->m_sessionId[index];
    session::IMatchmakeSession* matchmakeSession = session::g_piaSession->m_matchmakeSession[index];
    if (sessionId == 0)
    {
        SelectStep(m_currentStep, &NexMatchDestroySessionJob::SendMonitoringData,
                   "DestroySessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    const Result result = matchmakeSession->UnregisterMatchmakeSession(&m_callContext, sessionId);
    if (result != 0)
    {
        SelectStep(m_currentStep, &NexMatchDestroySessionJob::SendMonitoringData,
                   "DestroySessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    SelectStep(m_currentStep,
               static_cast<ExecuteResult (session::DestroySessionJob::*)()>(
                   &NexMatchDestroySessionJob::WaitUnregisterCurrentMatchmakeSession),
               "NexMatchDestroySessionJob::WaitUnregisterCurrentMatchmakeSession");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchDestroySessionJob::ExecuteResult NexMatchDestroySessionJob::WaitUnregisterCurrentMatchmakeSession()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchDestroySessionJob::SendMonitoringData,
                   "nn::pia::inet::NexMatchDestroySessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchDestroySessionJob::SendMonitoringData,
                   "nn::pia::inet::NexMatchDestroySessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchDestroySessionJob::SendMonitoringData,
                   "nn::pia::inet::NexMatchDestroySessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchDestroySessionJob::SendMonitoringData,
                   "nn::pia::inet::NexMatchDestroySessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    if (!session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex]
             ->WaitUnregisterMatchmakeSession())
        return MakeExecuteResult(cExecuteKindNextStep);
    session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex] = 0;
    SelectStep(m_currentStep, &NexMatchDestroySessionJob::SendMonitoringData, "DestroySessionJob::SendMonitoringData");
    return MakeExecuteResult(cExecuteKindContinue);
}

NexMatchDestroySessionJob::ExecuteResult NexMatchDestroySessionJob::UnregisterBufferMatchmakeSession()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchDestroySessionJob::SendMonitoringData,
                   "nn::pia::inet::NexMatchDestroySessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchDestroySessionJob::SendMonitoringData,
                   "nn::pia::inet::NexMatchDestroySessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchDestroySessionJob::SendMonitoringData,
                   "nn::pia::inet::NexMatchDestroySessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchDestroySessionJob::SendMonitoringData,
                   "nn::pia::inet::NexMatchDestroySessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    if (session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex == 0] == 0)
    {
        SelectStep(m_currentStep,
                   static_cast<ExecuteResult (session::DestroySessionJob::*)()>(
                       &NexMatchDestroySessionJob::UnregisterCurrentMatchmakeSession),
                   "NexMatchDestroySessionJob::UnregisterCurrentMatchmakeSession");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    const Result result =
        session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex == 0]
            ->UnregisterMatchmakeSession(
                &m_callContext, session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex == 0]);
    if (result != 0)
    {
        SelectStep(m_currentStep,
                   static_cast<ExecuteResult (session::DestroySessionJob::*)()>(
                       &NexMatchDestroySessionJob::UnregisterCurrentMatchmakeSession),
                   "NexMatchDestroySessionJob::UnregisterCurrentMatchmakeSession");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    SelectStep(m_currentStep,
               static_cast<ExecuteResult (session::DestroySessionJob::*)()>(
                   &NexMatchDestroySessionJob::WaitUnregisterBufferMatchmakeSession),
               "NexMatchDestroySessionJob::WaitUnregisterBufferMatchmakeSession");
    return MakeExecuteResult(cExecuteKindContinue);
}

NexMatchDestroySessionJob::ExecuteResult NexMatchDestroySessionJob::WaitUnregisterBufferMatchmakeSession()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchDestroySessionJob::SendMonitoringData,
                   "nn::pia::inet::NexMatchDestroySessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown6:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchDestroySessionJob::SendMonitoringData,
                   "nn::pia::inet::NexMatchDestroySessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown7:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchDestroySessionJob::SendMonitoringData,
                   "nn::pia::inet::NexMatchDestroySessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    case session::Session::cStatusUnknown8:
        session::g_piaSession->m_meshLayerController->MeshCleanup();
        SelectStep(m_currentStep, &NexMatchDestroySessionJob::SendMonitoringData,
                   "nn::pia::inet::NexMatchDestroySessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    default:
        break;
    }
    if (!session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex == 0]
             ->WaitUnregisterMatchmakeSession())
        return MakeExecuteResult(cExecuteKindNextStep);
    session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex == 0] = 0;
    SelectStep(m_currentStep,
               static_cast<ExecuteResult (session::DestroySessionJob::*)()>(
                   &NexMatchDestroySessionJob::UnregisterCurrentMatchmakeSession),
               "NexMatchDestroySessionJob::UnregisterCurrentMatchmakeSession");
    return MakeExecuteResult(cExecuteKindContinue);
}

void NexMatchDestroySessionJob::Cleanup()
{
    session::DestroySessionJob::Cleanup();
}

void NexMatchDestroySessionJob::Trace(unsigned long long) const
{
}

Result NexMatchDestroySessionJob::StartupImpl()
{
    return 0;
}

void NexMatchDestroySessionJob::SetNextStepAtWaitDestroyMesh()
{
    SelectStep(m_currentStep, &NexMatchDestroySessionJob::MeshCleanup, "NexMatchDestroySessionJob::MeshCleanup");
}

void NexMatchDestroySessionJob::SetNextStepAtMeshCleanup()
{
    SelectStep(m_currentStep,
               static_cast<ExecuteResult (session::DestroySessionJob::*)()>(
                   &NexMatchDestroySessionJob::UnregisterBufferMatchmakeSession),
               "NexMatchDestroySessionJob::UnregisterBufferMatchmakeSession");
}

}
}
}
