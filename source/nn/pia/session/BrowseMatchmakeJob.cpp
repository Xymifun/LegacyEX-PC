#include "nn/pia/session/BrowseMatchmakeJob.hpp"

#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/SessionSearchCriteria.hpp"

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
    cResultInvalidArgument = 0x10c07,
    cResultInvalidState = 0x10c08
};
}

BrowseMatchmakeJob::BrowseMatchmakeJob() : common::StepSequenceJob(), m_callerContext(0)
{
}

BrowseMatchmakeJob::~BrowseMatchmakeJob()
{
}

Result BrowseMatchmakeJob::Startup(common::CallContext* callerContext, IMatchmakeSession* matchmakeSession,
                                   const SessionSearchCriteria* criteria)
{
    if (callerContext == 0 || criteria == 0)
        return cResultInvalidArgument;
    if (matchmakeSession == 0)
        return cResultInvalidState;
    if (m_callerContext != 0)
        return cResultInvalidArgument;

    const Result result = StartupImpl(matchmakeSession, criteria);
    if (result != 0)
        return result;

    m_callerContext = callerContext;
    callerContext->InitiateCall();
    Reset(true);
    return 0;
}

Result BrowseMatchmakeJob::Startup(common::CallContext* callerContext, IMatchmakeSession* matchmakeSession,
                                   unsigned int sessionId)
{
    if (callerContext == 0)
        return cResultInvalidArgument;
    if (matchmakeSession == 0)
        return cResultInvalidArgument;
    if (m_callerContext != 0)
        return cResultInvalidArgument;

    const Result result = StartupImpl(matchmakeSession, sessionId);
    if (result != 0)
        return result;

    m_callerContext = callerContext;
    callerContext->InitiateCall();
    Reset(true);
    return 0;
}

BrowseMatchmakeJob::ExecuteResult BrowseMatchmakeJob::CompleteProcess()
{
    common::CallContext* callerContext = m_callerContext;
    Result result = 0;
    callerContext->SignalSuccess(result);
    m_callerContext = 0;
    return MakeExecuteResult(cExecuteKindFinished);
}

void BrowseMatchmakeJob::Cleanup()
{
    if (m_callerContext != 0)
    {
        if (m_callerContext->m_state == common::CallContext::cStateInProgress)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
    }
}

void BrowseMatchmakeJob::Trace(unsigned long long) const
{
}

}
}
}
