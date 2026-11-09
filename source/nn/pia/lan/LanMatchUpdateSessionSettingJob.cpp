#include "nn/pia/lan/LanMatchUpdateSessionSettingJob.hpp"

#include "nn/pia/Transport.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/lan/LanMatchmakeSession.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/session/UpdateSessionSetting.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

namespace
{
typedef common::Job::ExecuteResult ExecuteResult;

enum
{
    cResultCancelled = 0x6c05
};

inline void SelectStep(common::StepSequenceJob::Step& slot,
                       ExecuteResult (LanMatchUpdateSessionSettingJob::*function)(), const char* name)
{
    const common::StepSequenceJob::Step step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(function), name);
    slot = step;
}
}

LanMatchUpdateSessionSettingJob::LanMatchUpdateSessionSettingJob()
    : session::UpdateSessionSettingJob(),
      m_matchmakeSession(0)
{
}

LanMatchUpdateSessionSettingJob::~LanMatchUpdateSessionSettingJob()
{
}

Result LanMatchUpdateSessionSettingJob::StartupImpl(unsigned int sessionId,
                                                    session::IMatchmakeSession* matchmakeSession,
                                                    const session::UpdateSessionSetting* setting, bool isJointSession)
{
    m_matchmakeSession = static_cast<LanMatchmakeSession*>(matchmakeSession);
    if (m_matchmakeSession->GetGatheringId() != sessionId)
        return transport::cResultInvalidArgument;
    m_isJointSession = isJointSession;
    m_matchmakeSession->ReflectSettingToMatchmakeSession(setting, isJointSession);
    SelectStep(m_currentStep, &LanMatchUpdateSessionSettingJob::UpdateSessionSetting,
               "LanMatchUpdateSessionSettingJob::UpdateSessionSetting");
    return 0;
}

LanMatchUpdateSessionSettingJob::ExecuteResult LanMatchUpdateSessionSettingJob::UpdateSessionSetting()
{
    const int status = session::g_piaSession->GetStatus();
    if (status == session::Session::cStatusDisconnectedNetwork)
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusDisconnectedNetwork);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        m_callerContext->SignalFailure(cResultCancelled);
        m_callerContext = 0;
        Cleanup();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    const Result result = m_matchmakeSession->UpdateSessionSetting(&m_callContext, m_sessionId);
    if (result != 0)
    {
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &LanMatchUpdateSessionSettingJob::WaitUpdateSessionSetting,
               "LanMatchUpdateSessionSettingJob::WaitUpdateSessionSetting");
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchUpdateSessionSettingJob::ExecuteResult LanMatchUpdateSessionSettingJob::WaitUpdateSessionSetting()
{
    const int status = session::g_piaSession->GetStatus();
    if (status == session::Session::cStatusDisconnectedNetwork)
    {
        m_callerContext->SignalFailure(session::Session::cResultStatusDisconnectedNetwork);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        m_callerContext->SignalFailure(cResultCancelled);
        m_callerContext = 0;
        Cleanup();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (m_matchmakeSession->WaitUpdateSessionSetting())
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            m_callerContext->SignalFailure(m_callContext.m_result);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            if (m_isJointSession
                && m_sessionId == session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex])
                SetCurrentSessionOpenStatus(session::Session::cSessionOpenStatusOpen);
            SelectStep(m_currentStep, &LanMatchUpdateSessionSettingJob::CompleteProcess,
                       "UpdateSessionSettingJob::CompleteProcess");
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

void LanMatchUpdateSessionSettingJob::Cleanup()
{
    session::UpdateSessionSettingJob::Cleanup();
    m_matchmakeSession = 0;
}

void LanMatchUpdateSessionSettingJob::Trace(unsigned long long) const
{
}

}
}
}
