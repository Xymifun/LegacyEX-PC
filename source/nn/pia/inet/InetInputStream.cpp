#include "nn/pia/inet/InetInputStream.hpp"

#include <coreinit/memory.h>

#include "nn/pia/String.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/InetAddress.hpp"
#include "nn/pia/common/Packet.hpp"
#include "nn/pia/common/Socket.hpp"
#include "nn/pia/common/StationAddress.hpp"
#include "nn/pia/common/common.hpp"
#include "nn/pia/inet/NexFacade.hpp"
#include "nn/pia/inet/NexNatRelay.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

common::InetAddress InetInputStream::s_ignoreAddresses[cIgnoreAddressNum];

InetInputStream::InetInputStream() : common::SocketStreamBase(), common::IPacketInput()
{
    for (unsigned int index = 0; index < cIgnoreAddressNum; ++index)
    {
        s_ignoreAddresses[index].m_address = 0;
        s_ignoreAddresses[index].m_port = 0;
    }
}

InetInputStream::~InetInputStream()
{
}

Result InetInputStream::readFromSocket(common::Packet* packet)
{
    common::InetAddress source;
    unsigned char ttl = 0;
    unsigned char* data = packet->m_packetData.m_data;
    int* size = reinterpret_cast<int*>(&packet->m_size);
    Result result = m_socket->RecvFrom(data, common::Packet::cDataSizeMax, &source, &ttl, size);
    if (result != 0)
    {
        if (!m_started)
            return transport::cResultDataNotReady;
        return result;
    }

    if (NexFacade::GetInstance() != 0 && NexFacade::GetInstance()->IsRelayServiceEnabled())
    {
        Result unpackResult = NexFacade::GetInstance()->m_natRelay->Unpack(data, size, &source);
        if (unpackResult != 0)
            return unpackResult;
    }

    if (!source.IsValid())
    {
        common::FixedString<32> addressString;
        source.GetAddressString(&addressString);
        return transport::cResultDataNotReady;
    }

    bool ignored = false;
    for (unsigned int index = 0; index < cIgnoreAddressNum; ++index)
    {
        if (s_ignoreAddresses[index].m_address == source.m_address)
        {
            ignored = true;
            break;
        }
    }
    if (ignored)
        return transport::cResultDataNotReady;

    common::StationAddress stationAddress;
    result = stationAddress.SetInetAddress(source);
    packet->m_field608 = stationAddress;
    packet->m_field600 = ttl;
    return 0;
}

Result InetInputStream::Read(common::Packet* packet)
{
    if (!m_started)
        return transport::cResultDataNotReady;
    return readFromSocket(packet);
}

Result InetInputStream::CalculateCryptoParameter(unsigned char* parameter, unsigned int size, common::Packet* packet)
{
    if (size < 12)
        return ResultInvalidArgument();
    unsigned int gatheringId = 0;
    if (NexFacade::GetInstance() != 0)
        gatheringId = NexFacade::GetInstance()->m_gatheringId;
    common::serializeU32(parameter, (packet->m_packetData.m_header.m_passId << 24) | (gatheringId & 0xffffff));
    OSBlockMove(parameter + 4, packet->m_packetData.m_header.m_cryptoNonce,
                sizeof(packet->m_packetData.m_header.m_cryptoNonce), FALSE);
    return 0;
}

void InetInputStream::AddIgnoreAddress(const common::InetAddress& address1, const common::InetAddress& address2)
{
    s_ignoreAddresses[0] = address1;
    s_ignoreAddresses[1] = address2;
}

}
}
}
