#include "nn/pia/lan/LanNetworkFactory.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/IPacketInput.hpp"
#include "nn/pia/common/IPacketOutput.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/lan/LanConnectStationJob.hpp"
#include "nn/pia/lan/LanInputStream.hpp"
#include "nn/pia/lan/LanLeaveWithHostMigrationJob.hpp"
#include "nn/pia/lan/LanMatchBrowseMatchmakeJob.hpp"
#include "nn/pia/lan/LanMatchCreateSessionJob.hpp"
#include "nn/pia/lan/LanMatchDestroySessionJob.hpp"
#include "nn/pia/lan/LanMatchJoinSessionJob.hpp"
#include "nn/pia/lan/LanMatchJointSessionJob.hpp"
#include "nn/pia/lan/LanMatchLeaveSessionJob.hpp"
#include "nn/pia/lan/LanMatchMeshLayerController.hpp"
#include "nn/pia/lan/LanMatchRandomMatchmakeJob.hpp"
#include "nn/pia/lan/LanMatchUpdateSessionSettingJob.hpp"
#include "nn/pia/lan/LanMatchmakeSession.hpp"
#include "nn/pia/lan/LanOutputStream.hpp"
#include "nn/pia/lan/LanProcessHostMigrationJob.hpp"
#include "nn/pia/lan/LanProtocol.hpp"
#include "nn/pia/lan/LanSessionInfoPrivate.hpp"
#include "nn/pia/session/CreateMeshJob.hpp"
#include "nn/pia/session/JoinMeshJob.hpp"
#include "nn/pia/session/KickoutManageJob.hpp"
#include "nn/pia/session/LeaveMeshJob.hpp"
#include "nn/pia/session/SessionInfoList.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

namespace
{

enum
{
    cMaxStationNum = 16,
    cHeaderSize = 28,
    cMtuSize = 1364
};

}

LanNetworkFactory::LanNetworkFactory()
{
}

LanNetworkFactory::~LanNetworkFactory()
{
}

unsigned int LanNetworkFactory::GetMaxStationNum() const
{
    return cMaxStationNum;
}

unsigned int LanNetworkFactory::GetHeaderSize() const
{
    return cHeaderSize;
}

unsigned int LanNetworkFactory::GetMtuSizeMax() const
{
    return cMtuSize;
}

unsigned int LanNetworkFactory::GetMtuSizeMin() const
{
    return cMtuSize;
}

int LanNetworkFactory::GetDefaultRttInitialPulseInterval() const
{
    return 600;
}

transport::NetworkFactory* LanNetworkFactory::CreateSelf()
{
    return transport::CreateObject<LanNetworkFactory>();
}

void LanNetworkFactory::DeleteSelf(transport::NetworkFactory* factory)
{
    transport::DestroyObject(factory);
}

Result LanNetworkFactory::CreateProtocols()
{
    transport::Transport* transport = transport::g_piaTransport;
    if (transport == 0)
        return transport::cResultInvalidState;
    transport->m_protocolManager.CreateProtocol<LanProtocol>(0);
    return 0;
}

common::IPacketInput* LanNetworkFactory::CreateInputStream()
{
    return transport::CreateObject<LanInputStream>();
}

common::IPacketOutput* LanNetworkFactory::CreateOutputStream()
{
    return transport::CreateObject<LanOutputStream>();
}

void LanNetworkFactory::DeleteInputStream(common::IPacketInput* stream)
{
    if (stream != 0)
        transport::DestroyObject(static_cast<common::SocketStreamBase*>(static_cast<LanInputStream*>(stream)));
}

void LanNetworkFactory::DeleteOutputStream(common::IPacketOutput* stream)
{
    if (stream != 0)
        transport::DestroyObject(static_cast<common::SocketStreamBase*>(static_cast<LanOutputStream*>(stream)));
}

transport::ConnectStationJob* LanNetworkFactory::CreateConnectStationJob()
{
    return transport::CreateObject<LanConnectStationJob>();
}

void LanNetworkFactory::DeleteConnectStationJob(transport::ConnectStationJob* job)
{
    transport::DestroyObject(job);
}

unsigned int LanNetworkFactory::GetHostMigrationMode() const
{
    return 2;
}

unsigned int LanNetworkFactory::GetBrowseSessionInfoListSizeMax() const
{
    return 10;
}

unsigned int LanNetworkFactory::GetBrowseCommunityInfoListSizeMax() const
{
    return 0;
}

bool LanNetworkFactory::IsMultiCandidatePermitted() const
{
    return true;
}

bool LanNetworkFactory::IsJointSessionPermitted() const
{
    return true;
}

bool LanNetworkFactory::IsMultiplayerPermitted() const
{
    return false;
}

common::StepSequenceJob* LanNetworkFactory::CreateCreateMeshJob()
{
    return transport::CreateObject<session::CreateMeshJob>();
}

common::StepSequenceJob* LanNetworkFactory::CreateJoinMeshJob()
{
    return transport::CreateObject<session::JoinMeshJob>();
}

common::StepSequenceJob* LanNetworkFactory::CreateLeaveMeshJob()
{
    return transport::CreateObject<session::LeaveMeshJob>();
}

