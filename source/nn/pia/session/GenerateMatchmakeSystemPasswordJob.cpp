#include "nn/pia/session/GenerateMatchmakeSystemPasswordJob.hpp"
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

GenerateMatchmakeSystemPasswordJob::GenerateMatchmakeSystemPasswordJob()
    : common::StepSequenceJob(),
      m_sessionId(0),
      m_callerContext(0),
      m_callContext()
{
}

GenerateMatchmakeSystemPasswordJob::~GenerateMatchmakeSystemPasswordJob()
{
}

Result GenerateMatchmakeSystemPasswordJob::Startup(common::CallContext* callerContext, unsigned int sessionId,
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

GenerateMatchmakeSystemPasswordJob::ExecuteResult GenerateMatchmakeSystemPasswordJob::CompleteProcess()
{
    Result result = 0;
    m_callerContext->SignalSuccess(result);
    m_callerContext = 0;
    return MakeExecuteResult(cExecuteKindFinished);
}

GenerateMatchmakeSystemPasswordJob::ExecuteResult GenerateMatchmakeSystemPasswordJob::FailureProcess()
{
    if (m_callerContext->m_result == static_cast<Result>(cResultDisconnectedSession))
        g_piaSession->ChangeStatusToDisconnectedSession();
    m_callerContext = 0;
    return MakeExecuteResult(cExecuteKindFinished);
}

void GenerateMatchmakeSystemPasswordJob::Cleanup()
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

void GenerateMatchmakeSystemPasswordJob::Trace(unsigned long long) const
{
}

}
}
}
