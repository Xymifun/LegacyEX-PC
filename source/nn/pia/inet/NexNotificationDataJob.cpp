#include "nn/pia/inet/NexNotificationDataJob.hpp"
#include "nn/nex/ErrorCodeConverter.hpp"
#include "nn/nex/Reconstruction.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qList.hpp"
#include "nn/pia/String.hpp"
#include "nn/pia/inet/NexFacade.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

namespace
{
typedef common::Job::ExecuteResult (NexNotificationDataJob::*Step)();

enum
{
    cFirstNotificationType = 0x65,
    cNotificationTypeCount = 8,
    cMaxTextLength = 0x100,
    cResultCancelled = 0x6c05,
    cResultInvalidArgument = 0x10c07,
    cResultInvalidState = 0x10c08,
    cResultRequestFailed = 0xac5a,
    cResultNexError = 0xe437,
    cNexResultInvalidArgument = static_cast<int>(0x8001000Au)
};

inline void SelectStep(common::StepSequenceJob::Step& slot, Step function, const char* name)
{
    const common::StepSequenceJob::Step step(
        static_cast<common::Job::ExecuteResult (common::StepSequenceJob::*)()>(function), name);
    slot = step;
}
}

NexNotificationDataJob::NexNotificationDataJob() : common::StepSequenceJob(), m_callContext(0), m_protocolCallContext(0)
{
}

NexNotificationDataJob::~NexNotificationDataJob()
{
}

Result NexNotificationDataJob::Startup(common::CallContext* callContext, unsigned int type, unsigned int param1,
                                       unsigned int param2, const common::String& text)
{
    const void* buffer = text.GetBuffer();
    if (type - cFirstNotificationType >= cNotificationTypeCount)
        return ResultInvalidArgument();
    if (text.GetStringLength() > cMaxTextLength)
        return ResultInvalidArgument();
    if (text.GetStringLength() > 0 && text.GetEncodingType() != common::String::EncodingType_Utf16)
        return ResultInvalidArgument();
    if (m_protocolCallContext.GetState() == nex::CallContext::CallInProgress)
        return ResultInvalidState();
    m_protocolCallContext.Reset();
    if (!NexFacade::s_instance->m_matchmakeExtensionClient->UpdateNotificationData(
            &m_protocolCallContext, type, param1, param2, nex::String(static_cast<const wchar_t*>(buffer))))
        return cResultRequestFailed;
    m_callContext = callContext;
    callContext->InitiateCall();
    Reset(true);
    SelectStep(m_currentStep, &NexNotificationDataJob::WaitUpdateNotificationData,
               "NexNotificationDataJob::WaitUpdateNotificationData");
    return 0;
}

Result NexNotificationDataJob::Startup(common::CallContext* callContext, unsigned char typeMask,
                                       nex::qList<nex::NotificationEvent>* events)
{
    nex::qList<unsigned int> types;
    if (m_protocolCallContext.GetState() == nex::CallContext::CallInProgress)
        return ResultInvalidState();
    for (unsigned int index = 0; index < cNotificationTypeCount; ++index)
    {
        if ((typeMask & (1 << index)) != 0)
            types.push_back(cFirstNotificationType + index);
    }
    m_protocolCallContext.Reset();
    if (!NexFacade::s_instance->m_matchmakeExtensionClient->GetFriendNotificationData(&m_protocolCallContext, events,
                                                                                      types))
        return cResultRequestFailed;
    m_callContext = callContext;
    callContext->InitiateCall();
    Reset(true);
    SelectStep(m_currentStep, &NexNotificationDataJob::WaitRetrieveNotificationData,
               "NexNotificationDataJob::WaitRetrieveNotificationData");
    return 0;
}

void NexNotificationDataJob::Cleanup()
{
    if (m_protocolCallContext.GetState() == nex::CallContext::CallInProgress)
        m_protocolCallContext.Cancel(nex::CallContext::CallCanceled);
    m_protocolCallContext.Reset();
    if (m_callContext != 0 && m_callContext->m_state == common::CallContext::cStateInProgress)
    {
        m_callContext->SignalCancel();
        m_callContext = 0;
    }
}

NexNotificationDataJob::ExecuteResult NexNotificationDataJob::WaitUpdateNotificationData()
{
    if (m_callContext != 0 && m_callContext->m_cancelRequested)
    {
        m_callContext->SignalFailure(cResultCancelled);
        m_callContext = 0;
        Cleanup();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (m_protocolCallContext.GetState() == nex::CallContext::CallFailed)
    {
        nex::qResult result = m_protocolCallContext.GetResult();
        if (result.GetReturnCode() == static_cast<unsigned int>(cNexResultInvalidArgument))
        {
            m_callContext->SignalFailure(cResultInvalidArgument);
        }
        else
        {
            Result::SetNexNetworkErrorCode(nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
            m_callContext->SignalFailure(cResultNexError);
        }
        m_callContext = 0;
        m_protocolCallContext.Reset();
        Cleanup();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (m_protocolCallContext.GetState() == nex::CallContext::CallSucceeded)
    {
        m_callContext->SignalSuccess(0);
        m_callContext = 0;
        m_protocolCallContext.Reset();
        Cleanup();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexNotificationDataJob::ExecuteResult NexNotificationDataJob::WaitRetrieveNotificationData()
{
    if (m_callContext != 0 && m_callContext->m_cancelRequested)
    {
        m_callContext->SignalFailure(cResultCancelled);
        m_callContext = 0;
        Cleanup();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (m_protocolCallContext.GetState() == nex::CallContext::CallFailed)
    {
        nex::qResult result = m_protocolCallContext.GetResult();
        if (result.GetReturnCode() == static_cast<unsigned int>(cNexResultInvalidArgument))
        {
            m_callContext->SignalFailure(cResultInvalidArgument);
        }
        else
        {
            Result::SetNexNetworkErrorCode(nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
            m_callContext->SignalFailure(cResultNexError);
        }
        m_callContext = 0;
        m_protocolCallContext.Reset();
        Cleanup();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (m_protocolCallContext.GetState() == nex::CallContext::CallSucceeded)
    {
        m_callContext->SignalSuccess(0);
        m_callContext = 0;
        m_protocolCallContext.Reset();
        Cleanup();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

}
}
}
