#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHJOINTSESSIONJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHJOINTSESSIONJOB_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/session/JointSessionJob.hpp"
#include "nn/pia/transport/StationConnectionInfo.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class NexMatchJointSessionJob : public session::JointSessionJob
{
public:
    enum
    {
        cStationNumMax = 32
    };

    NexMatchJointSessionJob();
    virtual ~NexMatchJointSessionJob();
    virtual void Trace(unsigned long long p0) const;
    virtual unsigned int GetJointSessionStateForDebug() const;
    virtual Result StartupJoinRandomJointSessionImpl(const session::JoinRandomSessionSetting* setting);
    virtual Result StartupCreateJointSessionImpl(const session::CreateSessionSetting* setting);
    virtual Result StartupJoinJointSessionImpl(const session::JoinSessionSetting* setting);
    virtual Result StartupLeaveJointSessionImpl();
    virtual Result StartupDestroyJointSessionImpl();
    virtual Result StartupPassiveProcessImpl(session::Session::ProcessType processType,
                                             session::SessionProtocol::MessageType messageType,
                                             const unsigned long long* stationIds, unsigned short stationIdNum,
                                             unsigned long long senderStationId);
    virtual Result ReceiveNextSessionIdImpl(session::Session::ProcessType processType, unsigned int sessionId,
                                            const unsigned char* applicationData, unsigned char applicationDataSize,
                                            unsigned long long senderStationId);
    virtual Result ReceiveLeaveInvitationImpl(session::Session::ProcessType processType,
                                              const unsigned long long* stationIds, unsigned short stationIdNum,
                                              unsigned long long senderStationId);
    virtual Result ReceiveLeaveMeshInvitationImpl(session::Session::ProcessType processType,
                                                  unsigned long long senderStationId);
    virtual Result ReceiveCompletionResponseImpl(session::Session::ProcessType processType,
                                                 unsigned long long senderStationId);
    virtual void NoticeHostChangeEventImpl(unsigned long long hostStationId);
    virtual void NoticeOwnerChangeEventImpl(unsigned int sessionId, unsigned long long ownerStationId);
    virtual void NoticeLeaveEventImpl(unsigned int sessionId, unsigned long long stationId);
    virtual void NoticeUnregisterEventImpl(unsigned int sessionId);
    virtual void CleanupImpl();

    ExecuteResult CallDestroySessionEvent();
    ExecuteResult CallSessionEvent();
    void CheckJoinedOnlyCompanion();
    void CleanupMonitoringData();
    ExecuteResult CloseJointSessionParticipation();
    ExecuteResult CloseMatchmakeSession();
    ExecuteResult GetMatchmakeSessionOwners();
    ExecuteResult GetNextMatchmakeSessionInfo();
    ExecuteResult MeshRestart();
    void PrepareBecomingToLeader();
    ExecuteResult ProcessComplete();
    ExecuteResult ProcessFailure();
    ExecuteResult ResendDestroyInvitation();
    ExecuteResult ResendInvitationAsCompanion();
    ExecuteResult ResendNextSessionId();
    ExecuteResult SendAnswerToDestroyInvitation();
    ExecuteResult SendAnswerToInvitation();
    ExecuteResult SendCompletionForMigrateSession();
    ExecuteResult SendCompletionInvitation();
    ExecuteResult SendDestroyInvitation();
    ExecuteResult SendInvitationAsCompanion();
    ExecuteResult SendLeaveMeshCompanion();
    ExecuteResult SendNextSessionId();
    ExecuteResult SendPreparedForMigrateSession();
    ExecuteResult StartCreateMatchmakeSession();
    ExecuteResult StartCreateNextMesh();
    ExecuteResult StartGetNextMeshHostStationConnectionInfo();
    ExecuteResult StartJoinMatchmakeSession();
    ExecuteResult StartJoinNextMesh();
    ExecuteResult StartLeaveBufferMatchmakeSession();
    ExecuteResult StartLeaveCurrentMatchmakeSession();
    ExecuteResult StartLeaveMesh();
    ExecuteResult StartLeavePreviousMatchmakeSession();
    ExecuteResult StartLeavePreviousMesh();
    ExecuteResult StartRandomMatchmake();
    ExecuteResult UpdateJointSessionOpenStatus();
    ExecuteResult WaitCloseJointSessionParticipation();
    ExecuteResult WaitCloseMatchmakeSession();
    ExecuteResult WaitCompanionStation();
    ExecuteResult WaitCompanionStationPrepared();
    ExecuteResult WaitCompletionInvitation();
    ExecuteResult WaitCreateMatchmakeSession();
    ExecuteResult WaitCreateNextMesh();
    ExecuteResult WaitForAnswerToCompletionInvitation();
    ExecuteResult WaitForAnswerToDestroyInvitation();
    ExecuteResult WaitForAnswerToInvitation();
    ExecuteResult WaitForInvitationAsCompanion();
    ExecuteResult WaitGetMatchmakeSessionOwners();
    ExecuteResult WaitGetNextMatchmakeSessionInfo();
    ExecuteResult WaitGetNextMeshHostStationConnectionInfo();
    ExecuteResult WaitHostMigrated();
    ExecuteResult WaitHostStationId();
    ExecuteResult WaitJoinMatchmakeSession();
    ExecuteResult WaitJoinNextMesh();
    ExecuteResult WaitLeaveBufferMatchmakeSession();
    ExecuteResult WaitLeaveCurrentMatchmakeSession();
    ExecuteResult WaitLeaveMesh();
    ExecuteResult WaitLeavePreviousMatchmakeSession();
    ExecuteResult WaitLeavePreviousMesh();
    ExecuteResult WaitMeshRestart();
    ExecuteResult WaitNextSessionId();
    ExecuteResult WaitNotification();
    ExecuteResult WaitRandomMatchmake();
    ExecuteResult WaitStartRetryJoinMesh();
    ExecuteResult WaitUntilCompanionCompletion();
    ExecuteResult WaitUntilCompanionLeavesMesh();
    ExecuteResult WaitUntilLeaderLeavesMesh();
    ExecuteResult WaitUpdateJointSessionOpenStatus();

    void setStep(ExecuteResult (NexMatchJointSessionJob::*function)(), const char* name)
    {
        Step step(static_cast<ExecuteResult (common::StepSequenceJob::*)()>(function), name);
        m_currentStep.m_function = step.m_function;
        m_currentStep.m_name = step.m_name;
    }

    transport::StationConnectionInfo m_stationConnectionInfo;
    common::Time m_time148;
    common::Time m_time150;
    common::Time m_time158;
    common::Time m_time160;
    common::Time m_time168;
    common::Time m_time170;
    common::Time m_time178;
    common::Time m_time180;
    common::Time m_time188;
    common::Time m_time190;
    common::Time m_time198;
    common::Time m_time1a0;
    common::Time m_time1a8;
    Result m_failureResult;
    unsigned int m_jointSessionState;
    unsigned char m_field1b8;
    unsigned long long m_field1c0;
    unsigned char m_field1c8;
    unsigned long long m_field1d0;
    unsigned char m_field1d8;
    unsigned long long m_field1e0;
    unsigned char m_field1e8;
    unsigned char m_field1e9;
    unsigned char m_field1ea;
    unsigned char m_field1eb;
    unsigned char m_field1ec;
    unsigned char m_field1ed;
    unsigned char m_field1ee;
    unsigned int m_sessionIds[cStationNumMax];
    unsigned char m_sessionIdNum;
    bool m_sessionInfoRequested[cStationNumMax];
};

}
}
}

#endif
