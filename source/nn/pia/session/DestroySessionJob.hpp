#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_DESTROY_SESSION_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_DESTROY_SESSION_JOB_HPP

#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/common/Time.hpp"

namespace nn
{
namespace pia
{
namespace session
{

class DestroySessionJob : public common::StepSequenceJob
{
public:
    DestroySessionJob();
    virtual ~DestroySessionJob();
    Result Startup(common::CallContext* callerContext);
    ExecuteResult WaitForcedTerminatingOfJointSessionJob();
    ExecuteResult DestroyMesh();
    ExecuteResult WaitDestroyMesh();
    ExecuteResult MeshCleanup();
    void CalcProcessingTimeOfLeave() const;
    ExecuteResult SendMonitoringData();
    ExecuteResult CompleteProcess();
    virtual void Cleanup();
    virtual void Trace(unsigned long long value) const;

protected:
    virtual Result StartupImpl() = 0;
    virtual void SetNextStepAtWaitDestroyMesh() = 0;
    virtual void SetNextStepAtMeshCleanup() = 0;

    void setStep(ExecuteResult (DestroySessionJob::*function)(), const char* name)
    {
        Step step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(function), name);
        m_currentStep.m_function = step.m_function;
        m_currentStep.m_name = step.m_name;
    }

    common::CallContext* m_callerContext;
    common::CallContext m_callContext;
    common::Time m_startTime;
    bool m_hasWaitedJointSessionJob;
};

}
}
}

#endif
