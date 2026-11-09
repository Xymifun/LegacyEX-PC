#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHRANDOMMATCHMAKEJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHRANDOMMATCHMAKEJOB_HPP

#include "nn/pia/common/Time.hpp"
#include "nn/pia/session/RandomMatchmakeJob.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class NexMatchmakeSession;

class NexMatchRandomMatchmakeJob : public session::RandomMatchmakeJob
{
public:
    NexMatchRandomMatchmakeJob();
    virtual ~NexMatchRandomMatchmakeJob();
    virtual void Trace(unsigned long long value) const;
    virtual void NoticeOwnerChangeEvent(unsigned int sessionId, unsigned long long ownerId);
    virtual void NoticeUnregisterEvent(unsigned int sessionId);
    virtual Result StartupImpl(const session::JoinRandomSessionSetting* setting);
    virtual void CleanupImpl();
    virtual void SetNextStepAtWaitCreateMesh();
    virtual void SetNextStepAtWaitJoinMesh();
    virtual ExecuteResult SetNextStepAtFailedCreateMesh(Result result);
    virtual ExecuteResult SetNextStepAtFailedJoinMesh(Result result);
    virtual ExecuteResult SetNextStepAtWaitLeaveMesh();

    ExecuteResult LeaveJoinedMatchmakeSession();
    ExecuteResult WaitLeaveJoinedMatchmakeSession();
    ExecuteResult RandomMatchmake();
    ExecuteResult WaitRandomMatchmake();
    ExecuteResult WaitNotification();
    ExecuteResult StartNatSession();
    ExecuteResult WaitStartNatSession();
    ExecuteResult LeaveMatchmakeSession();
    ExecuteResult WaitLeaveCurrentMatchmakeSession();
    ExecuteResult WaitLeaveBufferMatchmakeSession();
    ExecuteResult ChangeParticipation();
    ExecuteResult WaitChangeParticipation();
    ExecuteResult CleanupForRetryJoinMesh();
    ExecuteResult WaitForRetryJoinMesh();

private:
    enum
    {
        cJoinedSessionIdNumMax = 4
    };

    NexMatchmakeSession* m_matchmakeSession;
    unsigned long long m_ownerPrincipalId;
    common::Time m_retryTimeoutTime;
    common::Time m_retryTime;
    common::Time m_notificationTimeoutTime;
    unsigned int m_joinedSessionId[cJoinedSessionIdNumMax];
    unsigned int m_joinedSessionIdNum;
    bool m_waitingForRetryJoinMesh;
    bool m_isOpenSession;
    common::Time m_natStartTime;
    bool m_ngsInterfaceAvailable;
};

}
}
}

#endif
