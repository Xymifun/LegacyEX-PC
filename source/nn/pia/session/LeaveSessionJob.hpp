#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_LEAVE_SESSION_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_LEAVE_SESSION_JOB_HPP

#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/session/Session.hpp"

namespace nn
{
namespace pia
{
namespace session
{

class LeaveSessionJob : public common::StepSequenceJob
{
public:
    LeaveSessionJob();
    virtual ~LeaveSessionJob();
    Result Startup(common::CallContext* callerContext);
    ExecuteResult WaitForcedTerminatingOfJointSessionJob();
    ExecuteResult LeaveMesh();
    ExecuteResult WaitLeaveMesh();
    ExecuteResult WaitLeaveMeshWithHostMigration();
    ExecuteResult WaitHostMigrated();
    ExecuteResult LeaveCurrentMatchmakeSession();
    ExecuteResult WaitLeaveCurrentMatchmakeSession();
    void CalcProcessingTimeOfLeave() const;
    ExecuteResult SendMonitoringData();
    ExecuteResult MeshCleanup();
    ExecuteResult CompleteProcess();
    ExecuteResult LeaveBufferMatchmakeSession();
    ExecuteResult WaitLeaveBufferMatchmakeSession();
    virtual void SetNextStepAtWaitForcedTerminatingOfJointSessionJob();
    virtual void SetNextStepAtWaitLeaveMesh();
    virtual void SetNextStepAtMeshCleanup();
    virtual void SetNextStepAtWaitLeaveCurrentMatchmakeSession();
    virtual void SetNextStepAtLeaveCurrentMatchmakeSessionFailure();
    virtual bool IsNeedSkipLeaveCurrentMatchmakeSession() = 0;
    virtual void SetNextStepAtWaitHostMigratedTimeout();
    virtual void SetNextStepAtWaitLeaveBufferMatchmakeSession();
    void Cleanup();
    virtual void Trace(unsigned long long value) const;
    virtual unsigned int GetWaitHostMigrationTimeout() const { return 0; }
    virtual void ChangeStatusToDisconnectedNetwork() { g_piaSession->ChangeStatusToDisconnectedNetwork(); }
    virtual void CleanupImpl() {}

protected:
    common::CallContext* m_callerContext;
    common::CallContext m_callContext;
    common::Time m_hostMigrationTimeoutTime;
    bool m_isSingleStation;
    common::Time m_startTime;

    void setStep(ExecuteResult (LeaveSessionJob::*function)(), const char* name)
    {
        Step step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(function), name);
        m_currentStep.m_function = step.m_function;
        m_currentStep.m_name = step.m_name;
    }
};

}
}
}

#endif
