#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHLEAVESESSIONJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHLEAVESESSIONJOB_HPP

#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/session/LeaveSessionJob.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class NexMatchLeaveSessionJob : public session::LeaveSessionJob
{
public:
    NexMatchLeaveSessionJob();
    virtual ~NexMatchLeaveSessionJob();
    virtual void Trace(unsigned long long value) const;
    ExecuteResult GetMatchmakeSessionOwners();
    ExecuteResult WaitGetMatchmakeSessionOwners();
    ExecuteResult MigrateMatchmakeSessionOwner();
    ExecuteResult WaitMigrateMatchmakeSessionOwner();
    ExecuteResult LeaveMatchmakeSessionCompanions();
    ExecuteResult WaitLeaveMatchmakeSessionCompanions();

    virtual void SetNextStepAtWaitForcedTerminatingOfJointSessionJob();
    virtual void SetNextStepAtMeshCleanup();
    virtual void SetNextStepAtWaitLeaveCurrentMatchmakeSession();
    virtual void SetNextStepAtLeaveCurrentMatchmakeSessionFailure();
    virtual bool IsNeedSkipLeaveCurrentMatchmakeSession();
    virtual void SetNextStepAtWaitHostMigratedTimeout();
    virtual void SetNextStepAtWaitLeaveBufferMatchmakeSession();
    virtual unsigned int GetWaitHostMigrationTimeout() const;
    virtual void CleanupImpl();

private:
    enum
    {
        cSessionIdMax = 32
    };

    void ChooseNextStepByCheckingBufferMatchmakeSessions(const common::StepSequenceJob::Step& step);

    unsigned int m_sessionIds[cSessionIdMax];
    unsigned char m_sessionIdNum;
    bool m_isSessionInfoRequested[cSessionIdMax];
};

}
}
}

#endif
