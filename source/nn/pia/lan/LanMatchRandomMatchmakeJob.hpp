#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHRANDOMMATCHMAKEJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHRANDOMMATCHMAKEJOB_HPP

#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/session/RandomMatchmakeJob.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

class LanMatchmakeSession;

class LanMatchRandomMatchmakeJob : public session::RandomMatchmakeJob
{
public:
    LanMatchRandomMatchmakeJob();
    virtual ~LanMatchRandomMatchmakeJob();
    virtual void Trace(unsigned long long value) const;
    virtual void Cleanup();
    virtual void NoticeOwnerChangeEvent(unsigned int sessionId, unsigned long long ownerId);
    virtual Result StartupImpl(const session::JoinRandomSessionSetting* setting);
    virtual void SetNextStepAtWaitCreateMesh();
    virtual void SetNextStepAtWaitJoinMesh();
    virtual ExecuteResult SetNextStepAtFailedCreateMesh(Result result);
    virtual ExecuteResult SetNextStepAtFailedJoinMesh(Result result);
    virtual ExecuteResult SetNextStepAtWaitLeaveMesh();

    ExecuteResult RandomMatchmake();
    ExecuteResult WaitRandomMatchmake();
    ExecuteResult UnregisterGathering();
    ExecuteResult WaitUnregisterGathering();
    ExecuteResult LeaveMatchmakeSession();
    ExecuteResult WaitLeaveMatchmakeSession();
    ExecuteResult WaitForRetryRandomMatchmake();
    ExecuteResult WaitRequestSessionInfo();

private:
    common::CallContext m_subCallContext;
    LanMatchmakeSession* m_matchmakeSession;
    common::Time m_retryTime;
    bool m_isOpenSession;
};

}
}
}

#endif
