#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHJOINTSESSIONJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANMATCHJOINTSESSIONJOB_HPP

#include "nn/pia/session/JointSessionJob.hpp"
#include "nn/pia/transport/StationConnectionInfo.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

class LanMatchJointSessionJob : public session::JointSessionJob
{
public:
    LanMatchJointSessionJob();
    virtual ~LanMatchJointSessionJob();
    virtual void Trace(unsigned long long value) const;
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
    virtual void CleanupImpl();

    ExecuteResult CallSessionEvent();
    ExecuteResult SendInvitationAsCompanion();
    ExecuteResult WaitForAnswerToInvitation();
    ExecuteResult StartRandomMatchmake();
    ExecuteResult WaitRandomMatchmake();
    ExecuteResult StartJoinMatchmakeSession();
    ExecuteResult WaitJoinMatchmakeSession();
    ExecuteResult SendNextSessionId();
    ExecuteResult WaitCompanionStationPrepared();
    ExecuteResult SendAnswerToInvitation();
    ExecuteResult WaitNextSessionId();
    ExecuteResult StartUpdateSessionKey();
    ExecuteResult WaitUpdateSessionKey();
    ExecuteResult SendPreparedForMigrateSession();
    ExecuteResult WaitUntilLeaderLeavesMesh();
    ExecuteResult StartLeavePreviousMesh();
    ExecuteResult WaitLeavePreviousMesh();
    ExecuteResult StartLeavePreviousMatchmakeSession();
    ExecuteResult WaitLeavePreviousMatchmakeSession();
    ExecuteResult MeshRestart();
    ExecuteResult StartCreateNextMesh();
    ExecuteResult WaitCreateNextMesh();
    ExecuteResult StartGetNextMeshHostStationConnectionInfo();
    ExecuteResult WaitGetNextMeshHostStationConnectionInfo();
    ExecuteResult StartJoinNextMesh();
    ExecuteResult WaitJoinNextMesh();
    ExecuteResult WaitCompanionStation();
    ExecuteResult WaitHostStationId();
    ExecuteResult ProcessComplete();

    transport::StationConnectionInfo m_stationConnectionInfo;
};

}
}
}

#endif
