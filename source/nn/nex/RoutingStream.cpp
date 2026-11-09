#include "nn/nex/RoutingStream.hpp"

#include "nn/nex/Buffer.hpp"
#include "nn/nex/ByteStream.hpp"
#include "nn/nex/PacketBuffer.hpp"
#include "nn/nex/PacketBufferPacketOut.hpp"
#include "nn/nex/PacketOut.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/Router.hpp"
#include "nn/nex/RoutingAddressResolver.hpp"
#include "nn/nex/StreamSettings.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int ROUTING_HEADER_SIZE = 6;
const unsigned int ROUTING_CRITICAL_SECTION_TRACE_MASK = 1;
const unsigned int ROUTING_TABLE_TIMEOUT_FACTOR = 3;
const unsigned int ROUTING_MAX_UNROUTABLE_SIZE = 12;
const unsigned int ROUTING_APPEND_AT_END = 0xFFFFFFFFu;
const unsigned int ROUTING_PACKET_TYPE_ROUTED = 6;
const unsigned long long ROUTING_TRACE_FILTER = 0x200000000000ULL;
const unsigned char ROUTING_SOURCE_PORT = 1;
const unsigned char ROUTING_DESTINATION_PORT = 1;

unsigned int s_uiHistoryCleaningPeriod = 0;
unsigned int s_uiHistoryLifetime = 10000;
unsigned int s_uiHistoryLimit = 0;

}

RoutingStream::RoutingStream()
    : Stream(Routing),
      m_criticalSection(ROUTING_CRITICAL_SECTION_TRACE_MASK),
      m_routingTable(g_streamSettings[Unknown1].Get().GetMaxSilenceTime() * ROUTING_TABLE_TIMEOUT_FACTOR),
      m_pTransport(0),
      m_pFilter(0)
{
    m_lastCleaning = Time(0);
}

RoutingStream::~RoutingStream()
{
}

PacketOut* RoutingStream::BuildRoutingPacket(Buffer* pPayload, PacketType type, const InetAddress& rAddress)
{
    Buffer* pBuffer;
    if (pPayload != 0 && pPayload->IsPooled())
    {
        pBuffer = new (RootObject::TARGET_POOL_PACKET_MANAGEMENT)
            PacketBuffer(pPayload->GetLength() + ROUTING_HEADER_SIZE, Buffer::BUFFER_FLAG_POOLED);
        if (pBuffer == 0)
            return 0;
        if (!pBuffer->IsValid())
        {
            delete pBuffer;
            return 0;
        }
    }
    else
    {
        pBuffer = new Buffer();
    }
    unsigned int uiAddress;
    unsigned short uiPort;
    ByteStream stream(pBuffer);
    pBuffer->ReleaseRef();
    uiAddress = rAddress.GetAddress();
    stream.Append(reinterpret_cast<const unsigned char*>(&uiAddress), 4, 1);
    uiPort = rAddress.GetPortNumber();
    stream.Append(reinterpret_cast<const unsigned char*>(&uiPort), 2, 1);
    stream << *pPayload;
    PacketOut* pPacket;
    if (pPayload != 0 && pPayload->IsPooled())
    {
        pPacket = new (RootObject::TARGET_POOL_PACKET_MANAGEMENT)
            PacketBufferPacketOut(0, type, 0, stream.GetBuffer(), 0);
        if (pPacket == 0 || !pPacket->IsValid())
        {
            delete pPacket;
            return 0;
        }
    }
    else
    {
        pPacket = new PacketOut(0, type, 0, stream.GetBuffer(), 0);
    }
    pPacket->m_uiVersion = 0;
    return pPacket;
}

