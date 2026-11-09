#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_JOIN_SESSION_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_JOIN_SESSION_JOB_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/transport/StationConnectionInfo.hpp"

namespace nn
{
namespace pia
{
namespace session
{

class JoinSessionSetting;

class JoinSessionJob : public common::StepSequenceJob
{
public:
    JoinSessionJob();
    virtual ~JoinSessionJob();
    virtual void Trace(unsigned long long value) const;
    virtual void NoticeOwnerChangeEvent(unsigned int sessionId, unsigned long long ownerId);
    virtual void NoticeUnregisterEvent(unsigned int sessionId);
    virtual void CleanupInternalContexts();
    virtual bool CheckContextCallCancelled();
    virtual void Cleanup();
    virtual unsigned int GetJoinSessionStateForDebug() const;

    Result Startup(common::CallContext* callerContext, const JoinSessionSetting* setting);

protected:
    virtual Result StartupImpl(const JoinSessionSetting* setting) = 0;
    virtual void CleanupImpl();
    virtual void SetNextStepAtWaitJoinMesh();
    virtual ExecuteResult SetNextStepAtFailedJoinMesh(Result result) = 0;
    virtual ExecuteResult SetNextStepAtWaitLeaveMesh() = 0;
    virtual ExecuteResult SetNextStepAtMeshCleanup() = 0;

public:
    ExecuteResult MeshStartup();
    ExecuteResult JoinMesh();
    ExecuteResult WaitJoinMesh();
    ExecuteResult LeaveMesh();
    ExecuteResult WaitLeaveMesh();
    ExecuteResult WaitLeaveMeshWithHostMigration();
    ExecuteResult WaitDestroyMesh();
    ExecuteResult MeshCleanup();
    void CalcProcessingTimeOfJoin() const;
    ExecuteResult CompleteProcess();
    ExecuteResult CompleteFailure();
    void SetMatchmakeStatus(Session::MatchmakeStatus status);
    void SetConnectionStatus(Session::ConnectionStatus status);

    common::CallContext* m_callerContext;
    common::CallContext m_callContext;
    transport::StationConnectionInfo m_stationConnectionInfo;
    unsigned int m_joinState;
    unsigned int m_sessionId;
    bool m_joined;
    bool m_connectionInfoRetry;
    bool m_sessionInfoRetry;
    bool m_retryReasonB;
    bool m_sessionInvalidated;
    bool m_ownerChanged;
    bool m_retryReasonA;
    unsigned char m_retryCount;
    Result m_result;
    common::Time m_startTime;
};

}
}
}

#endif
