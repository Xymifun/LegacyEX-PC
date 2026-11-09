#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHJOINSESSIONJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHJOINSESSIONJOB_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/session/JoinSessionJob.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class NexMatchJoinSessionJob : public session::JoinSessionJob
{
public:
    NexMatchJoinSessionJob();
    virtual ~NexMatchJoinSessionJob();
    virtual void Trace(unsigned long long value) const;
    virtual void NoticeOwnerChangeEvent(unsigned int sessionId, unsigned long long ownerId);
    virtual void NoticeUnregisterEvent(unsigned int sessionId);
    ExecuteResult LeaveJoinedMatchmakeSession();
    ExecuteResult WaitLeaveJoinedMatchmakeSession();
    ExecuteResult JoinMatchmakeSession();
    ExecuteResult WaitJoinMatchmake();
    ExecuteResult WaitNotification();
    ExecuteResult StartNatSession();
    ExecuteResult WaitStartNatSession();
    ExecuteResult GetStationConnectionInfo();
    ExecuteResult WaitGetStationConnectionInfo();
    ExecuteResult LeaveMatchmakeSession();
    ExecuteResult WaitLeaveCurrentMatchmakeSession();
    ExecuteResult WaitLeaveBufferMatchmakeSession();
    ExecuteResult CleanupForRetryJoinMesh();
    ExecuteResult WaitForRetryJoinMesh();

protected:
    virtual Result StartupImpl(const session::JoinSessionSetting* setting);
    virtual void CleanupImpl();
    virtual void SetNextStepAtWaitJoinMesh();
    virtual ExecuteResult SetNextStepAtFailedJoinMesh(Result result);
    virtual ExecuteResult SetNextStepAtWaitLeaveMesh();
    virtual ExecuteResult SetNextStepAtMeshCleanup();

private:
    enum
    {
        cJoinedSessionIdMax = 4
    };

    unsigned long long m_ownerPrincipalId;
    common::Time m_retryTimeoutDeadline;
    common::Time m_retryWaitDeadline;
    common::Time m_notificationDeadline;
    unsigned int m_joinedSessionIds[cJoinedSessionIdMax];
    unsigned int m_joinedSessionIdNum;
    bool m_isWaitingForRetry;
    common::Time m_joinMatchmakeStartTime;
    bool m_isNgsInterfaceBound;
};

}
}
}

#endif