bool RoutingStream::ReceiveIncomingPacket(unsigned short uiPort, unsigned char, Packet* pPacket)
{
    Router* pRouter = m_pTransport->GetRouter();
    Buffer* pRawBuffer = pPacket->m_pPayload;
    bool bResult = false;
    if (pRawBuffer != 0 && pRawBuffer->GetLength() > ROUTING_MAX_UNROUTABLE_SIZE)
    {
        InetAddress source;
        Buffer payload;
        {
            unsigned int uiAddress;
            unsigned short uiSourcePort;
            ByteStream stream(pRawBuffer);
            stream.ExtractPrimitive(&uiAddress);
            stream.ExtractPrimitive(&uiSourcePort);
            source.SetAddress(uiAddress);
            source.SetPortNumber(uiSourcePort);
            payload.Clear();
            payload.AppendData(stream.GetBuffer()->GetData() + stream.GetPosition(),
                               stream.GetLength() - stream.GetPosition(), ROUTING_APPEND_AT_END);
        }

        unsigned short uiType = pPacket->m_uiTypeAndFlags & PACKET_TYPE_MASK;
        if (uiType == PACKET_TYPE_DATA)
        {
            InetAddress route(source);
            m_routingTable.Add(route, pPacket->m_address, true);
            if (m_pTransport->Receive(uiPort, &payload, &route))
                bResult = true;
            else
                m_routingTable.Remove(route);
        }
        else if (uiType == ROUTING_PACKET_TYPE_ROUTED)
        {
            InetAddress target;
            InetAddress origin;
            if (source.GetAddress() == pRouter->GetRoutingIPAddressTemplate())
            {
                unsigned short uiID;
                pRouter->m_pResolver->ResolveToAddress(source.GetPortNumber(), &target);
                pRouter->m_pResolver->ResolveToID(pPacket->m_address, &uiID);
                origin.SetAddress(pRouter->GetRoutingIPAddressTemplate());
                origin.SetPortNumber(uiID);
                target.Trace(ROUTING_TRACE_FILTER);
                origin.Trace(ROUTING_TRACE_FILTER);
            }
            else
            {
                target = source;
                origin = pPacket->m_address;
            }

            bool bRoute = true;
            if (s_uiHistoryCleaningPeriod != 0)
            {
                if (m_denied.find(source) == m_denied.end() && m_denied.find(origin) == m_denied.end())
                {
                    CriticalSection& rCriticalSection = m_criticalSection;
                    rCriticalSection.Enter();
                    Time now = Time::GetTime();
                    HistoryMap::iterator it = m_history.find(std::make_pair(source, origin));
                    if (it != m_history.end())
                    {
                        it->second = now;
                    }
                    else if (s_uiHistoryLimit != 0 && GetHistoryCount(0) >= s_uiHistoryLimit)
                    {
                        bRoute = false;
                    }
                    else
                    {
                        m_history[std::make_pair(source, origin)] = now;
                    }
                    rCriticalSection.Leave();
                }
            }

            if (m_pFilter != 0)
                bRoute = m_pFilter(origin, target, pPacket);
            if (bRoute)
            {
                PacketOut* pRouted = BuildRoutingPacket(&payload, PACKET_TYPE_DATA, origin);
                if (pRouted != 0)
                {
                    if (!pRouted->IsValid())
                    {
                        delete pRouted;
                        bResult = true;
                    }
                    else
                    {
                        pRouted->m_address = target;
                        m_pTransport->Send(uiPort, m_Type, ROUTING_SOURCE_PORT, ROUTING_DESTINATION_PORT, pRouted,
                                           false);
                        pRouted->ReleaseRef();
                    }
                }
                bResult = true;
            }
        }
    }
    if (bResult)
        pPacket->ReleaseRef();
    return bResult;
}

void RoutingStream::DoWork()
{
    if (s_uiHistoryCleaningPeriod != 0)
    {
        Time now(Time::GetTime());
        if (static_cast<long long>(now - m_lastCleaning) > static_cast<long long>(s_uiHistoryCleaningPeriod))
        {
            m_lastCleaning = now;
            CleanHostRoutingHistory();
        }
    }
}

void RoutingStream::CleanHostRoutingHistory()
{
    CriticalSection& rCriticalSection = m_criticalSection;
    rCriticalSection.Enter();
    Time now(Time::GetTime());

    for (HistoryMap::iterator it = m_history.begin(); it != m_history.end();)
    {
        if (static_cast<long long>(now - it->second) > static_cast<long long>(s_uiHistoryLifetime))
            m_history.erase(it++);
        else
            ++it;
    }

    for (DenyMap::iterator it = m_denied.begin(); it != m_denied.end();)
    {
        if (static_cast<long long>(now - it->second) > static_cast<long long>(s_uiHistoryLifetime))
            m_denied.erase(it++);
        else
            ++it;
    }

    rCriticalSection.Leave();
}

unsigned int RoutingStream::GetHistoryCount(const InetAddress* pAddress)
{
    if (pAddress == 0)
        return m_history.size();

    CriticalSection& rCriticalSection = m_criticalSection;
    rCriticalSection.Enter();
    unsigned int uiCount = 0;
    for (HistoryMap::iterator it = m_history.begin(); it != m_history.end(); ++it)
    {
        if (it->first.first == *pAddress || it->first.second == *pAddress)
            ++uiCount;
    }
    rCriticalSection.Leave();
    return uiCount;
}

void RoutingStream::SetDenyForAddress(const InetAddress* pAddress)
{
    CriticalSection& rCriticalSection = m_criticalSection;
    rCriticalSection.Enter();
    m_denied[*pAddress] = Time(Time::GetTime());

    for (HistoryMap::iterator it = m_history.begin(); it != m_history.end();)
    {
        if (it->first.first == *pAddress || it->first.second == *pAddress)
            m_history.erase(it++);
        else
            ++it;
    }
    rCriticalSection.Leave();
}

}
}
