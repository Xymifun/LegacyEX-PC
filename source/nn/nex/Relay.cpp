#include "nn/nex/Relay.hpp"

#include <string.h>

#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/NATTraversalEngine.hpp"
#include "nn/nex/Network.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/PacketOut.hpp"
#include "nn/nex/RelayStream.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/StationURL.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned char RELAY_STREAM_ID = 0xF;
const unsigned char RELAY_PING_HEADER_TYPE_2 = 2;

}

Relay::Relay() : m_bEnabled(false), m_bRegistered(false), m_pNetwork(0), m_address()
{
    m_pStream = new RelayStream();
}

Relay::~Relay()
{
    UnregisterStream();

    if (m_pStream != 0)
    {
        delete m_pStream;
    }
}

void Relay::SetTransport(RootTransport* pTransport)
{
    UnregisterStream();

    m_pStream->m_pTransport = pTransport;

    if (m_bEnabled)
    {
        m_bRegistered = pTransport->m_streamManager.RegisterStream(m_pStream, RELAY_STREAM_ID, 0);
    }
}

void Relay::UnregisterStream()
{
    if (m_bRegistered)
    {
        RootTransport* pTransport = m_pStream->m_pTransport;

        if (pTransport != 0)
        {
            m_bRegistered = !pTransport->m_streamManager.UnregisterStream(m_pStream->GetType(), RELAY_STREAM_ID);
        }

        Clean();
    }
}

void Relay::Clean()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());

    m_pStream->m_routingTable.Clean();
    SetRelayGatheringID(RELAY_INVALID_ID);
    SetGameServerID(RELAY_INVALID_ID);

    if (Network::GetInstance() != 0)
    {
        if (Network::GetInstance()->GetNATTraversalEngine() != 0)
        {
            NATTraversalEngine* pEngine = Network::GetInstance()->GetNATTraversalEngine();
            pEngine->m_relayServerAddress = String("");
            Network::GetInstance()->GetNATTraversalEngine()->m_uiRelayServerPort = 0;
        }

        if (&Network::GetInstance()->GetMessageSelector() != 0)
        {
            Network::GetInstance()->GetMessageSelector().GetRelayMessage()->ClearRelayDataPacketSignatureKey();
        }
    }
}

void Relay::EnableRelay(bool bEnable)
{
    m_bEnabled = bEnable;

    if (bEnable)
    {
        RootTransport* pTransport = m_pStream->m_pTransport;

        if (pTransport != 0)
        {
            m_bRegistered = pTransport->m_streamManager.RegisterStream(m_pStream, RELAY_STREAM_ID, 0);
        }

        m_pNetwork = Network::GetInstance();
    }
    else
    {
        UnregisterStream();
    }
}

bool Relay::Exists()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    return m_pStream->m_routingTable.GetSize() != 0;
}

bool Relay::HasRelay(const InetAddress* pAddress, const InetAddress* pRelay)
{
    InetAddress found;
    bool bFound = m_pStream->m_routingTable.Find(*pAddress, found);

    if (bFound)
    {
        if (pAddress->GetAddress() == found.GetAddress() && pAddress->GetPortNumber() == found.GetPortNumber())
        {
            return false;
        }

        if (pRelay == 0 || *pRelay == found)
        {
            return true;
        }
    }

    return false;
}

bool Relay::ShouldRelay(const InetAddress* pAddress)
{
    if (!m_bEnabled)
    {
        return false;
    }

    return HasRelay(pAddress, 0);
}

bool Relay::AddRelay(const InetAddress& rAddress, const InetAddress& rKey, bool bReplace)
{
    return m_pStream->m_routingTable.Add(rAddress, rKey, bReplace);
}

