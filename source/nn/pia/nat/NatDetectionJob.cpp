#include "nn/pia/nat/NatDetectionJob.hpp"

#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/nat/Api.hpp"
#include "nn/pia/nat/NatDetecter.hpp"
#include "nn/pia/nat/NatTraverser.hpp"

namespace nn
{
namespace pia
{
namespace nat
{

namespace
{
typedef common::Job::ExecuteResult ExecuteResult;
typedef common::StepSequenceJob::Step Step;
typedef ExecuteResult (common::StepSequenceJob::*StepFunction)();

inline void SelectStep(Step& slot, const Step& step)
{
    slot = step;
}

inline void SetDeadline(common::Time& deadline, int timeoutMsec)
{
    const common::TimeSpan span(common::TimeSpan::GetTicksPerMSec().m_tick * timeoutMsec);
    common::Time now;
    now.SetNow();
    deadline.m_tick = now.m_tick + span.m_tick;
}

enum
{
    cProbeSendIntervalMsec = 20,
    cReplyWaitIntervalMsec = 100,
    cRetryTimeoutStepMsec = 1000,
    cWaitPollIntervalMsec = 5
};
}

NatDetectionJob::NatDetectionJob() : common::StepSequenceJob(), m_endTime(), m_waitTime(), m_firstSendTime()
{
    m_detecter = 0;
}

NatDetectionJob::~NatDetectionJob()
{
}

Result NatDetectionJob::Startup(common::CallContext* context, NatDetecter* detecter, NatTraverser* traverser)
{
    m_endTime.m_tick = 0;
    m_detecter = detecter;
    SelectStep(m_currentStep,
               Step(static_cast<StepFunction>(&NatDetectionJob::StepPreTest), "NatDetectionJob::StepPreTest"));
    m_waitTime.m_tick = 0;
    m_traverser = traverser;
    common::StepSequenceJob::Reset(true);
    m_context = context;
    if (!m_context->InitiateCall())
    {
        return ResultInvalidState();
    }
    m_sendStarted = false;
    m_receivedFirstReply = false;
    return 0;
}

void NatDetectionJob::Cleanup()
{
    m_detecter->Cleanup();
    m_detecter = 0;
}

void NatDetectionJob::CancelCleanup()
{
    if (m_context != 0 && m_context->m_state == common::CallContext::cStateInProgress)
        m_context->SignalCancel();
    Cleanup();
}

NatDetectionJob::ExecuteResult NatDetectionJob::StepPreTest()
{
    if (m_traverser->IsStartupCancelled())
    {
        SelectStep(m_currentStep,
                   Step(static_cast<StepFunction>(&NatDetectionJob::StepComplete), "NatDetectionJob::StepComplete"));
        return MakeExecuteResult(cExecuteKindEntryForeground);
    }
    const Result result = m_detecter->OpenSocket();
    if (result != 0)
    {
        m_context->SignalFailure(result);
        Cleanup();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep,
               Step(static_cast<StepFunction>(&NatDetectionJob::StepStart), "NatDetectionJob::StepStart"));
    return MakeExecuteResult(cExecuteKindContinue);
}

NatDetectionJob::ExecuteResult NatDetectionJob::StepStart()
{
    if (m_traverser->IsStartupCancelled())
    {
        SelectStep(m_currentStep,
                   Step(static_cast<StepFunction>(&NatDetectionJob::StepEnd), "NatDetectionJob::StepEnd"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    NatDetecter* detecter = m_detecter;
    const common::TimeSpan timeout = common::TimeSpan::FromMSec(detecter->GetDetectionTimeout()
                                                                + detecter->m_retryCount * cRetryTimeoutStepMsec);
    common::Time now;
    now.SetNow();
    now.m_tick += timeout.m_tick;
    m_endTime = now;
    m_detecter->InitializeNatCheckReply();
    m_detecter->StartSendingNatCheckProbe();
    SelectStep(m_currentStep, Step(static_cast<StepFunction>(&NatDetectionJob::StepSend), "NatDetectionJob::StepSend"));
    return MakeExecuteResult(cExecuteKindContinue);
}

void NatDetectionJob::StopReceivingMessage()
{
    if (m_detecter->CheckRetry() != 0)
    {
        m_detecter->Retry();
        SelectStep(m_currentStep,
                   Step(static_cast<StepFunction>(&NatDetectionJob::StepStart), "NatDetectionJob::StepStart"));
    }
    else
    {
        SelectStep(m_currentStep,
                   Step(static_cast<StepFunction>(&NatDetectionJob::StepEnd), "NatDetectionJob::StepEnd"));
    }
}

NatDetectionJob::ExecuteResult NatDetectionJob::StepSend()
{
    if (m_traverser->IsStartupCancelled())
    {
        SelectStep(m_currentStep,
                   Step(static_cast<StepFunction>(&NatDetectionJob::StepEnd), "NatDetectionJob::StepEnd"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    common::Time now;
    now.SetNow();
    if (now.m_tick > m_endTime.m_tick)
    {
        StopReceivingMessage();
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (!m_sendStarted)
    {
        m_sendStarted = true;
        m_firstSendTime.SetNow();
    }
    if (m_detecter->SendAllQueuedProbe() == 0)
    {
        SetDeadline(m_waitTime, cProbeSendIntervalMsec);
        SelectStep(m_currentStep,
                   Step(static_cast<StepFunction>(&NatDetectionJob::StepWait), "NatDetectionJob::StepWait"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_detecter->ReceiveNatCheckReply() == 0)
    {
        SetDeadline(m_waitTime, cReplyWaitIntervalMsec);
        SelectStep(m_currentStep,
                   Step(static_cast<StepFunction>(&NatDetectionJob::StepWait), "NatDetectionJob::StepWait"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (!m_receivedFirstReply)
    {
        m_receivedFirstReply = true;
        common::Time current;
        current.SetNow();
        const common::TimeSpan elapsed(current.m_tick - m_firstSendTime.m_tick);
        m_detecter->m_firstReplyMsec = static_cast<unsigned int>(elapsed.m_tick
                                                                 / common::TimeSpan::GetTicksPerMSec().m_tick);
    }
    if (m_detecter->IsReceivedAllMessage())
    {
        StopReceivingMessage();
        return MakeExecuteResult(cExecuteKindContinue);
    }
    SetDeadline(m_waitTime, cReplyWaitIntervalMsec);
    SelectStep(m_currentStep, Step(static_cast<StepFunction>(&NatDetectionJob::StepWait), "NatDetectionJob::StepWait"));
    return MakeExecuteResult(cExecuteKindContinue);
}

NatDetectionJob::ExecuteResult NatDetectionJob::StepWait()
{
    if (m_traverser->IsStartupCancelled())
    {
        SelectStep(m_currentStep,
                   Step(static_cast<StepFunction>(&NatDetectionJob::StepEnd), "NatDetectionJob::StepEnd"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    common::Time now;
    now.SetNow();
    if (now.m_tick > m_waitTime.m_tick)
    {
        SelectStep(m_currentStep,
                   Step(static_cast<StepFunction>(&NatDetectionJob::StepSend), "NatDetectionJob::StepSend"));
        return MakeExecuteResult(cExecuteKindContinue);
    }
    return MakeExecuteResult(cExecuteKindWait, cWaitPollIntervalMsec);
}

NatDetectionJob::ExecuteResult NatDetectionJob::StepEnd()
{
    const Result result = m_detecter->CloseSocket();
    m_closeSocketFailed = (result != 0);
    SelectStep(m_currentStep,
               Step(static_cast<StepFunction>(&NatDetectionJob::StepComplete), "NatDetectionJob::StepComplete"));
    return MakeExecuteResult(cExecuteKindEntryForeground);
}

NatDetectionJob::ExecuteResult NatDetectionJob::StepComplete()
{
    ExecuteResult result;
    if (m_traverser->IsStartupCancelled())
    {
        m_context->SignalFailure(cResultCancelled);
        Cleanup();
        result = MakeExecuteResult(cExecuteKindFinished);
    }
    else if (m_closeSocketFailed)
    {
        m_context->SignalFailure(cResultPortDetectionFailed);
        Cleanup();
        result = MakeExecuteResult(cExecuteKindFinished);
    }
    else if (m_detecter->HandleResult() == 0)
    {
        m_context->SignalFailure(cResultPortDetectionFailed);
        Cleanup();
        result = MakeExecuteResult(cExecuteKindFinished);
    }
    else if (GetDebugSetting()->options[2] != 0)
    {
        m_context->SignalFailure(cResultPortDetectionFailed);
        Cleanup();
        result = MakeExecuteResult(cExecuteKindFinished);
    }
    else
    {
        m_context->SignalSuccess(0);
        Cleanup();
        result = MakeExecuteResult(cExecuteKindFinished);
    }
    return result;
}

}
}
}
