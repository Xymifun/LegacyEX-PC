#include "nn/pia/session/UpdateSessionSettingJob.hpp"

#include "nn/pia/Transport.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/Session.hpp"

namespace nn
{
namespace pia
{
namespace session
{

UpdateSessionSettingJob::UpdateSessionSettingJob() : common::StepSequenceJob(), m_callerContext(0), m_callContext()
{
}

UpdateSessionSettingJob::~UpdateSessionSettingJob()
{
}

Result UpdateSessionSettingJob::Startup(common::CallContext* callerContext, unsigned int sessionId,
                                        IMatchmakeSession* matchmakeSession, const UpdateSessionSetting* setting,
                                        bool isJointSession)
{
    if (callerContext == 0)
        return Result(transport::cResultInvalidArgument);

    if (!matchmakeSession->IsMatchmakeOwner())
        return Result(transport::cResultInvalidState);

    const Result result = StartupImpl(sessionId, matchmakeSession, setting, isJointSession);
    if (result != 0)
        return result;

    m_callerContext = callerContext;
    callerContext->InitiateCall();
    m_sessionId = sessionId;
    Reset(true);
    return 0;
}

void UpdateSessionSettingJob::Cleanup()
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
    m_sessionId = 0;
}

UpdateSessionSettingJob::ExecuteResult UpdateSessionSettingJob::CompleteProcess()
{
    Result result = 0;
    m_callerContext->SignalSuccess(result);
    m_callerContext = 0;
    return MakeExecuteResult(cExecuteKindFinished);
}

void UpdateSessionSettingJob::SetCurrentSessionOpenStatus(Session::SessionOpenStatus status)
{
    g_piaSession->SetCurrentSessionOpenStatus(status);
}

void UpdateSessionSettingJob::SetBufferSessionOpenStatus(Session::SessionOpenStatus status)
{
    g_piaSession->SetBufferSessionOpenStatus(status);
}

void UpdateSessionSettingJob::Trace(unsigned long long value) const
{
}

}
}
}
