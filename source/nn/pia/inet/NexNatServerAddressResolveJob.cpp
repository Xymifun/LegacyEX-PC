#include "nn/pia/inet/NexNatServerAddressResolveJob.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/inet/Api.hpp"
#include "nn/pia/inet/NexFacade.hpp"
#include "nn/pia/inet/NexNatRelay.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

namespace
{
enum
{
    cResultNatServerAddressUnavailable = 0x6436
};

typedef common::Job::ExecuteResult ExecuteResult;

inline void SelectStep(common::StepSequenceJob::Step& slot, ExecuteResult (NexNatServerAddressResolveJob::*function)(),
                       const char* name)
{
    const common::StepSequenceJob::Step step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(function), name);
    slot = step;
}

inline void SetDeadline(common::Time& deadline, unsigned int milliseconds)
{
    const common::TimeSpan span = common::TimeSpan::FromMSec(milliseconds);
    deadline = common::g_piaJobScheduler->m_dispatchTime + span;
}
}

NexNatServerAddressResolveJob::NexNatServerAddressResolveJob() : common::StepSequenceJob(), m_deadline(), m_result(0)
{
    m_callContext = 0;
}

NexNatServerAddressResolveJob::~NexNatServerAddressResolveJob()
{
}

Result NexNatServerAddressResolveJob::Startup(common::CallContext* callContext)
{
    if (callContext == 0)
        return ResultInvalidArgument();
    if (!callContext->InitiateCall())
        return ResultInvalidState();
    m_callContext = callContext;
    m_deadline = common::g_piaJobScheduler->m_dispatchTime;
    m_result = 0;
    SelectStep(m_currentStep, &NexNatServerAddressResolveJob::StepResolveNatServerAddress,
               "NexNatServerAddressResolveJob::StepResolveNatServerAddress");
    common::StepSequenceJob::Reset(true);
    return Result(0);
}

void NexNatServerAddressResolveJob::Cleanup()
{
    m_callContext = 0;
}

void NexNatServerAddressResolveJob::CancelCleanup()
{
    if (m_callContext != 0 && m_callContext->m_state == common::CallContext::cStateInProgress)
        m_callContext->SignalCancel();
    Cleanup();
}

NexNatServerAddressResolveJob::ExecuteResult NexNatServerAddressResolveJob::StepResolveNatServerAddress()
{
    if (m_callContext != 0 && m_callContext->m_cancelRequested)
    {
        SelectStep(m_currentStep, &NexNatServerAddressResolveJob::StepComplete,
                   "NexNatServerAddressResolveJob::StepComplete");
        return MakeExecuteResult(cExecuteKindEntryForeground);
    }
    if (GetDebugSetting()->options[3] != 0)
    {
        SelectStep(m_currentStep, &NexNatServerAddressResolveJob::StepComplete,
                   "NexNatServerAddressResolveJob::StepComplete");
        m_result = cResultNatServerAddressUnavailable;
        return MakeExecuteResult(cExecuteKindEntryForeground);
    }
    SetDeadline(m_deadline, 30000);
    m_result = NexFacade::s_instance->UpdateNatServerAddress();
    if (m_result != 0)
    {
        SelectStep(m_currentStep, &NexNatServerAddressResolveJob::StepComplete,
                   "NexNatServerAddressResolveJob::StepComplete");
        return MakeExecuteResult(cExecuteKindEntryForeground);
    }
    SelectStep(m_currentStep, &NexNatServerAddressResolveJob::StepRequestRelayServerSetting,
               "NexNatServerAddressResolveJob::StepRequestRelayServerSetting");
    return MakeExecuteResult(cExecuteKindEntryForeground);
}

NexNatServerAddressResolveJob::ExecuteResult NexNatServerAddressResolveJob::StepRequestRelayServerSetting()
{
    if (m_callContext != 0 && m_callContext->m_cancelRequested)
    {
        SelectStep(m_currentStep, &NexNatServerAddressResolveJob::StepComplete,
                   "NexNatServerAddressResolveJob::StepComplete");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    SetDeadline(m_deadline, 10000);
    NexFacade::s_instance->m_natRelay->RequestRelayServerSettingAsync();
    SelectStep(m_currentStep, &NexNatServerAddressResolveJob::StepWaitRequestRelayServerSetting,
               "NexNatServerAddressResolveJob::StepWaitRequestRelayServerSetting");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexNatServerAddressResolveJob::ExecuteResult NexNatServerAddressResolveJob::StepWaitRequestRelayServerSetting()
{
    if (NexFacade::s_instance->IsRelayServiceEnabled() && !(m_deadline < common::g_piaJobScheduler->m_dispatchTime))
    {
        if (NexFacade::s_instance->m_natRelay->IsRequestRelayServerSettingCompleted())
        {
            const Result result = NexFacade::s_instance->m_natRelay->GetRequestRelayServerSettingResult();
            if (result == 0 && m_callContext != 0 && !m_callContext->m_cancelRequested)
            {
                SelectStep(m_currentStep, &NexNatServerAddressResolveJob::StepResolveRelayServerSetting,
                           "NexNatServerAddressResolveJob::StepResolveRelayServerSetting");
                return MakeExecuteResult(cExecuteKindEntryBackground);
            }
        }
        else
        {
            return MakeExecuteResult(cExecuteKindNextStep);
        }
    }
    SelectStep(m_currentStep, &NexNatServerAddressResolveJob::StepComplete,
               "NexNatServerAddressResolveJob::StepComplete");
    return MakeExecuteResult(cExecuteKindNextStep);
}

NexNatServerAddressResolveJob::ExecuteResult NexNatServerAddressResolveJob::StepResolveRelayServerSetting()
{
    if (m_callContext != 0 && m_callContext->m_cancelRequested)
    {
        SelectStep(m_currentStep, &NexNatServerAddressResolveJob::StepComplete,
                   "NexNatServerAddressResolveJob::StepComplete");
        return MakeExecuteResult(cExecuteKindEntryForeground);
    }
    if (NexFacade::s_instance->IsRelayServiceEnabled())
    {
        NexFacade::s_instance->m_natRelay->ResolveRelayServerSetting();
        NexFacade::s_instance->m_natRelay->IsRelayServerAvailable();
    }
    SelectStep(m_currentStep, &NexNatServerAddressResolveJob::StepComplete,
               "NexNatServerAddressResolveJob::StepComplete");
    return MakeExecuteResult(cExecuteKindEntryForeground);
}

NexNatServerAddressResolveJob::ExecuteResult NexNatServerAddressResolveJob::StepComplete()
{
    if (m_callContext != 0)
    {
        if (m_callContext->m_cancelRequested)
        {
            m_callContext->SignalCancel();
        }
        else if (m_result == 0)
        {
            m_callContext->SignalSuccess(0);
        }
        else
        {
            m_callContext->SignalFailure(m_result);
        }
    }
    Cleanup();
    return MakeExecuteResult(cExecuteKindFinished);
}

}
}
}
