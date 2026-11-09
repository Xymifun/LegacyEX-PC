#include "nn/pia/session/ClearMatchmakeSystemPasswordJob.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"
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
    cResultDisconnectedSession = 0x8c56,
    cResultInvalidArgument = 0x10c07,
    cResultInvalidState = 0x10c08
};
}

ClearMatchmakeSystemPasswordJob::ClearMatchmakeSystemPasswordJob()
    : common::StepSequenceJob(),
      m_sessionId(0),
      m_callerContext(0),
      m_callContext()
{
}

ClearMatchmakeSystemPasswordJob::~ClearMatchmakeSystemPasswordJob()
{
}

Result ClearMatchmakeSystemPasswordJob::Startup(common::CallContext* callerContext, unsigned int sessionId,
                                                IMatchmakeSession* session)
{
    if (callerContext == 0 || session == 0)
        return cResultInvalidArgument;
    const bool isOwner = session->IsMatchmakeOwner();
    if (!isOwner)
        return cResultInvalidState;

    const Result operationResult = StartupImpl();
    if (operationResult != 0)
        return operationResult;

    m_callerContext = callerContext;
    callerContext->InitiateCall();
    m_sessionId = sessionId;
    Reset(true);
    return 0;
}

ClearMatchmakeSystemPasswordJob::ExecuteResult ClearMatchmakeSystemPasswordJob::CompleteProcess()
{
    Result result = 0;
    m_callerContext->SignalSuccess(result);
    m_callerContext = 0;
    return MakeExecuteResult(cExecuteKindFinished);
}

ClearMatchmakeSystemPasswordJob::ExecuteResult ClearMatchmakeSystemPasswordJob::FailureProcess()
{
    if (m_callerContext->m_result == static_cast<Result>(cResultDisconnectedSession))
        g_piaSession->ChangeStatusToDisconnectedSession();
    m_callerContext = 0;
    return MakeExecuteResult(cExecuteKindFinished);
}

void ClearMatchmakeSystemPasswordJob::Cleanup()
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

void ClearMatchmakeSystemPasswordJob::Trace(unsigned long long) const
{
}

}
}
}
