#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_STEP_SEQUENCE_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_STEP_SEQUENCE_JOB_HPP

#include "nn/pia/common/Job.hpp"

namespace nn
{
namespace pia
{
namespace common
{

class StepSequenceJob : public Job
{
public:
    class Step
    {
    public:
        Step() : m_function(0), m_name(0) {}
        Step(ExecuteResult (StepSequenceJob::*function)(), const char* name) : m_function(function), m_name(name) {}

        virtual ~Step() {}
        virtual void Trace(unsigned long long value) const;

        ExecuteResult (StepSequenceJob::*m_function)();
        const char* m_name;
    };

    StepSequenceJob();
    virtual ~StepSequenceJob();
    virtual void Reset(bool isBackground);
    virtual ExecuteResult ExecuteCore();
    virtual void CancelCleanup();
    virtual void Trace(unsigned long long value) const;
    void WaitForCompletion(unsigned int intervalMsec);
    void SetCurrentStep(const Step& step) { m_currentStep = step; }

    Step m_currentStep;
    bool m_cancelRequested;
    unsigned long long m_traceFlag;
};

}
}
}

#endif
