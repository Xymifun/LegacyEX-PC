#include "nn/pia/lan/LanInputStream.hpp"

#include <coreinit/memory.h>

#include "nn/pia/String.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/StationAddress.hpp"
#include "nn/pia/common/common.hpp"
#include "nn/pia/lan/LanProtocol.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

namespace
{
inline void traceAddress(const common::InetAddress& address)
{
    common::FixedString<32> addressString;
    address.GetAddressString(&addressString);
}
}

LanInputStream::LanInputStream() : common::SocketStreamBase(), common::IPacketInput()
{
    m_protocol = 0;
    m_broadcastSocket = 0;
    m_broadcastReceived = false;
}

LanInputStream::~LanInputStream()
{
    m_broadcastSocket = 0;
}

Result LanInputStream::readFromSocket(common::Socket* socket, common::Packet* packet)
{
    common::InetAddress source;
    unsigned char ancillary = 0;
    Result result;
    result = socket->RecvFrom(packet->m_packetData.m_data, common::Packet::cDataSizeMax, &source, &ancillary,
                              reinterpret_cast<int*>(&packet->m_size));
    if (result != 0)
    {
        if (!m_started)
            return transport::cResultDataNotReady;
        return result;
    }
    if (!source.IsValid())
    {
        traceAddress(source);
        return transport::cResultDataNotReady;
    }
    bool ignored = false;
    for (unsigned int index = 0; index < cIgnoreAddressMax; ++index)
    {
        if (m_ignoreAddresses[index].GetKey() == source.GetKey())
        {
            ignored = true;
            break;
        }
    }
    if (ignored)
    {
        traceAddress(source);
        return transport::cResultDataNotReady;
    }
    common::StationAddress stationAddress;
    stationAddress.SetInetAddress(source);
    packet->m_field608 = stationAddress;
    packet->m_field600 = ancillary;
    return 0;
}

Result LanInputStream::Read(common::Packet* packet)
{
    if (!m_started)
        return transport::cResultDataNotReady;

    Result result;
    if (m_protocol->m_broadcastSocketBound)
    {
        if (m_broadcastReceived)
        {
            result = readFromSocket(m_broadcastSocket, packet);
            if (result == transport::cResultDataNotReady)
                result = readFromSocket(m_socket, packet);
            else if (result == 0)
                m_broadcastReceived = false;
        }
        else
        {
            result = readFromSocket(m_socket, packet);
            if (result == transport::cResultDataNotReady)
                result = readFromSocket(m_broadcastSocket, packet);
            else if (result == 0)
                m_broadcastReceived = true;
        }
    }
    else
    {
        result = readFromSocket(m_socket, packet);
    }
    if (result == 0)
        m_protocol->m_unknown618 = 1;
    return result;
}

Result LanInputStream::CalculateCryptoParameter(unsigned char* parameter, unsigned int size, common::Packet* packet)
{
    if (size < 12)
        return transport::cResultInvalidArgument;
    common::serializeU32(parameter, packet->m_field608.m_inetAddress.m_address);
    OSBlockMove(parameter + 4, packet->m_packetData.m_header.m_cryptoNonce,
                sizeof(packet->m_packetData.m_header.m_cryptoNonce), FALSE);
    parameter[4] = packet->m_packetData.m_header.m_passId;
    return 0;
}

void LanInputStream::RegisterLanProtocol(LanProtocol* protocol)
{
    m_protocol = protocol;
}

void LanInputStream::UnregisterLanProtocol()
{
    m_protocol = 0;
}

Result LanInputStream::ImportBroadcastSocket(common::Socket* socket)
{
    m_broadcastSocket = socket;
    return 0;
}

void LanInputStream::AddIgnoreAddress(const common::InetAddress& address)
{
    if (!address.IsValid())
        return;
    int freeIndex = cIgnoreAddressMax;
    for (unsigned int index = 0; index < cIgnoreAddressMax; index++)
    {
        if (freeIndex == cIgnoreAddressMax && !m_ignoreAddresses[index].IsValid())
            freeIndex = index;
        if (m_ignoreAddresses[index] == address)
            return;
    }
    if (freeIndex != cIgnoreAddressMax)
        m_ignoreAddresses[freeIndex] = address;
}

void LanInputStream::ResetIgnoreAddress()
{
    for (unsigned int index = 0; index < cIgnoreAddressMax; index++)
    {
        common::InetAddress& address = m_ignoreAddresses[index];
        address.m_address = 0;
        address.m_port = 0;
    }
}

void LanInputStream::RemoveIgnoreAddess(const common::InetAddress& address)
{
    for (unsigned int index = 0; index < cIgnoreAddressMax; ++index)
    {
        if (m_ignoreAddresses[index].GetKey() == address.GetKey())
        {
            m_ignoreAddresses[index].m_address = 0;
            m_ignoreAddresses[index].m_port = 0;
        }
    }
}

}
}
}
