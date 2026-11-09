#include "nn/pia/lan/LanMatchBrowseMatchmakeJob.hpp"

#include "nn/pia/lan/LanMatchmakeSession.hpp"
#include "nn/pia/session/SessionSearchCriteria.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

namespace
{
enum
{
    cResultCancelled = 0x6c05,
    cResultInvalidArgument = 0x10c07
};

enum
{
    cBrowseSearchCriteriaNum = 1,
    cWaitIntervalMsec = 500
};

template <class T>
inline void setStep(common::StepSequenceJob::Step& slot, common::Job::ExecuteResult (T::*function)(), const char* name)
{
    const common::StepSequenceJob::Step step(
        static_cast<common::Job::ExecuteResult (common::StepSequenceJob::*)()>(function), name);
    slot = step;
}
}

LanMatchBrowseMatchmakeJob::LanMatchBrowseMatchmakeJob()
    : session::BrowseMatchmakeJob(),
      m_callContext(),
      m_matchmakeSession(0)
{
}

LanMatchBrowseMatchmakeJob::~LanMatchBrowseMatchmakeJob()
{
}

void LanMatchBrowseMatchmakeJob::Cleanup()
{
    if (m_matchmakeSession != 0)
        m_matchmakeSession->UnregisterSearchCriteria();
    session::BrowseMatchmakeJob::Cleanup();
}

LanMatchBrowseMatchmakeJob::ExecuteResult LanMatchBrowseMatchmakeJob::BrowseMatchmake()
{
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        m_callerContext->SignalFailure(cResultCancelled);
        m_callerContext = 0;
        Cleanup();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    const Result result = m_matchmakeSession->BrowseMatchmakeSession(&m_callContext);
    if (result != 0)
    {
        if (m_callerContext != 0)
        {
            m_callerContext->SignalFailure(result);
            m_callerContext = 0;
        }
        return MakeExecuteResult(cExecuteKindFinished);
    }
    setStep(m_currentStep, &LanMatchBrowseMatchmakeJob::WaitBrowseMatchmake,
            "LanMatchBrowseMatchmakeJob::WaitBrowseMatchmake");
    return MakeExecuteResult(cExecuteKindWait, cWaitIntervalMsec);
}

LanMatchBrowseMatchmakeJob::ExecuteResult LanMatchBrowseMatchmakeJob::WaitBrowseMatchmake()
{
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        m_callerContext->SignalFailure(cResultCancelled);
        m_callerContext = 0;
        Cleanup();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (m_matchmakeSession->WaitBrowseMatchmakeSession())
    {
        m_matchmakeSession->UnregisterSearchCriteria();
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            m_callerContext->SignalFailure(m_callContext.m_result);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            setStep(m_currentStep, &session::BrowseMatchmakeJob::CompleteProcess,
                    "session::BrowseMatchmakeJob::CompleteProcess");
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

LanMatchBrowseMatchmakeJob::ExecuteResult LanMatchBrowseMatchmakeJob::WaitRequestSessionInfo()
{
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        m_callerContext->SignalFailure(cResultCancelled);
        m_callerContext = 0;
        Cleanup();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (m_matchmakeSession->WaitRequestSessionInfo())
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            m_callerContext->SignalFailure(m_callContext.m_result);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            setStep(m_currentStep, &session::BrowseMatchmakeJob::CompleteProcess,
                    "session::BrowseMatchmakeJob::CompleteProcess");
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

void LanMatchBrowseMatchmakeJob::Trace(unsigned long long) const
{
}

Result LanMatchBrowseMatchmakeJob::StartupImpl(session::IMatchmakeSession* matchmakeSession,
                                               const session::SessionSearchCriteria* criteria)
{
    m_matchmakeSession = static_cast<LanMatchmakeSession*>(matchmakeSession);
    if (m_matchmakeSession->RegisterSearchCriteria(criteria, cBrowseSearchCriteriaNum) == 0)
        return cResultInvalidArgument;
    setStep(m_currentStep, &LanMatchBrowseMatchmakeJob::BrowseMatchmake, "LanMatchBrowseMatchmakeJob::BrowseMatchmake");
    return 0;
}

Result LanMatchBrowseMatchmakeJob::StartupImpl(session::IMatchmakeSession* matchmakeSession, unsigned int sessionId)
{
    m_matchmakeSession = static_cast<LanMatchmakeSession*>(matchmakeSession);
    const Result result = m_matchmakeSession->RequestSessionInfo(&m_callContext, sessionId);
    if (result != 0)
        return result;
    setStep(m_currentStep, &LanMatchBrowseMatchmakeJob::WaitRequestSessionInfo,
            "LanMatchBrowseMatchmakeJob::WaitRequestSessionInfo");
    return 0;
}

}
}
}
