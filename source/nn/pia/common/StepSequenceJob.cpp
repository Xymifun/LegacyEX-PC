#include "nn/pia/common/StepSequenceJob.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn
{
namespace pia
{
namespace common
{

void StepSequenceJob::Step::Trace(unsigned long long value) const
{
}

StepSequenceJob::StepSequenceJob() : Job(), m_currentStep(), m_cancelRequested(false), m_traceFlag(0)
{
}

StepSequenceJob::~StepSequenceJob()
{
}

void StepSequenceJob::Reset(bool isBackground)
{
    Job::Reset(isBackground);
    m_cancelRequested = false;
}

Job::ExecuteResult StepSequenceJob::ExecuteCore()
{
    for (;;)
    {
        if (m_cancelRequested)
        {
            CancelCleanup();
            m_cancelRequested = false;
            return MakeExecuteResult(cExecuteKindCancelled);
        }
        const ExecuteResult result = (this->*m_currentStep.m_function)();
        if (result.m_kind != cExecuteKindContinue)
            return result;
    }
}

void StepSequenceJob::WaitForCompletion(unsigned int intervalMsec)
{
    const long long ticks = static_cast<long long>(intervalMsec) * pead::TickSpan::cFrequency / 1000;

    for (;;)
    {
        if (IsForeground())
        {
            CancelCleanup();
            m_cancelRequested = false;
            Reset(false);
            return;
        }
        if (!IsBackground())
            break;
        pead::Thread::sleep(pead::TickSpan(ticks));
    }
    if (GetState() == cStateSuspended)
    {
        CancelCleanup();
        m_cancelRequested = false;
        Reset(false);
    }
}

void StepSequenceJob::CancelCleanup()
{
}

void StepSequenceJob::Trace(unsigned long long value) const
{
}

}
}
}
