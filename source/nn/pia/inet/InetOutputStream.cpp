#include "nn/pia/inet/InetOutputStream.hpp"

#include <coreinit/memory.h>

#include "nn/pia/common/InetAddress.hpp"
#include "nn/pia/common/Packet.hpp"
#include "nn/pia/common/Socket.hpp"
#include "nn/pia/common/common.hpp"
#include "nn/pia/inet/NexFacade.hpp"
#include "nn/pia/inet/NexNatRelay.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"
#include "nn/pia/transport/StationManager.hpp"

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
    cConnectedStationState = 5,
    cRelayHeaderSize = 44,
    cRelayPayloadSizeMax = 1320,
    cRelayBufferSize = 1408
};

enum
{
    cTraceError = 0x1,
    cTracePacket = 0x200
};
}

InetOutputStream::InetOutputStream() : common::SocketStreamBase(), common::IPacketOutput()
{
}

InetOutputStream::~InetOutputStream()
{
}

Result InetOutputStream::OnStationConnectionEvent()
{
    if (!m_started)
        return 0;

    OSBlockSet(m_stationAddresses, 0, sizeof(m_stationAddresses));
    for (transport::Station** station = transport::StationManager::s_pInstance->m_stations.begin();
         station != transport::StationManager::s_pInstance->m_stations.end(); ++station)
    {
        if ((*station)->m_field20 != cConnectedStationState)
            continue;
        const int index = (*station)->GetStationIndex();
        if (index > transport::cStationNumMax - 1)
            continue;
        if (NexFacade::GetInstance() != 0 && NexFacade::GetInstance()->IsRelayServiceEnabled()
            && NexFacade::GetInstance()->m_natRelay != 0
            && NexFacade::GetInstance()
                   ->m_natRelay->GetRelayServerAddress((*station)->m_stationAddress.m_inetAddress)
                   .IsValid())
            continue;
        const common::InetAddress& inetAddress = (*station)->m_stationAddress.m_inetAddress;
        common::SocketAddress address;
        address.SetInetAddress(inetAddress);
        m_stationAddresses[index] = address.GetSockAddrIn();
    }
    return 0;
}

Result InetOutputStream::CalculateCryptoParameter(unsigned char* parameter, unsigned int size, common::Packet* packet)
{
    if (size < 12)
        return transport::cResultInvalidArgument;

    unsigned int connectionId = 0;
    unsigned int gatheringId = 0;
    if (NexFacade::GetInstance() != 0)
    {
        connectionId = NexFacade::GetInstance()->GetLocalServiceConnectionId();
        gatheringId = NexFacade::GetInstance()->m_gatheringId;
    }
    const unsigned int key = (packet->m_packetData.m_header.m_passId << 24) | (gatheringId & 0xffffff);
    unsigned long long nonce = common::deserializeU64(packet->m_packetData.m_header.m_cryptoNonce);
    nonce = (static_cast<unsigned long long>(connectionId) << 48) | (nonce & 0xffffffffffULL);
    common::serializeU64(packet->m_packetData.m_header.m_cryptoNonce, nonce);
    common::serializeU32(parameter, key);
    common::serializeU64(parameter + 4, nonce);
    return 0;
}

int InetOutputStream::CreateMulticastAddressArray(common::SockAddrIn* addresses, unsigned int stationBitmap)
{
    int count = 0;
    const transport::Station* localStation = transport::StationManager::s_pInstance->m_localStation;
    if (localStation != 0)
    {
        const unsigned int localIndex = localStation->GetStationIndex();
        for (unsigned int index = 0; index < transport::cStationNumMax; ++index)
        {
            if (index == localIndex || ((stationBitmap >> index) & 1) == 0)
                continue;
            const common::SockAddrIn& address = m_stationAddresses[index];
            if (address.address == 0 || address.port == 0)
                continue;
            addresses[count] = address;
            ++count;
        }
    }
    return count;
}

Result InetOutputStream::SendTo(const void* data, unsigned int size, const common::InetAddress& address,
                                unsigned char ttl)
{
    int sentSize = 0;
    if (ttl != 0)
    {
        Result result = m_socket->SetTtl(ttl);
        if (result != 0)
        {
            if (!m_started)
                return 0;
            return result;
        }
    }
    Result result = m_socket->SendTo(data, size, address, &sentSize);
    if (ttl != 0)
    {
        Result ttlResult = m_socket->SetTtl(0);
        if (ttlResult != 0)
        {
            if (!m_started)
                return 0;
            return ttlResult;
        }
    }
    if (result != 0)
    {
        if (!m_started)
            return 0;
        return result;
    }
    if (sentSize != 0 && sentSize == static_cast<int>(size))
        return 0;
    return transport::cResultInvalidState;
}

