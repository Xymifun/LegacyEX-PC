#include "nn/pia/inet/NexNetworkFactory.hpp"
#include "nn/pia/common/MonitoringDataSender.hpp"
#include "nn/pia/common/IPacketInput.hpp"
#include "nn/pia/common/IPacketOutput.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/inet/InetConnectStationJob.hpp"
#include "nn/pia/inet/InetCreateMeshJob.hpp"
#include "nn/pia/inet/InetDisconnectStationJob.hpp"
#include "nn/pia/inet/InetInputStream.hpp"
#include "nn/pia/inet/InetJoinMeshJob.hpp"
#include "nn/pia/inet/InetLeaveWithHostMigrationJob.hpp"
#include "nn/pia/inet/InetMissingStationHandler.hpp"
#include "nn/pia/inet/InetOutputStream.hpp"
#include "nn/pia/inet/InetProcessHostMigrationJob.hpp"
#include "nn/pia/inet/MonitoringDataSender.hpp"
#include "nn/pia/inet/NexCommunityInfoList.hpp"
#include "nn/pia/inet/NexFacade.hpp"
#include "nn/pia/inet/NexMatchBrowseMatchmakeJob.hpp"
#include "nn/pia/inet/NexMatchClearSystemPasswordJob.hpp"
#include "nn/pia/inet/NexMatchCommunityManagementJob.hpp"
#include "nn/pia/inet/NexMatchCreateSessionJob.hpp"
#include "nn/pia/inet/NexMatchDestroySessionJob.hpp"
#include "nn/pia/inet/NexMatchGenerateSystemPasswordJob.hpp"
#include "nn/pia/inet/NexMatchJoinSessionJob.hpp"
#include "nn/pia/inet/NexMatchJointSessionJob.hpp"
#include "nn/pia/inet/NexMatchLeaveSessionJob.hpp"
#include "nn/pia/inet/NexMatchMeshLayerController.hpp"
#include "nn/pia/inet/NexMatchRandomMatchmakeJob.hpp"
#include "nn/pia/inet/NexMatchUpdateSessionSettingJob.hpp"
#include "nn/pia/inet/NexMatchmakeSession.hpp"
#include "nn/pia/inet/NexSessionInfoPrivate.hpp"
#include "nn/pia/session/ICommunityInfoList.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/ISessionInfoList.hpp"
#include "nn/pia/session/KickoutManageJob.hpp"
#include "nn/pia/session/LeaveMeshJob.hpp"
#include "nn/pia/session/MeshLayerController.hpp"
#include "nn/pia/session/SessionInfoList.hpp"
#include "nn/pia/String.hpp"
#include "nn/pia/transport/ConnectStationJob.hpp"
#include "nn/pia/transport/DisconnectStationJob.hpp"
#include "nn/pia/transport/MissingStationHandler.hpp"
#include "nn/pia/transport/RelayStationPacketHandler.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

namespace
{

enum
{
    cMaxStationNum = 16,
    cHeaderSize = 28,
    cMtuSizeMax = 1364,
    cMtuSizeMin = 576
};

}

NexNetworkFactory::NexNetworkFactory()
{
}

NexNetworkFactory::~NexNetworkFactory()
{
}

unsigned int NexNetworkFactory::GetMaxStationNum() const
{
    return cMaxStationNum;
}

unsigned int NexNetworkFactory::GetHeaderSize() const
{
    return cHeaderSize;
}

unsigned int NexNetworkFactory::GetMtuSizeMax() const
{
    return cMtuSizeMax;
}

unsigned int NexNetworkFactory::GetMtuSizeMin() const
{
    return cMtuSizeMin;
}

int NexNetworkFactory::GetDefaultRttInitialPulseInterval() const
{
    return 200;
}

bool NexNetworkFactory::IsClientRelayConnectionPermitted() const
{
    return true;
}

bool NexNetworkFactory::IsBitRateCheckPermitted() const
{
    return true;
}

transport::NetworkFactory* NexNetworkFactory::CreateSelf()
{
    return transport::CreateObject<NexNetworkFactory>();
}

void NexNetworkFactory::DeleteSelf(transport::NetworkFactory* factory)
{
    transport::DestroyObject(factory);
}

Result NexNetworkFactory::CreateProtocols()
{
    Result result;
    result = NexFacade::GetInstance()->CreateProtocols();
    if (result != 0)
        return result;
    return 0;
}

transport::PacketHandler* NexNetworkFactory::CreatePacketHandler()
{
    return transport::CreateObject<transport::RelayStationPacketHandler>();
}

