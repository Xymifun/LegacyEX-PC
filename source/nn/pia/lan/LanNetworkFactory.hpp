#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANNETWORKFACTORY_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANNETWORKFACTORY_HPP

#include "nn/pia/session/SessionNetworkFactory.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

class LanNetworkFactory : public session::SessionNetworkFactory
{
public:
    LanNetworkFactory();
    virtual ~LanNetworkFactory();
    virtual unsigned int GetMaxStationNum() const;
    virtual unsigned int GetHeaderSize() const;
    virtual unsigned int GetMtuSizeMax() const;
    virtual unsigned int GetMtuSizeMin() const;
    virtual int GetDefaultRttInitialPulseInterval() const;
    virtual transport::NetworkFactory* CreateSelf();
    virtual void DeleteSelf(transport::NetworkFactory* factory);
    virtual Result CreateProtocols();
    virtual common::IPacketInput* CreateInputStream();
    virtual common::IPacketOutput* CreateOutputStream();
    virtual void DeleteInputStream(common::IPacketInput* stream);
    virtual void DeleteOutputStream(common::IPacketOutput* stream);
    virtual transport::ConnectStationJob* CreateConnectStationJob();
    virtual void DeleteConnectStationJob(transport::ConnectStationJob* job);
    virtual unsigned int GetHostMigrationMode() const;
    virtual unsigned int GetBrowseSessionInfoListSizeMax() const;
    virtual unsigned int GetBrowseCommunityInfoListSizeMax() const;
    virtual bool IsMultiCandidatePermitted() const;
    virtual bool IsJointSessionPermitted() const;
    virtual bool IsMultiplayerPermitted() const;
    virtual common::StepSequenceJob* CreateCreateMeshJob();
    virtual common::StepSequenceJob* CreateJoinMeshJob();
    virtual common::StepSequenceJob* CreateLeaveMeshJob();
    virtual common::StepSequenceJob* CreateProcessHostMigrationJob();
    virtual common::StepSequenceJob* CreateLeaveWithHostMigrationJob();
    virtual common::StepSequenceJob* CreateKickoutManageJob();
    virtual void DeleteCreateMeshJob(common::StepSequenceJob* job);
    virtual void DeleteJoinMeshJob(common::StepSequenceJob* job);
    virtual void DeleteLeaveMeshJob(common::StepSequenceJob* job);
    virtual void DeleteProcessHostMigrationJob(common::StepSequenceJob* job);
    virtual void DeleteLeaveWithHostMigrationJob(common::StepSequenceJob* job);
    virtual void DeleteKickoutManageJob(common::StepSequenceJob* job);
    virtual common::StepSequenceJob* CreateMatchCreateSessionJob();
    virtual common::StepSequenceJob* CreateMatchRandomMatchmakeJob();
    virtual common::StepSequenceJob* CreateMatchBrowseMatchmakeJob();
    virtual common::StepSequenceJob* CreateMatchJoinSessionJob();
    virtual common::StepSequenceJob* CreateMatchLeaveSessionJob();
    virtual common::StepSequenceJob* CreateMatchDestroySessionJob();
    virtual common::StepSequenceJob* CreateMatchUpdateSessionSettingJob();
    virtual common::StepSequenceJob* CreateMatchJointSessionJob();
    virtual void DeleteMatchCreateSessionJob(common::StepSequenceJob* job);
    virtual void DeleteMatchRandomMatchmakeJob(common::StepSequenceJob* job);
    virtual void DeleteMatchBrowseMatchmakeJob(common::StepSequenceJob* job);
    virtual void DeleteMatchJoinSessionJob(common::StepSequenceJob* job);
    virtual void DeleteMatchLeaveSessionJob(common::StepSequenceJob* job);
    virtual void DeleteMatchDestroySessionJob(common::StepSequenceJob* job);
    virtual void DestroyMatchJointSessionJob(common::StepSequenceJob* job);
    virtual void DeleteMatchUpdateSessionSettingJob(common::StepSequenceJob* job);
    virtual session::ISessionInfoList* CreateSessionInfoList(unsigned short size);
    virtual session::IMatchmakeSession* CreateMatchmakeSession();
    virtual session::MeshLayerController* CreateMatchMeshLayerController();
    virtual void DeleteSessionInfoList(session::ISessionInfoList* list);
    virtual void DeleteMatchmakeSession(session::IMatchmakeSession* session);
    virtual void DeleteMatchMeshLayerController(session::MeshLayerController* controller);
};

}
}
}

#endif