bool Relay::RelayMessage(Buffer* pPayload, const InetAddress* pDestination, unsigned short uiPort)
{
    InetAddress route;

    m_pStream->m_routingTable.Find(*pDestination, route);

    PacketOut* pPacket = RelayStream::BuildRelayPacket(pPayload, RELAY_TYPE_DATA);

    if (pPacket == 0)
    {
        return false;
    }

    if (!pPacket->IsValid())
    {
        delete pPacket;
        return false;
    }

    pPacket->m_address = route;
    pPacket->m_unknown128 = true;
    pPacket->GetRelayHeader().m_uiSourceAddress = m_address.GetAddress();
    pPacket->GetRelayHeader().m_uiSourcePort = m_address.GetPortNumber();
    pPacket->GetRelayHeader().m_uiGameServerID = GetGameServerID();
    pPacket->GetRelayHeader().m_uiGatheringID = GetRelayGatheringID();
    pPacket->GetRelayHeader().m_uiDestinationAddress = pDestination->GetAddress();
    pPacket->GetRelayHeader().m_uiDestinationPort = pDestination->GetPortNumber();

    const bool bSent = m_pStream->m_pTransport->Send(uiPort, m_pStream->GetType(), RELAY_STREAM_ID, RELAY_STREAM_ID,
                                                     pPacket, true);

    pPacket->ReleaseRef();
    return bSent;
}

bool Relay::PingMessage(Buffer* pPayload, const RelayPingProtocolHeader* pHeader, const InetAddress* pDestination,
                        unsigned short uiPort)
{
    PacketOut* pPacket = RelayStream::BuildRelayPacket(pPayload, RELAY_TYPE_PING);

    if (pPacket == 0)
    {
        return false;
    }

    if (!pPacket->IsValid())
    {
        delete pPacket;
        return false;
    }

    memset(&pPacket->GetRelayPingHeader(), 0, sizeof(RelayPingProtocolHeader));
    bool bSent;

    if (pHeader->m_unknown0 == RELAY_PING_HEADER_TYPE_2)
    {
        InetAddress route;

        m_pStream->m_routingTable.Find(*pDestination, route);
        pPacket->m_address = route;
        pPacket->GetRelayHeader().m_uiSourceAddress = m_address.GetAddress();
        pPacket->GetRelayHeader().m_uiSourcePort = m_address.GetPortNumber();
        pPacket->GetRelayHeader().m_uiGameServerID = GetGameServerID();
        pPacket->GetRelayHeader().m_uiGatheringID = GetRelayGatheringID();
        pPacket->GetRelayHeader().m_uiDestinationAddress = pDestination->GetAddress();
        pPacket->GetRelayHeader().m_uiDestinationPort = pDestination->GetPortNumber();
        pPacket->GetRelayPingHeader().m_unknown0 = pHeader->m_unknown0;
        pPacket->GetRelayPingHeader().m_unknown4 = pHeader->m_unknown4;
        pPacket->GetRelayPingHeader().m_unknown8 = pHeader->m_unknown8;
        pPacket->m_unknown128 = true;
        bSent = m_pStream->m_pTransport->Send(uiPort, m_pStream->GetType(), RELAY_STREAM_ID, RELAY_STREAM_ID, pPacket,
                                              true);
        pPacket->ReleaseRef();
    }
    else
    {
        pPacket->m_address = *pDestination;
        pPacket->GetRelayHeader().m_uiGameServerID = GetGameServerID();
        pPacket->GetRelayHeader().m_uiGatheringID = GetRelayGatheringID();
        pPacket->GetRelayPingHeader().m_unknown0 = pHeader->m_unknown0;
        pPacket->GetRelayPingHeader().m_unknown4 = pHeader->m_unknown4;
        pPacket->GetRelayPingHeader().m_unknown8 = pHeader->m_unknown8;
        pPacket->m_unknown128 = true;
        bSent = m_pStream->m_pTransport->Send(uiPort, m_pStream->GetType(), RELAY_STREAM_ID, RELAY_STREAM_ID, pPacket,
                                              true);
        pPacket->ReleaseRef();
    }

    return bSent;
}

void Relay::SetRelayGatheringID(unsigned int uiID)
{
    m_pStream->m_uiRelayGatheringID = uiID;
}

unsigned int Relay::GetRelayGatheringID()
{
    return m_pStream->m_uiRelayGatheringID;
}

void Relay::SetGameServerID(unsigned int uiID)
{
    m_pStream->m_uiGameServerID = uiID;
}

unsigned int Relay::GetGameServerID()
{
    return m_pStream->m_uiGameServerID;
}

void Relay::SetSrcPublicAddress(const StationURL& rURL)
{
    m_address = *rURL.GetInetAddress();
}

}
}
