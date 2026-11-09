#include "nn/pia/lan/LanMatchDestroySessionJob.hpp"

#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/Session.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

LanMatchDestroySessionJob::LanMatchDestroySessionJob() : session::DestroySessionJob()
{
}

LanMatchDestroySessionJob::~LanMatchDestroySessionJob()
{
}

LanMatchDestroySessionJob::ExecuteResult LanMatchDestroySessionJob::UnregisterCurrentMatchmakeSession()
{
    const Result result = session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex]
                              ->UnregisterMatchmakeSession(
                                  &m_callContext,
                                  session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex]);
    if (result != 0)
    {
        setStep(&session::DestroySessionJob::SendMonitoringData, "DestroySessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    setStep(static_cast<ExecuteResult (session::DestroySessionJob::*)()>(
                &LanMatchDestroySessionJob::WaitUnregisterCurrentMatchmakeSession),
            "LanMatchDestroySessionJob::WaitUnregisterCurrentMatchmakeSession");
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchDestroySessionJob::ExecuteResult LanMatchDestroySessionJob::WaitUnregisterCurrentMatchmakeSession()
{
    const bool isCompleted = session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex]
                                 ->WaitUnregisterMatchmakeSession();
    if (isCompleted)
    {
        session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex] = 0;
        setStep(&session::DestroySessionJob::SendMonitoringData, "DestroySessionJob::SendMonitoringData");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

void LanMatchDestroySessionJob::Cleanup()
{
    session::DestroySessionJob::Cleanup();
}

void LanMatchDestroySessionJob::Trace(unsigned long long) const
{
}

Result LanMatchDestroySessionJob::StartupImpl()
{
    return 0;
}

void LanMatchDestroySessionJob::SetNextStepAtWaitDestroyMesh()
{
    setStep(&session::DestroySessionJob::MeshCleanup, "LanMatchDestroySessionJob::MeshCleanup");
}

void LanMatchDestroySessionJob::SetNextStepAtMeshCleanup()
{
    setStep(static_cast<ExecuteResult (session::DestroySessionJob::*)()>(
                &LanMatchDestroySessionJob::UnregisterCurrentMatchmakeSession),
            "LanMatchDestroySessionJob::UnregisterCurrentMatchmakeSession");
}

}
}
}