common::StepSequenceJob* LanNetworkFactory::CreateProcessHostMigrationJob()
{
    return transport::CreateObject<LanProcessHostMigrationJob>();
}

common::StepSequenceJob* LanNetworkFactory::CreateLeaveWithHostMigrationJob()
{
    return transport::CreateObject<LanLeaveWithHostMigrationJob>();
}

common::StepSequenceJob* LanNetworkFactory::CreateKickoutManageJob()
{
    return transport::CreateObject<session::KickoutManageJob>();
}

void LanNetworkFactory::DeleteCreateMeshJob(common::StepSequenceJob* job)
{
    transport::DestroyObject(job);
}

void LanNetworkFactory::DeleteJoinMeshJob(common::StepSequenceJob* job)
{
    transport::DestroyObject(job);
}

void LanNetworkFactory::DeleteLeaveMeshJob(common::StepSequenceJob* job)
{
    transport::DestroyObject(job);
}

void LanNetworkFactory::DeleteProcessHostMigrationJob(common::StepSequenceJob* job)
{
    transport::DestroyObject(job);
}

void LanNetworkFactory::DeleteLeaveWithHostMigrationJob(common::StepSequenceJob* job)
{
    transport::DestroyObject(job);
}

void LanNetworkFactory::DeleteKickoutManageJob(common::StepSequenceJob* job)
{
    transport::DestroyObject(job);
}

common::StepSequenceJob* LanNetworkFactory::CreateMatchCreateSessionJob()
{
    return transport::CreateObject<LanMatchCreateSessionJob>();
}

common::StepSequenceJob* LanNetworkFactory::CreateMatchRandomMatchmakeJob()
{
    return transport::CreateObject<LanMatchRandomMatchmakeJob>();
}

common::StepSequenceJob* LanNetworkFactory::CreateMatchBrowseMatchmakeJob()
{
    return transport::CreateObject<LanMatchBrowseMatchmakeJob>();
}

common::StepSequenceJob* LanNetworkFactory::CreateMatchJoinSessionJob()
{
    return transport::CreateObject<LanMatchJoinSessionJob>();
}

common::StepSequenceJob* LanNetworkFactory::CreateMatchLeaveSessionJob()
{
    return transport::CreateObject<LanMatchLeaveSessionJob>();
}

common::StepSequenceJob* LanNetworkFactory::CreateMatchDestroySessionJob()
{
    return transport::CreateObject<LanMatchDestroySessionJob>();
}

common::StepSequenceJob* LanNetworkFactory::CreateMatchUpdateSessionSettingJob()
{
    return transport::CreateObject<LanMatchUpdateSessionSettingJob>();
}

common::StepSequenceJob* LanNetworkFactory::CreateMatchJointSessionJob()
{
    return transport::CreateObject<LanMatchJointSessionJob>();
}

void LanNetworkFactory::DeleteMatchCreateSessionJob(common::StepSequenceJob* job)
{
    transport::DestroyObject(job);
}

void LanNetworkFactory::DeleteMatchRandomMatchmakeJob(common::StepSequenceJob* job)
{
    transport::DestroyObject(job);
}

void LanNetworkFactory::DeleteMatchBrowseMatchmakeJob(common::StepSequenceJob* job)
{
    transport::DestroyObject(job);
}

void LanNetworkFactory::DeleteMatchJoinSessionJob(common::StepSequenceJob* job)
{
    transport::DestroyObject(job);
}

void LanNetworkFactory::DeleteMatchLeaveSessionJob(common::StepSequenceJob* job)
{
    transport::DestroyObject(job);
}

void LanNetworkFactory::DeleteMatchDestroySessionJob(common::StepSequenceJob* job)
{
    transport::DestroyObject(job);
}

void LanNetworkFactory::DestroyMatchJointSessionJob(common::StepSequenceJob* job)
{
    transport::DestroyObject(job);
}

void LanNetworkFactory::DeleteMatchUpdateSessionSettingJob(common::StepSequenceJob* job)
{
    transport::DestroyObject(job);
}

session::ISessionInfoList* LanNetworkFactory::CreateSessionInfoList(unsigned short size)
{
    typedef session::SessionInfoList<LanSessionInfoPrivate> List;
    unsigned char listSize = 1;
    if (size != 0)
        listSize = static_cast<unsigned char>(size);
    void* storage = pead::allocateFromSeadHeap(sizeof(List), common::HeapManager::GetHeap());
    List* list;
    if (storage == 0)
        list = 0;
    else
        list = new (storage) List(listSize);
    return list;
}

session::IMatchmakeSession* LanNetworkFactory::CreateMatchmakeSession()
{
    return transport::CreateObject<LanMatchmakeSession>();
}

session::MeshLayerController* LanNetworkFactory::CreateMatchMeshLayerController()
{
    return transport::CreateObject<LanMatchMeshLayerController>();
}

void LanNetworkFactory::DeleteSessionInfoList(session::ISessionInfoList* list)
{
    transport::DestroyObject(list);
}

void LanNetworkFactory::DeleteMatchmakeSession(session::IMatchmakeSession* session)
{
    transport::DestroyObject(session);
}

void LanNetworkFactory::DeleteMatchMeshLayerController(session::MeshLayerController* controller)
{
    transport::DestroyObject(controller);
}

}
}
}