transport::MissingStationHandler* NexNetworkFactory::CreateMissingStationHandler()
{
    return transport::CreateObject<InetMissingStationHandler>();
}

void NexNetworkFactory::DeletePacketHandler(transport::PacketHandler* handler)
{
    transport::DestroyObject(handler);
}

void NexNetworkFactory::DeleteMissingStationHandler(transport::MissingStationHandler* handler)
{
    transport::DestroyObject(handler);
}

common::IPacketInput* NexNetworkFactory::CreateInputStream()
{
    return transport::CreateObject<InetInputStream>();
}

common::IPacketOutput* NexNetworkFactory::CreateOutputStream()
{
    return transport::CreateObject<InetOutputStream>();
}

void NexNetworkFactory::DeleteInputStream(common::IPacketInput* stream)
{
    if (stream != 0)
        transport::DestroyObject(static_cast<common::SocketStreamBase*>(static_cast<InetInputStream*>(stream)));
}

void NexNetworkFactory::DeleteOutputStream(common::IPacketOutput* stream)
{
    if (stream != 0)
        transport::DestroyObject(static_cast<common::SocketStreamBase*>(static_cast<InetOutputStream*>(stream)));
}

transport::ConnectStationJob* NexNetworkFactory::CreateConnectStationJob()
{
    return transport::CreateObject<InetConnectStationJob>();
}

transport::DisconnectStationJob* NexNetworkFactory::CreateDisconnectStationJob()
{
    return transport::CreateObject<InetDisconnectStationJob>();
}

void NexNetworkFactory::DeleteConnectStationJob(transport::ConnectStationJob* job)
{
    transport::DestroyObject(job);
}

void NexNetworkFactory::DeleteDisconnectStationJob(transport::DisconnectStationJob* job)
{
    transport::DestroyObject(job);
}

common::MonitoringDataSender* NexNetworkFactory::CreateMonitoringDataSender()
{
    return transport::CreateObject<MonitoringDataSender>();
}

void NexNetworkFactory::DeleteMonitoringDataSender(common::MonitoringDataSender* sender)
{
    transport::DestroyObject(sender);
}

unsigned int NexNetworkFactory::GetHostMigrationMode() const
{
    return 2;
}

unsigned int NexNetworkFactory::GetBrowseSessionInfoListSizeMax() const
{
    return 300;
}

unsigned int NexNetworkFactory::GetBrowseCommunityInfoListSizeMax() const
{
    return 100;
}

bool NexNetworkFactory::IsBindRequired() const
{
    return true;
}

bool NexNetworkFactory::IsServerRequired() const
{
    return true;
}

bool NexNetworkFactory::IsMultiCandidatePermitted() const
{
    return true;
}

bool NexNetworkFactory::IsJointSessionPermitted() const
{
    return true;
}

bool NexNetworkFactory::IsMultiplayerPermitted() const
{
    return true;
}

bool NexNetworkFactory::IsRecentPlayRecordPermitted() const
{
    return true;
}

common::StepSequenceJob* NexNetworkFactory::CreateCreateMeshJob()
{
    return transport::CreateObject<InetCreateMeshJob>();
}

common::StepSequenceJob* NexNetworkFactory::CreateJoinMeshJob()
{
    return transport::CreateObject<InetJoinMeshJob>();
}

common::StepSequenceJob* NexNetworkFactory::CreateLeaveMeshJob()
{
    return transport::CreateObject<session::LeaveMeshJob>();
}

common::StepSequenceJob* NexNetworkFactory::CreateProcessHostMigrationJob()
{
    return transport::CreateObject<InetProcessHostMigrationJob>();
}

common::StepSequenceJob* NexNetworkFactory::CreateLeaveWithHostMigrationJob()
{
    return transport::CreateObject<InetLeaveWithHostMigrationJob>();
}

common::StepSequenceJob* NexNetworkFactory::CreateKickoutManageJob()
{
    return transport::CreateObject<session::KickoutManageJob>();
}

void NexNetworkFactory::DeleteCreateMeshJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

void NexNetworkFactory::DeleteJoinMeshJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

void NexNetworkFactory::DeleteLeaveMeshJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

void NexNetworkFactory::DeleteProcessHostMigrationJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

void NexNetworkFactory::DeleteLeaveWithHostMigrationJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

void NexNetworkFactory::DeleteKickoutManageJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

common::StepSequenceJob* NexNetworkFactory::CreateMatchCreateSessionJob()
{
    return transport::CreateObject<NexMatchCreateSessionJob>();
}

