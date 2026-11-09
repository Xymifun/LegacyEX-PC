#include "nn/pia/lan/LanOutputStream.hpp"

#include <coreinit/memory.h>

#include "nn/pia/common/common.hpp"
#include "nn/pia/common/InetAddress.hpp"
#include "nn/pia/common/Packet.hpp"
#include "nn/pia/common/Socket.hpp"
#include "nn/pia/lan/LanProtocol.hpp"
#include "nn/pia/transport/StationManager.hpp"

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
    cConnectedStationState = 5,
    cBroadcastPort = 40000
};

const unsigned long long cTraceInvalidArgument = 0x1ULL;
const unsigned long long cTraceSendFailure = 0x0200000000000000ULL;
}

LanOutputStream::LanOutputStream() : common::SocketStreamBase(), common::IPacketOutput(), m_protocol(0)
{
}

LanOutputStream::~LanOutputStream()
{
}

Result LanOutputStream::OnStationConnectionEvent()
{
    if (!m_started)
        return 0;

    OSBlockSet(m_stationAddresses, 0, sizeof(m_stationAddresses));
    for (transport::Station** it = transport::StationManager::s_pInstance->m_stations.begin();
         it != transport::StationManager::s_pInstance->m_stations.end(); ++it)
    {
        transport::Station* station = *it;
        if (station->m_field20 == cConnectedStationState)
        {
            const int index = station->GetStationIndex();
            if (index <= transport::cStationNumMax - 1)
            {
                const common::InetAddress& inetAddress = station->m_stationAddress.m_inetAddress;
                common::SocketAddress address;
                address.SetInetAddress(inetAddress);
                m_stationAddresses[index] = address.GetSockAddrIn();
            }
        }
    }
    return 0;
}

Result LanOutputStream::CalculateCryptoParameter(unsigned char* output, unsigned int size, common::Packet* packet)
{
    if (size < 12)
        return transport::cResultInvalidArgument;
    unsigned int protocolId = 0;
    if (m_protocol != 0)
        protocolId = m_protocol->GetProtocolIdentifier();
    common::serializeU32(output, protocolId);
    OSBlockMove(output + 4, packet->m_packetData.m_header.m_cryptoNonce, 8, FALSE);
    output[4] = packet->m_packetData.m_header.m_passId;
    return 0;
}

int LanOutputStream::CreateMulticastAddressArray(common::SockAddrIn* output, unsigned int mask)
{
    int count = 0;
    const transport::Station* localStation = transport::StationManager::s_pInstance->m_localStation;
    if (localStation != 0)
    {
        const unsigned int localIndex = localStation->GetStationIndex();
        for (unsigned int index = 0; index < transport::cStationNumMax; ++index)
        {
            if (index == localIndex || ((mask >> index) & 1) == 0)
                continue;
            const common::SockAddrIn& address = m_stationAddresses[index];
            if (address.address == 0 || address.port == 0)
                continue;
            output[count] = address;
            ++count;
        }
    }
    return count;
}

Result LanOutputStream::SendTo(const void* data, unsigned int size, const common::InetAddress& address,
                               unsigned char ttl)
{
    int sent = 0;
    Result result;
    Result ttlResult;
    if (ttl != 0)
    {
        ttlResult = m_socket->SetTtl(ttl);
        if (ttlResult != 0)
        {
            if (!m_started)
                return 0;
            return ttlResult;
        }
    }
    result = m_socket->SendTo(data, static_cast<int>(size), address, &sent);
    if (ttl != 0)
    {
        ttlResult = m_socket->SetTtl(0);
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
    if (sent == 0 || static_cast<unsigned int>(sent) != size)
        return transport::cResultInvalidState;
    m_protocol->m_unknown618 = 1;
    return 0;
}

bool LanOutputStream::IsDestinationAddressIndividual(common::SockAddrIn* addresses, unsigned int count) const
{
    const unsigned int localAddress = m_protocol->GetProtocolIdentifier();
    for (unsigned int i = 0; i < count; ++i)
        if (localAddress == addresses[i].address)
            return false;
    for (unsigned int i = 0; i < count - 1; ++i)
        for (unsigned int j = i + 1; j <= count - 1; ++j)
            if (addresses[i].address == addresses[j].address)
                return false;
    return true;
}

Result LanOutputStream::SendToMulti(const void* data, unsigned int size, const common::SockAddrIn* addresses, int count)
{
    int sent = 0;
    Result result = m_socket->SendToMulti(data, static_cast<int>(size), addresses, count, &sent);
    if (result != 0)
    {
        if (!m_started)
            return 0;
        return result;
    }
    if (sent == 0 || static_cast<unsigned int>(sent) != size)
        return transport::cResultInvalidState;
    m_protocol->m_unknown618 = 1;
    return 0;
}

Result LanOutputStream::Write(const common::Packet& packet)
{
    if (!m_started)
        return 0;

    common::InetAddress address;
    if (packet.m_stationIndex != cStationIndexSentinelFF)
        address = packet.m_field5f4.m_inetAddress;
    if (address.IsValid())
    {
        Result result;
        result = SendTo(packet.m_packetData.m_data, packet.m_size, address, packet.m_field600);
        if (result != 0)
        {
            packet.Trace(result == transport::cResultInvalidArgument ? cTraceInvalidArgument : cTraceSendFailure,
                         false);
            return result;
        }
        return 0;
    }

    common::SockAddrIn addresses[transport::cStationNumMax - 1];
    const int count = CreateMulticastAddressArray(addresses, packet.m_stationBitmap);
    if (count == 0)
        return 0;

    if (count == 1)
    {
        common::SocketAddress socketAddress;
        socketAddress.SetSockAddrIn(addresses[0]);
        socketAddress.GetInetAddress(&address);
        Result result;
        result = SendTo(packet.m_packetData.m_data, packet.m_size, address, packet.m_field600);
        if (result != 0)
        {
            packet.Trace(result == transport::cResultInvalidArgument ? cTraceInvalidArgument : cTraceSendFailure,
                         false);
            return result;
        }
        return 0;
    }

    if (IsDestinationAddressIndividual(addresses, count) && m_protocol->m_broadcastSocketBound)
    {
        address.m_address = m_protocol->m_broadcastAddress;
        address.m_port = cBroadcastPort;
        Result result;
        result = SendTo(packet.m_packetData.m_data, packet.m_size, address, packet.m_field600);
        if (result != 0)
        {
            packet.Trace(result == transport::cResultInvalidArgument ? cTraceInvalidArgument : cTraceSendFailure,
                         false);
            return result;
        }
        return 0;
    }

    else
    {
        Result result;
        result = SendToMulti(packet.m_packetData.m_data, packet.m_size, addresses, count);
        if (result != 0)
        {
            packet.Trace(result == transport::cResultInvalidArgument ? cTraceInvalidArgument : cTraceSendFailure,
                         false);
            return result;
        }
        return 0;
    }
}

void LanOutputStream::RegisterLanProtocol(LanProtocol* protocol)
{
    m_protocol = protocol;
}

void LanOutputStream::UnregisterLanProtocol()
{
    m_protocol = 0;
}

unsigned int LanOutputStream::GetMulticastDestinationNum() const
{
    return 16;
}

unsigned int LanOutputStream::IsBroadcastSupported() const
{
    return true;
}

}
}
}
