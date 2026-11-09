#include "nn/pia/inet/NexMatchUpdateSessionSettingJob.hpp"

#include <cstring>

#include "nn/pia/Transport.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/inet/NexMatchmakeSession.hpp"
#include "nn/pia/inet/NexUpdateSessionSetting.hpp"
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

enum
{
    cResultCancelled = 0x6c05
};

inline void SelectStep(common::StepSequenceJob::Step& slot,
                       ExecuteResult (NexMatchUpdateSessionSettingJob::*function)(), const char* name)
{
    const common::StepSequenceJob::Step step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(function), name);
    slot = step;
}
}

NexMatchUpdateSessionSettingJob::NexMatchUpdateSessionSettingJob()
    : session::UpdateSessionSettingJob(),
      m_matchmakeSession(0)
{
}

NexMatchUpdateSessionSettingJob::~NexMatchUpdateSessionSettingJob()
{
}

Result NexMatchUpdateSessionSettingJob::StartupImpl(unsigned int sessionId,
                                                    session::IMatchmakeSession* matchmakeSession,
                                                    const session::UpdateSessionSetting* setting, bool isJointSession)
{
    if (sessionId == 0)
        return transport::cResultInvalidArgument;
    if (matchmakeSession == 0)
        return transport::cResultInvalidArgument;
    if (setting == 0)
        return transport::cResultInvalidArgument;
    m_matchmakeSession = static_cast<NexMatchmakeSession*>(matchmakeSession);
    m_isJointSession = isJointSession;
    const NexUpdateSessionSetting* nexSetting = static_cast<const NexUpdateSessionSetting*>(setting);
    if (!nexSetting->IsValid() && !m_isJointSession)
        return transport::cResultInvalidArgument;
    if (nexSetting->IsOnlyApplicationDataUpdated() && !m_isJointSession)
    {
        std::memset(m_applicationData, 0, sizeof(m_applicationData));
        nexSetting->GetApplicationData(m_applicationData, sizeof(m_applicationData));
        m_applicationDataSize = nexSetting->GetApplicationDataSize();
        SelectStep(m_currentStep, &NexMatchUpdateSessionSettingJob::UpdateApplicationData,
                   "NexMatchUpdateSessionSettingJob::UpdateApplicationData");
    }
    else if (nexSetting->IsOnlyAllAttributesUpdated() && !m_isJointSession)
    {
        for (unsigned int index = 0; index < cAttributeNum; ++index)
            m_attributes[index] = nexSetting->GetAttribute(index);
        SelectStep(m_currentStep, &NexMatchUpdateSessionSettingJob::UpdateAttributes,
                   "NexMatchUpdateSessionSettingJob::UpdateAttributes");
    }
    else if (nexSetting->IsOnlyAttributeUpdated() && !m_isJointSession)
    {
        for (unsigned int index = 0; index < cAttributeNum; ++index)
        {
            m_attributes[index] = nexSetting->GetAttribute(index);
            m_isAttributeUpdated[index] = nexSetting->IsAttributeUpdated(index);
        }
        SelectStep(m_currentStep, &NexMatchUpdateSessionSettingJob::ModifyAttribute,
                   "NexMatchUpdateSessionSettingJob::ModifyAttribute");
    }
    else
    {
        if (m_isJointSession)
        {
            if (sessionId == session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex])
            {
                if (session::g_piaSession->GetStatus() != session::Session::cStatusMatched)
                    return transport::cResultInvalidState;
            }
            if (sessionId == session::g_piaSession->m_sessionId[session::g_piaSession->m_currentSessionIndex == 0])
            {
                if (session::g_piaSession->GetStatus() != session::Session::cStatusBufferMatched)
                    return transport::cResultInvalidState;
            }
        }
        m_matchmakeSession->ReflectUpdateSetting(setting, m_isJointSession);
        SelectStep(m_currentStep, &NexMatchUpdateSessionSettingJob::UpdateSessionSetting,
                   "NexMatchUpdateSessionSettingJob::UpdateSessionSetting");
    }
    return 0;
}