Result InetOutputStream::SendToRelay(const void* data, unsigned int size, const common::InetAddress& address,
                                     unsigned char ttl)
{
    if (NexFacade::GetInstance()->IsRelayServiceEnabled())
    {
        if (size > cRelayPayloadSizeMax)
            return transport::cResultInvalidArgument;

        unsigned char buffer[cRelayBufferSize];
        NexFacade::GetInstance()->m_natRelay->Pack(
            buffer, data, size,
            transport::g_stationConnectionInfoTable->GetLocalStationConnectionInfo()
                .m_stationLocation.m_stationAddress.m_inetAddress,
            address);
        return SendTo(buffer, size + cRelayHeaderSize,
                      NexFacade::GetInstance()->m_natRelay->GetRelayServerAddress(address), ttl);
    }
    return SendTo(data, size, address, ttl);
}

Result InetOutputStream::SendToMulti(const void* data, unsigned int size, const common::SockAddrIn* addresses,
                                     int count)
{
    int sentSize = 0;
    Result result = m_socket->SendToMulti(data, size, addresses, count, &sentSize);
    if (result != 0)
    {
        if (!m_started)
            return 0;
        return result;
    }
    if (sentSize != 0 && sentSize == static_cast<int>(size))
        return 0;
    return transport::cResultInvalidState;
}

Result InetOutputStream::Write(const common::Packet& packet)
{
    if (!m_started)
        return 0;

    common::InetAddress address(packet.m_field5f4.m_inetAddress);
    if (address.IsValid())
    {
        Result result;
        if (NexFacade::GetInstance() != 0 && NexFacade::GetInstance()->IsRelayServiceEnabled()
            && NexFacade::GetInstance()->m_natRelay != 0
            && NexFacade::GetInstance()->m_natRelay->GetRelayServerAddress(address).IsValid())
            result = SendToRelay(packet.m_packetData.m_data, packet.m_size, address, packet.m_field600);
        else
            result = SendTo(packet.m_packetData.m_data, packet.m_size, address, packet.m_field600);
        if (result != 0)
        {
            packet.Trace(result == transport::cResultInvalidArgument ? cTraceError : cTracePacket, false);
            return result;
        }
        return 0;
    }

    common::SockAddrIn addresses[transport::cStationNumMax - 1];
    const int count = CreateMulticastAddressArray(addresses, packet.m_stationBitmap);
    if (count > 0)
    {
        Result result = SendToMulti(packet.m_packetData.m_data, packet.m_size, addresses, count);
        if (result != 0)
        {
            packet.Trace(result == transport::cResultInvalidArgument ? cTraceError : cTracePacket, false);
            return result;
        }
    }

    if (NexFacade::GetInstance() != 0 && NexFacade::GetInstance()->IsRelayServiceEnabled()
        && NexFacade::GetInstance()->m_natRelay != 0)
    {
        const transport::Station* localStation = transport::StationManager::s_pInstance->m_localStation;
        const unsigned int localIndex = localStation != 0 ? localStation->GetStationIndex() : cStationIndexInvalid;
        Result lastResult = 0;
        for (unsigned int index = 0; index < transport::cStationNumMax; ++index)
        {
            if (index == localIndex || (packet.m_stationBitmap >> index) == 0)
                continue;
            transport::Station* station = transport::StationManager::s_pInstance->GetStation(
                static_cast<StationIndex>(index));
            if (station == 0)
                continue;
            if (!NexFacade::GetInstance()
                     ->m_natRelay->GetRelayServerAddress(station->m_stationAddress.m_inetAddress)
                     .IsValid())
                continue;
            Result result = SendToRelay(packet.m_packetData.m_data, packet.m_size,
                                        station->m_stationAddress.m_inetAddress, 0);
            if (result != 0)
            {
                packet.Trace(result == transport::cResultInvalidArgument ? cTraceError : cTracePacket, false);
                if (result != transport::cResultInvalidArgument)
                    lastResult = result;
            }
        }
        if (lastResult != 0)
            return lastResult;
    }
    return 0;
}

unsigned int InetOutputStream::GetMulticastDestinationNum() const
{
    return 16;
}

unsigned int InetOutputStream::IsBroadcastSupported() const
{
    return false;
}

}
}
}
