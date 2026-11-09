#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_RANDOM_MATCHMAKE_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_RANDOM_MATCHMAKE_JOB_HPP

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

class JoinRandomSessionSetting;

class RandomMatchmakeJob : public common::StepSequenceJob
{
public:
    RandomMatchmakeJob();
    virtual ~RandomMatchmakeJob();
    virtual void Trace(unsigned long long value) const;
    virtual void Cleanup();
    virtual unsigned int GetJoinSessionStateForDebug() const;
    virtual void NoticeOwnerChangeEvent(unsigned int sessionId, unsigned long long ownerId);
    virtual void NoticeUnregisterEvent(unsigned int sessionId);
    virtual Result StartupImpl(const JoinRandomSessionSetting* setting) = 0;
    virtual void CleanupImpl();
    virtual void SetNextStepAtMeshStartup();
    virtual void SetNextStepAtWaitCreateMesh() = 0;
    virtual void SetNextStepAtWaitJoinMesh() = 0;
    virtual ExecuteResult SetNextStepAtFailedCreateMesh(Result result) = 0;
    virtual ExecuteResult SetNextStepAtFailedJoinMesh(Result result) = 0;
    virtual ExecuteResult SetNextStepAtWaitLeaveMesh() = 0;

    Result Startup(common::CallContext* callerContext, const JoinRandomSessionSetting* setting);
    ExecuteResult MeshStartup();
    ExecuteResult CreateMesh();
    ExecuteResult WaitCreateMesh();
    ExecuteResult GetStationConnection();
    ExecuteResult WaitGetStationConnection();
    ExecuteResult JoinMesh();
    bool CheckContextCallCancelled();
    ExecuteResult WaitJoinMesh();
    ExecuteResult LeaveMesh();
    ExecuteResult WaitLeaveMesh();
    void CalcProcessingTimeOfJoin() const;
    ExecuteResult CompleteProcess();
    ExecuteResult CompleteFailure();
    void CleanupInternalContexts();
    void SetMatchmakeStatus(Session::MatchmakeStatus status);
    void SetConnectionStatus(Session::ConnectionStatus status);

    template <class T>
    void setStep(ExecuteResult (T::*function)(), const char* name)
    {
        Step step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(function), name);
        m_currentStep.m_function = step.m_function;
        m_currentStep.m_name = step.m_name;
    }

    common::CallContext* m_callerContext;
    unsigned int m_joinState;
    unsigned int m_sessionId;
    bool m_isNewSession;
    bool m_joined;
    Result m_result;
    bool m_connectionInfoRetry;
    bool m_sessionInfoRetry;
    bool m_retryReasonB;
    bool m_sessionInvalidated;
    bool m_ownerChanged;
    bool m_retryReasonA;
    unsigned char m_retryCount;
    unsigned long long m_principalId;
    transport::StationConnectionInfo m_stationConnectionInfo;
    common::CallContext m_callContext;
    common::Time m_startTime;
};

}
}
}

#endif
