#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHJOINSESSIONJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHJOINSESSIONJOB_HPP

#include "nn/pia/common/Time.hpp"
#include "nn/pia/session/JoinSessionJob.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

class LanMatchmakeSession;

class LanMatchJoinSessionJob : public session::JoinSessionJob
{
public:
    LanMatchJoinSessionJob();
    virtual ~LanMatchJoinSessionJob();
    virtual void Trace(unsigned long long value) const;
    virtual void NoticeOwnerChangeEvent(unsigned int sessionId, unsigned long long ownerId);
    virtual void Cleanup();
    virtual Result StartupImpl(const session::JoinSessionSetting* setting);
    virtual void SetNextStepAtWaitJoinMesh();
    virtual ExecuteResult SetNextStepAtFailedJoinMesh(Result result);
    virtual ExecuteResult SetNextStepAtWaitLeaveMesh();
    virtual ExecuteResult SetNextStepAtMeshCleanup();

    ExecuteResult JoinMatchmakeSession();
    ExecuteResult WaitJoinMatchmake();
    ExecuteResult GetStationConnectionInfo();
    ExecuteResult WaitGetStationConnectionInfo();
    ExecuteResult LeaveMatchmakeSession();
    ExecuteResult WaitLeaveMatchmakeSession();
    ExecuteResult WaitForRetryJoinMatchmakeSession();
    ExecuteResult WaitRequestSessionInfo();

private:
    LanMatchmakeSession* m_matchmakeSession;
    common::Time m_retryTime;
    Result m_startupResult;
};

}
}
}

#endif
