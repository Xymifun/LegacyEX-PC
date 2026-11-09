#include "nn/pia/session/SessionNetworkFactory.hpp"

#include "nn/pia/transport/ConnectStationJob.hpp"
#include "nn/pia/transport/DisconnectStationJob.hpp"
#include "nn/pia/transport/StationPacketHandler.hpp"

namespace nn
{
namespace pia
{
namespace session
{

SessionNetworkFactory::SessionNetworkFactory()
{
}

SessionNetworkFactory::~SessionNetworkFactory()
{
}

bool SessionNetworkFactory::IsClientRelayConnectionPermitted() const
{
    return NetworkFactory::IsClientRelayConnectionPermitted();
}

bool SessionNetworkFactory::IsBitRateCheckPermitted() const
{
    return NetworkFactory::IsBitRateCheckPermitted();
}

Result SessionNetworkFactory::CreateProtocols()
{
    return NetworkFactory::CreateProtocols();
}

transport::PacketHandler* SessionNetworkFactory::CreatePacketHandler()
{
    return NetworkFactory::CreatePacketHandler();
}

transport::MissingStationHandler* SessionNetworkFactory::CreateMissingStationHandler()
{
    return NetworkFactory::CreateMissingStationHandler();
}

void SessionNetworkFactory::DeletePacketHandler(transport::PacketHandler* handler)
{
    NetworkFactory::DeletePacketHandler(handler);
}

void SessionNetworkFactory::DeleteMissingStationHandler(transport::MissingStationHandler* handler)
{
    NetworkFactory::DeleteMissingStationHandler(handler);
}

transport::ConnectStationJob* SessionNetworkFactory::CreateConnectStationJob()
{
    return NetworkFactory::CreateConnectStationJob();
}

transport::DisconnectStationJob* SessionNetworkFactory::CreateDisconnectStationJob()
{
    return NetworkFactory::CreateDisconnectStationJob();
}

void SessionNetworkFactory::DeleteConnectStationJob(transport::ConnectStationJob* job)
{
    NetworkFactory::DeleteConnectStationJob(job);
}

void SessionNetworkFactory::DeleteDisconnectStationJob(transport::DisconnectStationJob* job)
{
    NetworkFactory::DeleteDisconnectStationJob(job);
}

common::MonitoringDataSender* SessionNetworkFactory::CreateMonitoringDataSender()
{
    return NetworkFactory::CreateMonitoringDataSender();
}

void SessionNetworkFactory::DeleteMonitoringDataSender(common::MonitoringDataSender* sender)
{
    NetworkFactory::DeleteMonitoringDataSender(sender);
}

bool SessionNetworkFactory::IsBindRequired() const
{
    return false;
}

bool SessionNetworkFactory::IsServerRequired() const
{
    return false;
}

bool SessionNetworkFactory::IsMultiCandidatePermitted() const
{
    return false;
}

bool SessionNetworkFactory::IsJointSessionPermitted() const
{
    return false;
}

bool SessionNetworkFactory::IsMultiplayerPermitted() const
{
    return false;
}

bool SessionNetworkFactory::IsRecentPlayRecordPermitted() const
{
    return false;
}

common::StepSequenceJob* SessionNetworkFactory::CreateGenerateMatchmakeSystemPasswordJob()
{
    return 0;
}

common::StepSequenceJob* SessionNetworkFactory::CreateClearMatchmakeSystemPasswordJob()
{
    return 0;
}

common::StepSequenceJob* SessionNetworkFactory::CreateCommunityManagementJob()
{
    return 0;
}

void SessionNetworkFactory::DeleteGenerateMatchmakeSystemPasswordJob(common::StepSequenceJob*)
{
}

void SessionNetworkFactory::DeleteClearMatchmakeSystemPasswordJob(common::StepSequenceJob*)
{
}

void SessionNetworkFactory::DeleteCommunityManagementJob(common::StepSequenceJob*)
{
}

ICommunityInfoList* SessionNetworkFactory::CreateCommunityInfoList(unsigned char)
{
    return 0;
}

common::String* SessionNetworkFactory::CreateMatchmakeSessionSystemPassword()
{
    return 0;
}

void SessionNetworkFactory::DeleteCommunityInfoList(ICommunityInfoList*)
{
}

void SessionNetworkFactory::DeleteMatchmakeSessionSystemPassword(common::String*)
{
}

}
}
}