common::StepSequenceJob* NexNetworkFactory::CreateMatchRandomMatchmakeJob()
{
    return transport::CreateObject<NexMatchRandomMatchmakeJob>();
}

common::StepSequenceJob* NexNetworkFactory::CreateMatchBrowseMatchmakeJob()
{
    return transport::CreateObject<NexMatchBrowseMatchmakeJob>();
}

common::StepSequenceJob* NexNetworkFactory::CreateMatchJoinSessionJob()
{
    return transport::CreateObject<NexMatchJoinSessionJob>();
}

common::StepSequenceJob* NexNetworkFactory::CreateMatchLeaveSessionJob()
{
    return transport::CreateObject<NexMatchLeaveSessionJob>();
}

common::StepSequenceJob* NexNetworkFactory::CreateMatchDestroySessionJob()
{
    return transport::CreateObject<NexMatchDestroySessionJob>();
}

common::StepSequenceJob* NexNetworkFactory::CreateMatchUpdateSessionSettingJob()
{
    return transport::CreateObject<NexMatchUpdateSessionSettingJob>();
}

common::StepSequenceJob* NexNetworkFactory::CreateMatchJointSessionJob()
{
    return transport::CreateObject<NexMatchJointSessionJob>();
}

void NexNetworkFactory::DeleteMatchCreateSessionJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

void NexNetworkFactory::DeleteMatchRandomMatchmakeJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

void NexNetworkFactory::DeleteMatchBrowseMatchmakeJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

void NexNetworkFactory::DeleteMatchJoinSessionJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

void NexNetworkFactory::DeleteMatchLeaveSessionJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

void NexNetworkFactory::DeleteMatchDestroySessionJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

void NexNetworkFactory::DestroyMatchJointSessionJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

void NexNetworkFactory::DeleteMatchUpdateSessionSettingJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

void NexNetworkFactory::DeleteGenerateMatchmakeSystemPasswordJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

void NexNetworkFactory::DeleteClearMatchmakeSystemPasswordJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

common::StepSequenceJob* NexNetworkFactory::CreateGenerateMatchmakeSystemPasswordJob()
{
    return transport::CreateObject<NexMatchGenerateSystemPasswordJob>();
}

common::StepSequenceJob* NexNetworkFactory::CreateClearMatchmakeSystemPasswordJob()
{
    return transport::CreateObject<NexMatchClearSystemPasswordJob>();
}

common::StepSequenceJob* NexNetworkFactory::CreateCommunityManagementJob()
{
    return transport::CreateObject<NexMatchCommunityManagementJob>();
}

void NexNetworkFactory::DeleteCommunityManagementJob(common::StepSequenceJob* object)
{
    transport::DestroyObject(object);
}

session::ISessionInfoList* NexNetworkFactory::CreateSessionInfoList(unsigned short size)
{
    typedef session::SessionInfoList<NexSessionInfoPrivate> List;
    void* storage = pead::allocateFromSeadHeap(sizeof(List), common::HeapManager::GetHeap());
    return storage != 0 ? new (storage) List(size) : 0;
}

session::ICommunityInfoList* NexNetworkFactory::CreateCommunityInfoList(unsigned char size)
{
    void* storage = pead::allocateFromSeadHeap(sizeof(NexCommunityInfoList), common::HeapManager::GetHeap());
    session::ICommunityInfoList* list = storage != 0 ? new (storage) NexCommunityInfoList(size) : 0;
    return list;
}

session::IMatchmakeSession* NexNetworkFactory::CreateMatchmakeSession()
{
    return transport::CreateObject<NexMatchmakeSession>();
}

common::String* NexNetworkFactory::CreateMatchmakeSessionSystemPassword()
{
    return transport::CreateObject<common::FixedString<16> >();
}

void NexNetworkFactory::DeleteSessionInfoList(session::ISessionInfoList* list)
{
    transport::DestroyObject(list);
}

void NexNetworkFactory::DeleteCommunityInfoList(session::ICommunityInfoList* list)
{
    transport::DestroyObject(list);
}

void NexNetworkFactory::DeleteMatchmakeSession(session::IMatchmakeSession* session)
{
    transport::DestroyObject(session);
}

void NexNetworkFactory::DeleteMatchmakeSessionSystemPassword(common::String* password)
{
    transport::DestroyObject(password);
}

session::MeshLayerController* NexNetworkFactory::CreateMatchMeshLayerController()
{
    return transport::CreateObject<NexMatchMeshLayerController>();
}

void NexNetworkFactory::DeleteMatchMeshLayerController(session::MeshLayerController* controller)
{
    transport::DestroyObject(controller);
}

}
}
}