NexMatchUpdateSessionSettingJob::ExecuteResult NexMatchUpdateSessionSettingJob::UpdateSessionSetting()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
    {
        const Result result = session::Session::cResultStatusDisconnectedNetwork;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown6:
    {
        const Result result = session::Session::cResultStatusUnknown6;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown7:
    {
        const Result result = session::Session::cResultStatusUnknown7;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown8:
    {
        const Result result = session::Session::cResultStatusUnknown8;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    default:
        break;
    }
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        const Result result = cResultCancelled;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    const Result result = m_matchmakeSession->UpdateSessionSetting(&m_callContext, m_sessionId);
    if (result != 0)
    {
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &NexMatchUpdateSessionSettingJob::WaitUpdateSessionSetting,
               "NexMatchUpdateSessionSettingJob::WaitUpdateSessionSetting");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchUpdateSessionSettingJob::ExecuteResult NexMatchUpdateSessionSettingJob::WaitUpdateSessionSetting()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
    {
        const Result result = session::Session::cResultStatusDisconnectedNetwork;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown6:
    {
        const Result result = session::Session::cResultStatusUnknown6;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown7:
    {
        const Result result = session::Session::cResultStatusUnknown7;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown8:
    {
        const Result result = session::Session::cResultStatusUnknown8;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    default:
        break;
    }
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        const Result result = cResultCancelled;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
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
            if (m_isJointSession)
            {
                session::Session* piaSession = session::g_piaSession;
                if (m_sessionId == piaSession->m_sessionId[piaSession->m_currentSessionIndex])
                    SetCurrentSessionOpenStatus(session::Session::cSessionOpenStatusOpen);
                else if (m_sessionId == piaSession->m_sessionId[piaSession->m_currentSessionIndex == 0])
                    SetBufferSessionOpenStatus(session::Session::cSessionOpenStatusOpen);
            }
            SelectStep(m_currentStep, &NexMatchUpdateSessionSettingJob::CompleteProcess,
                       "UpdateSessionSettingJob::CompleteProcess");
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchUpdateSessionSettingJob::ExecuteResult NexMatchUpdateSessionSettingJob::ModifyAttribute()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
    {
        const Result result = session::Session::cResultStatusDisconnectedNetwork;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown6:
    {
        const Result result = session::Session::cResultStatusUnknown6;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown7:
    {
        const Result result = session::Session::cResultStatusUnknown7;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown8:
    {
        const Result result = session::Session::cResultStatusUnknown8;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    default:
        break;
    }
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        const Result result = cResultCancelled;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    unsigned int index;
    for (index = 0; index < cAttributeNum; ++index)
    {
        if (m_isAttributeUpdated[index])
            break;
    }
    if (index < cAttributeNum)
    {
        m_isAttributeUpdated[index] = false;
        const Result result = m_matchmakeSession->ModifyAttribute(&m_callContext, m_sessionId, index,
                                                                  m_attributes[index]);
        if (result != 0)
        {
            m_callerContext->SignalFailure(result);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
        SelectStep(m_currentStep, &NexMatchUpdateSessionSettingJob::WaitModifyAttribute,
                   "NexMatchUpdateSessionSettingJob::WaitModifyAttribute");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    SelectStep(m_currentStep, &NexMatchUpdateSessionSettingJob::CompleteProcess,
               "UpdateSessionSettingJob::CompleteProcess");
    return MakeExecuteResult(cExecuteKindContinue);
}

NexMatchUpdateSessionSettingJob::ExecuteResult NexMatchUpdateSessionSettingJob::WaitModifyAttribute()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
    {
        const Result result = session::Session::cResultStatusDisconnectedNetwork;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown6:
    {
        const Result result = session::Session::cResultStatusUnknown6;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown7:
    {
        const Result result = session::Session::cResultStatusUnknown7;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown8:
    {
        const Result result = session::Session::cResultStatusUnknown8;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    default:
        break;
    }
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        const Result result = cResultCancelled;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (m_matchmakeSession->WaitModifyAttribute())
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            m_callerContext->SignalFailure(m_callContext.m_result);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            SelectStep(m_currentStep, &NexMatchUpdateSessionSettingJob::CompleteProcess,
                       "UpdateSessionSettingJob::CompleteProcess");
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchUpdateSessionSettingJob::ExecuteResult NexMatchUpdateSessionSettingJob::UpdateAttributes()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
    {
        const Result result = session::Session::cResultStatusDisconnectedNetwork;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown6:
    {
        const Result result = session::Session::cResultStatusUnknown6;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown7:
    {
        const Result result = session::Session::cResultStatusUnknown7;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown8:
    {
        const Result result = session::Session::cResultStatusUnknown8;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    default:
        break;
    }
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        const Result result = cResultCancelled;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    const Result result = m_matchmakeSession->UpdateAttributes(&m_callContext, m_sessionId, m_attributes);
    if (result != 0)
    {
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &NexMatchUpdateSessionSettingJob::WaitUpdateAttributes,
               "NexMatchUpdateSessionSettingJob::WaitUpdateAttributes");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchUpdateSessionSettingJob::ExecuteResult NexMatchUpdateSessionSettingJob::WaitUpdateAttributes()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
    {
        const Result result = session::Session::cResultStatusDisconnectedNetwork;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown6:
    {
        const Result result = session::Session::cResultStatusUnknown6;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown7:
    {
        const Result result = session::Session::cResultStatusUnknown7;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown8:
    {
        const Result result = session::Session::cResultStatusUnknown8;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    default:
        break;
    }
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        const Result result = cResultCancelled;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (m_matchmakeSession->WaitUpdateAttributes())
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            m_callerContext->SignalFailure(m_callContext.m_result);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            SelectStep(m_currentStep, &NexMatchUpdateSessionSettingJob::CompleteProcess,
                       "UpdateSessionSettingJob::CompleteProcess");
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchUpdateSessionSettingJob::ExecuteResult NexMatchUpdateSessionSettingJob::UpdateApplicationData()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
    {
        const Result result = session::Session::cResultStatusDisconnectedNetwork;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown6:
    {
        const Result result = session::Session::cResultStatusUnknown6;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown7:
    {
        const Result result = session::Session::cResultStatusUnknown7;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown8:
    {
        const Result result = session::Session::cResultStatusUnknown8;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    default:
        break;
    }
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        const Result result = cResultCancelled;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    const Result result = m_matchmakeSession->UpdateApplicationBuffer(&m_callContext, m_sessionId, m_applicationData,
                                                                      m_applicationDataSize);
    if (result != 0)
    {
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &NexMatchUpdateSessionSettingJob::WaitUpdateApplicationData,
               "NexMatchUpdateSessionSettingJob::WaitUpdateApplicationData");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexMatchUpdateSessionSettingJob::ExecuteResult NexMatchUpdateSessionSettingJob::WaitUpdateApplicationData()
{
    unsigned int status = session::g_piaSession->GetStatus();
    switch (status)
    {
    case session::Session::cStatusDisconnectedNetwork:
    {
        const Result result = session::Session::cResultStatusDisconnectedNetwork;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown6:
    {
        const Result result = session::Session::cResultStatusUnknown6;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown7:
    {
        const Result result = session::Session::cResultStatusUnknown7;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    case session::Session::cStatusUnknown8:
    {
        const Result result = session::Session::cResultStatusUnknown8;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    default:
        break;
    }
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        const Result result = cResultCancelled;
        m_callerContext->SignalFailure(result);
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (m_matchmakeSession->WaitUpdateApplicationBuffer())
    {
        if (m_callContext.m_state == common::CallContext::cStateFailure)
        {
            m_callerContext->SignalFailure(m_callContext.m_result);
            m_callerContext = 0;
            return MakeExecuteResult(cExecuteKindFinished);
        }
        if (m_callContext.m_state == common::CallContext::cStateSuccess)
        {
            SelectStep(m_currentStep, &NexMatchUpdateSessionSettingJob::CompleteProcess,
                       "UpdateSessionSettingJob::CompleteProcess");
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

void NexMatchUpdateSessionSettingJob::Cleanup()
{
    session::UpdateSessionSettingJob::Cleanup();
    m_matchmakeSession = 0;
    std::memset(m_attributes, 0, sizeof(m_attributes));
    std::memset(m_applicationData, 0, sizeof(m_applicationData));
    m_applicationDataSize = 0;
}

void NexMatchUpdateSessionSettingJob::Trace(unsigned long long) const
{
}

}
}
}
