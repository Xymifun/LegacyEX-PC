#include "nn/nex/RelayStream.hpp"

#include <string.h>

#include "nn/nex/Buffer.hpp"
#include "nn/nex/PacketBufferPacketOut.hpp"
#include "nn/nex/PacketOut.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/StreamSettings.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned char RELAY_OUTGOING_BYTE = 0xD0;
const unsigned int RELAY_TYPE_MASK = 0xF;
const unsigned int RELAY_ROUTING_TIMEOUT_FACTOR = 3;
const PacketType RELAY_PACKET_TYPE = static_cast<PacketType>(0xF);

}

RelayStream::RelayStream()
    : Stream(Relay),
      m_pTransport(0),
      m_routingTable(g_streamSettings[Unknown1].Get().GetMaxSilenceTime() * RELAY_ROUTING_TIMEOUT_FACTOR),
      m_uiGameServerID(RELAY_INVALID_ID),
      m_uiRelayGatheringID(RELAY_INVALID_ID),
      m_pPingCallback(0),
      m_unknown3C(0)
{
}

bool RelayStream::ReceiveIncomingPacket(unsigned short uiPort, unsigned char, Packet* pPacket)
{
    bool bResult = false;
    Buffer* pPayload = pPacket->m_pPayload;
    InetAddress source(pPacket->GetRelayHeader().m_uiSourceAddress, pPacket->GetRelayHeader().m_uiSourcePort);
    InetAddress destination(pPacket->GetRelayHeader().m_uiDestinationAddress,
                            pPacket->GetRelayHeader().m_uiDestinationPort);

    if ((pPacket->GetRelayHeader().m_uiType & RELAY_TYPE_MASK) == RELAY_TYPE_DATA)
    {
        m_routingTable.Add(source, pPacket->m_address, true);

        if (!m_pTransport->Receive(uiPort, pPayload, &source))
        {
            m_routingTable.Remove(source);
        }
        else
        {
            bResult = true;
            pPacket->ReleaseRef();
        }
    }
    else if ((pPacket->GetRelayHeader().m_uiType & RELAY_TYPE_MASK) == RELAY_TYPE_PING
             && pPacket->GetRelayPingHeader().m_unknown0 == 0)
    {
        if (m_pPingCallback != 0)
        {
            m_pPingCallback(&m_unknown3C, &pPacket->m_address, uiPort, pPacket->GetRelayHeader().m_uiTimeStamp,
                            &pPacket->GetRelayPingHeader(), pPayload);
        }

        bResult = true;
        pPacket->ReleaseRef();
    }

    return bResult;
}

void RelayStream::DoWork()
{
    m_routingTable.PeriodicHouseCleaning();
}

PacketOut* RelayStream::BuildRelayPacket(Buffer* pPayload, RelayType type)
{
    PacketOut* pPacket;

    if (pPayload != 0 && pPayload->IsPooled())
    {
        pPacket = new (RootObject::TARGET_POOL_PACKET_MANAGEMENT)
            PacketBufferPacketOut(0, RELAY_PACKET_TYPE, 0, pPayload, 0);

        if (pPacket == 0)
        {
            return 0;
        }

        if (!pPacket->IsValid())
        {
            delete pPacket;
            return 0;
        }
    }
    else
    {
        pPacket = new PacketOut(0, RELAY_PACKET_TYPE, 0, pPayload, 0);
    }

    pPacket->m_uiVersion = RELAY_OUTGOING_BYTE;
    memset(&pPacket->GetRelayHeader(), 0, sizeof(RelayProtocolHeader));
    pPacket->GetRelayHeader().m_uiType = type & RELAY_TYPE_MASK;
    return pPacket;
}

}
}
