#include "nn/pia/NetworkFactory.hpp"
#include "nn/pia/transport/StationPacketHandler.hpp"
#include "nn/pia/transport/ConnectStationJob.hpp"
#include "nn/pia/transport/DisconnectStationJob.hpp"
#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/inet/MonitoringDataProtocol.hpp"
#include "nn/pia/lan/LanProtocol.hpp"
#include "nn/pia/nat/NatTraversalProtocol.hpp"
#include "nn/pia/session/MeshProtocol.hpp"
#include "nn/pia/session/SessionProtocol.hpp"
#include "nn/pia/session/SyncClockProtocol.hpp"
#include "nn/pia/transport/BandwidthCheckProtocol.hpp"
#include "nn/pia/transport/RttProtocol.hpp"
#include "nn/pia/transport/StationProtocol.hpp"
#include "nn/pia/transport/UnreliableProtocol.hpp"
#include "vendor/__N_22_chat_VoiceProtocol_cpp_bf22a0fe.hpp"

namespace nn

{
namespace pia
{
namespace transport
{

NetworkFactory::NetworkFactory()
{
}

NetworkFactory::~NetworkFactory()
{
}

Result NetworkFactory::CreateProtocols()
{
    return 0;
}

PacketHandler* NetworkFactory::CreatePacketHandler()
{
    return CreateObject<StationPacketHandler>();
}

MissingStationHandler* NetworkFactory::CreateMissingStationHandler()
{
    return 0;
}

void NetworkFactory::DeletePacketHandler(PacketHandler* handler)
{
    DestroyObject(handler);
}

void NetworkFactory::DeleteMissingStationHandler(MissingStationHandler*)
{
}

ConnectStationJob* NetworkFactory::CreateConnectStationJob()
{
    return CreateObject<ConnectStationJob>();
}

DisconnectStationJob* NetworkFactory::CreateDisconnectStationJob()
{
    return CreateObject<DisconnectStationJob>();
}

void NetworkFactory::DeleteConnectStationJob(ConnectStationJob* job)
{
    DestroyObject(job);
}

void NetworkFactory::DeleteDisconnectStationJob(DisconnectStationJob* job)
{
    DestroyObject(job);
}

common::MonitoringDataSender* NetworkFactory::CreateMonitoringDataSender()
{
    return 0;
}

void NetworkFactory::DeleteMonitoringDataSender(common::MonitoringDataSender*)
{
}

bool NetworkFactory::IsClientRelayConnectionPermitted() const
{
    return false;
}

bool NetworkFactory::IsStarConnection() const
{
    return false;
}

bool NetworkFactory::IsBitRateCheckPermitted() const
{
    return false;
}

unsigned int NetworkFactory::IsSupportedProtocol(unsigned char protocolType) const
{
    switch (protocolType)
    {
    case 0x08:
    case StationProtocol::cProtocolType:
    case session::MeshProtocol::cProtocolType:
    case session::SyncClockProtocol::cProtocolType:
    case 0x24:
    case 0x28:
    case 0x2c:
    case nat::NatTraversalProtocol::cProtocolType:
    case lan::LanProtocol::cProtocolType:
    case BandwidthCheckProtocol::cProtocolType:
    case RttProtocol::cProtocolType:
    case 0x64:
    case UnreliableProtocol::cProtocolType:
    case 0x6c:
    case 0x74:
    case chat::VoiceProtocol::cProtocolType:
    case ReliableProtocol::cProtocolType:
    case 0x84:
    case session::SessionProtocol::cProtocolType:
    case inet::MonitoringDataProtocol::cProtocolType:
    case 0xa8:
        return true;
    default:
        return false;
    }
}

}
}
}
