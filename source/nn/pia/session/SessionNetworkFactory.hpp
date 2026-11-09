#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_SESSIONNETWORKFACTORY_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_SESSIONNETWORKFACTORY_HPP

#include "nn/pia/NetworkFactory.hpp"

namespace nn
{
namespace pia
{
namespace session
{

class SessionNetworkFactory : public transport::NetworkFactory
{
public:
    SessionNetworkFactory();
    virtual ~SessionNetworkFactory();
    virtual bool IsClientRelayConnectionPermitted() const;
    virtual bool IsBitRateCheckPermitted() const;
    virtual Result CreateProtocols();
    virtual transport::PacketHandler* CreatePacketHandler();
    virtual transport::MissingStationHandler* CreateMissingStationHandler();
    virtual void DeletePacketHandler(transport::PacketHandler* handler);
    virtual void DeleteMissingStationHandler(transport::MissingStationHandler* handler);
    virtual transport::ConnectStationJob* CreateConnectStationJob();
    virtual transport::DisconnectStationJob* CreateDisconnectStationJob();
    virtual void DeleteConnectStationJob(transport::ConnectStationJob* job);
    virtual void DeleteDisconnectStationJob(transport::DisconnectStationJob* job);
    virtual common::MonitoringDataSender* CreateMonitoringDataSender();
    virtual void DeleteMonitoringDataSender(common::MonitoringDataSender* sender);
    virtual unsigned int GetHostMigrationMode() const = 0;
    virtual unsigned int GetBrowseSessionInfoListSizeMax() const = 0;
    virtual unsigned int GetBrowseCommunityInfoListSizeMax() const = 0;
    virtual bool IsBindRequired() const;
    virtual bool IsServerRequired() const;
    virtual bool IsMultiCandidatePermitted() const;
    virtual bool IsJointSessionPermitted() const;
    virtual bool IsMultiplayerPermitted() const;
    virtual bool IsRecentPlayRecordPermitted() const;
    virtual common::StepSequenceJob* CreateCreateMeshJob() = 0;
    virtual common::StepSequenceJob* CreateJoinMeshJob() = 0;
    virtual common::StepSequenceJob* CreateLeaveMeshJob() = 0;
    virtual common::StepSequenceJob* CreateProcessHostMigrationJob() = 0;
    virtual common::StepSequenceJob* CreateLeaveWithHostMigrationJob() = 0;
    virtual common::StepSequenceJob* CreateKickoutManageJob() = 0;
    virtual void DeleteCreateMeshJob(common::StepSequenceJob* job) = 0;
    virtual void DeleteJoinMeshJob(common::StepSequenceJob* job) = 0;
    virtual void DeleteLeaveMeshJob(common::StepSequenceJob* job) = 0;
    virtual void DeleteProcessHostMigrationJob(common::StepSequenceJob* job) = 0;
    virtual void DeleteLeaveWithHostMigrationJob(common::StepSequenceJob* job) = 0;
    virtual void DeleteKickoutManageJob(common::StepSequenceJob* job) = 0;
    virtual common::StepSequenceJob* CreateMatchCreateSessionJob() = 0;
    virtual common::StepSequenceJob* CreateMatchRandomMatchmakeJob() = 0;
    virtual common::StepSequenceJob* CreateMatchBrowseMatchmakeJob() = 0;
    virtual common::StepSequenceJob* CreateMatchJoinSessionJob() = 0;
    virtual common::StepSequenceJob* CreateMatchLeaveSessionJob() = 0;
    virtual common::StepSequenceJob* CreateMatchDestroySessionJob() = 0;
    virtual common::StepSequenceJob* CreateMatchJointSessionJob() = 0;
    virtual common::StepSequenceJob* CreateMatchUpdateSessionSettingJob() = 0;
    virtual void DeleteMatchCreateSessionJob(common::StepSequenceJob* job) = 0;
    virtual void DeleteMatchRandomMatchmakeJob(common::StepSequenceJob* job) = 0;
    virtual void DeleteMatchBrowseMatchmakeJob(common::StepSequenceJob* job) = 0;
    virtual void DeleteMatchJoinSessionJob(common::StepSequenceJob* job) = 0;
    virtual void DeleteMatchLeaveSessionJob(common::StepSequenceJob* job) = 0;
    virtual void DeleteMatchDestroySessionJob(common::StepSequenceJob* job) = 0;
    virtual void DestroyMatchJointSessionJob(common::StepSequenceJob* job) = 0;
    virtual void DeleteMatchUpdateSessionSettingJob(common::StepSequenceJob* job) = 0;
    virtual common::StepSequenceJob* CreateGenerateMatchmakeSystemPasswordJob();
    virtual common::StepSequenceJob* CreateClearMatchmakeSystemPasswordJob();
    virtual common::StepSequenceJob* CreateCommunityManagementJob();
    virtual void DeleteGenerateMatchmakeSystemPasswordJob(common::StepSequenceJob* job);
    virtual void DeleteClearMatchmakeSystemPasswordJob(common::StepSequenceJob* job);
    virtual void DeleteCommunityManagementJob(common::StepSequenceJob* job);
    virtual ISessionInfoList* CreateSessionInfoList(unsigned short size) = 0;
    virtual ICommunityInfoList* CreateCommunityInfoList(unsigned char size);
    virtual IMatchmakeSession* CreateMatchmakeSession() = 0;
    virtual common::String* CreateMatchmakeSessionSystemPassword();
    virtual MeshLayerController* CreateMatchMeshLayerController() = 0;
    virtual void DeleteSessionInfoList(ISessionInfoList* list) = 0;
    virtual void DeleteCommunityInfoList(ICommunityInfoList* list);
    virtual void DeleteMatchmakeSession(IMatchmakeSession* session) = 0;
    virtual void DeleteMatchmakeSessionSystemPassword(common::String* password);
    virtual void DeleteMatchMeshLayerController(MeshLayerController* controller) = 0;
};

}
}
}

#endif
