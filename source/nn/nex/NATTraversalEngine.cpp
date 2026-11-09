#include "nn/nex/NATTraversalEngine.hpp"

#include <algorithm>

#include "nn/nex/Buffer.hpp"
#include "nn/nex/ByteStream.hpp"
#include "nn/nex/ConnectionManager.hpp"
#include "nn/nex/ConnectivityManager.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/InetAddress.hpp"
#include "nn/nex/NATRelayInterface.hpp"
#include "nn/nex/NATTransportAdapter.hpp"
#include "nn/nex/NATTraversalStream.hpp"
#include "nn/nex/Network.hpp"
#include "nn/nex/JobCheckAndRelay.hpp"
#include "nn/nex/Relay.hpp"
#include "nn/nex/RootTransport.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int STATION_URL_TYPE_BIT = 1;
const unsigned long long TRACE_NAT_PROBE = 0x40000;
const NATProperties::MappingProperties NAT_MAPPING_ENDPOINT_DEPENDENT = static_cast<NATProperties::MappingProperties>(
    2);
const int PROBE_INTERVAL = 90000;
const unsigned int PROBE_MAX_COUNT = 12;
const unsigned int NAT_MESSAGE_APPEND_AT_END = 0xFFFFFFFF;

int s_iProbeRequestTimeout = 10000;
int s_iExecuteInterval = 500;
unsigned int s_uiMaxProbes = 32;
int s_iDummyPacketInterval = 10000;
unsigned char s_ucProbeFlags = 3;
unsigned int s_uiDummyPacketCount = 5;
unsigned short s_uiNATCheckPort = 33335;

inline bool IsSameStation(const StationURL& rLeft, const StationURL& rRight)
{
    return String::IsEqual(rLeft.GetAddress(), rRight.GetAddress())
           && rLeft.GetRVConnectionID() == rRight.GetRVConnectionID();
}

}

NATTraversalEngine::NATTraversalEngine()
    : m_unsupportedNATReferee(),
      m_pRelay(0),
      m_pEcho(0),
      m_pStream(0),
      m_pTransportAdapter(0),
      m_activeProbes(),
      m_completedProbes(),
      m_nextProbeTime(),
      m_pCheckAndRelayJob(0),
      m_callContext(),
      m_bSendingDummyPackets(false),
      m_nextDummyPacketTime(),
      m_uiProbeRequestCount(0),
      m_probeRequests(),
      m_relayServerAddress(L""),
      m_uiRelayServerPort(0),
      m_bUseRelayServer(false)
{
}

NATTraversalEngine::~NATTraversalEngine()
{
}

bool NATTraversalEngine::StartNATStream(RootTransport* pTransport)
{
    m_nextProbeTime = Time::GetTime() + s_iExecuteInterval;
    m_pStream = new NATTraversalStream(this, pTransport);
    return true;
}

bool NATTraversalEngine::StopNATStream()
{
    delete m_pStream;
    m_pStream = 0;
    return true;
}

bool NATTraversalEngine::RegisterEcho(NATEchoInterface* pEcho)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (m_pEcho != 0)
        return false;

    m_pEcho = pEcho;
    return true;
}

bool NATTraversalEngine::UnregisterEcho()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (m_pEcho == 0)
        return false;

    m_pEcho = 0;
    return true;
}

bool NATTraversalEngine::RegisterRelay(NATRelayInterface* pRelay)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (m_pRelay != 0)
        return false;

    m_pRelay = pRelay;
    StartNATStream(0);
    return true;
}

bool NATTraversalEngine::UnregisterRelay()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (m_pRelay == 0)
        return false;

    StopNATStream();
    m_pRelay = 0;
    return true;
}

bool NATTraversalEngine::RegisterConnectivityTester(ConnectivityTester* pConnectivityTester)
{
    if (Network::GetInstance() == 0)
        return false;

    if (&Network::GetInstance()->GetConnectivityManager() == 0)
        return false;

    return Network::GetInstance()->GetConnectivityManager().RegisterConnectivityTester(pConnectivityTester);
}

bool NATTraversalEngine::UnregisterConnectivityTester()
{
    if (Network::GetInstance() == 0)
        return false;

    if (&Network::GetInstance()->GetConnectivityManager() == 0)
        return false;

    return Network::GetInstance()->GetConnectivityManager().UnregisterConnectivityTester();
}

bool NATTraversalEngine::UnregisterTransportAdapter()
{
    if (m_pTransportAdapter == 0)
        return false;

    m_pTransportAdapter = 0;
    return true;
}

unsigned int NATTraversalEngine::GetNumOfMaxProbes()
{
    NATProperties::MappingProperties mapping = RootTransport::GetInstance()->GetNATProperties().GetNATMapping();
    if (mapping == NAT_MAPPING_ENDPOINT_DEPENDENT)
        return 1;

    return s_uiMaxProbes;
}

bool NATTraversalEngine::CanUseRelay(const StationURL& rStationURL, bool bCheckURL) const
{
    if (!RootTransport::GetInstance()->GetRelay()->IsEnabled() || String::IsEqual(m_relayServerAddress, L"")
        || m_uiRelayServerPort == 0)
        return false;

    if (bCheckURL)
    {
        if (rStationURL.GetUseRelayServer())
            return false;

        if (((rStationURL.GetType() >> STATION_URL_TYPE_BIT) & 1) == 0)
            return false;
    }

    return true;
}

bool NATTraversalEngine::ShouldUseRelay(const StationURL& rStationURL, bool bCheckURL) const
{
    if (CanUseRelay(rStationURL, bCheckURL) && m_bUseRelayServer)
        return true;

    return false;
}

bool NATTraversalEngine::PrepareNATTraversal(StationURL stationURL)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (m_pRelay == 0)
        return false;

    if (stationURL.GetRVConnectionID() == 0)
        return false;

    stationURL.Trace(TRACE_NAT_PROBE);
    stationURL.SetProbeRequestID(m_uiProbeRequestCount++);
    if (ShouldUseRelay(stationURL, false))
    {
        stationURL.Trace(TRACE_NAT_PROBE);
        stationURL.SetUseRelayServer(true);
        stationURL.SetRelayServerAddress(m_relayServerAddress);
        stationURL.SetRelayServerPort(m_uiRelayServerPort);
    }

    qList<StationURL>::iterator it;
    for (it = m_pendingURLs.begin(); it != m_pendingURLs.end(); ++it)
    {
        if (IsSameStation(*it, stationURL))
            break;
    }

    if (it == m_pendingURLs.end() && m_activeProbes.FindProbe(stationURL) == 0
        && m_completedProbes.FindProbe(stationURL) == 0)
    {
        m_pendingURLs.push_back(stationURL);
        return true;
    }

    return false;
}

bool NATTraversalEngine::PrepareNATTraversal(const qList<StationURL>& rStationURLs)
{
    {
        ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
        if (m_pRelay == 0)
            return false;
    }

    for (qList<StationURL>::const_iterator it = rStationURLs.begin(); it != rStationURLs.end(); ++it)
    {
        if (PrepareNATTraversal(*it))
            return true;
    }

    return true;
}

void NATTraversalEngine::ReportNATTraversalResult(unsigned int uiConnectionID, bool bSucceeded,
                                                  unsigned int uiRVConnectionID)
{
    if (m_pRelay != 0)
    {
        NATTraversalResult result = GetNATTraversalResult(uiConnectionID);
        m_pRelay->ReportNATTraversalResultDetail(uiConnectionID, bSucceeded, result, uiRVConnectionID);
    }
}

void NATTraversalEngine::ReportNATProperties(NATProperties::MappingProperties mapping,
                                             NATProperties::FilteringProperties filtering, unsigned int uiPortIncrease)
{
    if (m_pRelay != 0)
        m_pRelay->ReportNATProperties(mapping, filtering, uiPortIncrease);
}

void NATTraversalEngine::TimeoutProbeRequest(const Time& rTime)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    ProbeRequestMap::iterator it = m_probeRequests.begin();
    while (it != m_probeRequests.end())
    {
        if (rTime <= it->second.first)
        {
            ++it;
        }
        else
        {
            m_completedProbes.push_back(URLProbe(it->second.second, ConnectionManager::s_iNATTraversalTimeout,
                                                 PROBE_INTERVAL, PROBE_MAX_COUNT));
            ProbeRequestMap::iterator expired = it;
            ++it;
            m_probeRequests.erase(expired);
        }
    }
}

void NATTraversalEngine::ProcessRequestProbe(const StationURL& rStationURL)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (rStationURL.GetFastProbeResponse())
    {
        ProbeRequestMap::iterator it = m_probeRequests.find(rStationURL.GetProbeRequestID());
        if (it != m_probeRequests.end())
            it->second.first = Time::GetTime() + ConnectionManager::s_iNATTraversalTimeout;

        return;
    }

    if (rStationURL.GetProbeRequestInitiation())
    {
        if (m_pendingURLs.size() >= s_uiMaxProbes)
            return;

        bool bRelayFastProbe = false;
        if (!m_bUseRelayServer
            && RootTransport::GetInstance()->GetNATProperties().GetNATMapping() == NAT_MAPPING_ENDPOINT_DEPENDENT
            && (m_pendingURLs.size() != 0 || m_pCheckAndRelayJob != 0))
            bRelayFastProbe = true;

        if (bRelayFastProbe)
        {
            qList<StationURL> targetURLs;
            targetURLs.push_back(rStationURL);
            StationURL fastProbeURL;
            fastProbeURL.SetProbeRequestID(rStationURL.GetProbeRequestID());
            fastProbeURL.SetFastProbeResponse(true);
            m_pRelay->RelayProbeRequest(targetURLs, fastProbeURL);
        }

        URLProbeList::iterator completed = m_completedProbes.begin();
        while (completed != m_completedProbes.end())
        {
            if (completed->m_stationURL.GetRVConnectionID() == rStationURL.GetRVConnectionID())
            {
                completed->Trace(TRACE_NAT_PROBE);
                completed = m_completedProbes.erase(completed);
            }
            else
            {
                ++completed;
            }
        }

        URLProbeList::iterator active = m_activeProbes.begin();
        while (active != m_activeProbes.end())
        {
            if (active->m_stationURL.GetRVConnectionID() == rStationURL.GetRVConnectionID())
            {
                active->Trace(TRACE_NAT_PROBE);
                active = m_activeProbes.erase(active);
            }
            else
            {
                ++active;
            }
        }

        StationURL stationURL(rStationURL);
        if (ShouldUseRelay(stationURL, true))
        {
            stationURL.Trace(TRACE_NAT_PROBE);
            stationURL.SetUseRelayServer(true);
            stationURL.SetRelayServerAddress(m_relayServerAddress);
            stationURL.SetRelayServerPort(m_uiRelayServerPort);
        }

        m_pendingURLs.push_back(stationURL);
    }
    else
    {
        ProbeRequestMap::iterator it = m_probeRequests.find(rStationURL.GetProbeRequestID());
        if (it != m_probeRequests.end())
            m_probeRequests.erase(rStationURL.GetProbeRequestID());

        StartSendProbe(rStationURL);
    }
}

void NATTraversalEngine::SendDummyPacket(unsigned char)
{
    StationURL stationURL(String(RootTransport::GetInstance()->GetNATCheckServerAddresses()[0]->GetBuffer()));
    stationURL.SetPortNumber(s_uiNATCheckPort);
    stationURL.SetURLType(StationURL::Udp);
    Buffer* pBuffer = new Buffer();
    ByteStream stream;
    stream << Buffer(String(L"Dummy"));
    pBuffer->AppendData(stream.GetBuffer(), stream.GetBuffer()->GetLength(), NAT_MESSAGE_APPEND_AT_END);
    for (unsigned int uiCount = 0; uiCount < s_uiDummyPacketCount; ++uiCount)
        RootTransport::GetInstance()->Send(&stationURL, pBuffer);

    pBuffer->ReleaseRef();
}

void NATTraversalEngine::SendDummyPacket(const StationURL& rStationURL, unsigned char uiFlags)
{
    StationURL stationURL(rStationURL);
    stationURL.SetPortNumber(s_uiNATCheckPort);
    stationURL.SetURLType(StationURL::Udp);
    Buffer* pBuffer = new Buffer();
    ByteStream stream;
    stream << Buffer(String(L"Dummy"));
    pBuffer->AppendData(stream.GetBuffer(), stream.GetBuffer()->GetLength(), NAT_MESSAGE_APPEND_AT_END);
    for (unsigned int uiCount = 0; uiCount < s_uiDummyPacketCount; ++uiCount)
    {
        if (m_pTransportAdapter != 0)
        {
            m_pTransportAdapter->Send(stationURL, pBuffer->GetData(), pBuffer->GetLength(), uiFlags);
        }
        else if (m_pStream != 0)
        {
            m_pStream->SendMsg(stationURL, pBuffer, uiFlags);
        }
    }

    pBuffer->ReleaseRef();
}

void NATTraversalEngine::CheckPortAndRelay(const qList<StationURL>& rLocalURLs, const StationURL& rStationURL)
{
    if (m_callContext.GetState() != CallContext::Ready)
        m_callContext.Reset();

    m_pCheckAndRelayJob = new JobCheckAndRelay(&m_callContext, rLocalURLs, rStationURL);
    Scheduler::GetInstance()->Queue(m_pCheckAndRelayJob, false);
}

void NATTraversalEngine::SendDummyPacket(const InetAddress& rAddress)
{
    StationURL stationURL;
    stationURL.SetInetAddress(&rAddress);
    stationURL.SetURLType(StationURL::Udp);
    ByteStream stream;
    unsigned char uiMessage = 0;
    stream.Append(&uiMessage, 1, 1);
    stream.Append(reinterpret_cast<const unsigned char*>(&m_uiConnectionID), 4, 1);
    Time time = Time::GetTime();
    stream.Append(&time);
    Buffer* pBuffer = stream.GetBuffer();
    for (unsigned int uiCount = 0; uiCount < s_uiDummyPacketCount; ++uiCount)
    {
        if (m_pTransportAdapter != 0)
        {
            m_pTransportAdapter->Send(stationURL, pBuffer->GetData(), pBuffer->GetLength(), 0);
        }
        else if (m_pStream != 0)
        {
            m_pStream->SendMsg(stationURL, pBuffer, 0);
        }
    }
}

void NATTraversalEngine::StartSendProbe(const StationURL& rStationURL)
{
    URLProbe* pProbe = m_activeProbes.FindProbe(rStationURL);
    if (pProbe == 0)
    {
        StationURL stationURL(rStationURL);
        stationURL.Trace(TRACE_NAT_PROBE);
        if (RootTransport::GetInstance()->GetRelay()->IsEnabled() && stationURL.GetUseRelayServer())
        {
            if (String::IsEqual(stationURL.GetRelayAddress(), L"") || stationURL.GetRelayPort() == 0
                || String::IsEqual(stationURL.GetRelayServerAddress(), L"") || stationURL.GetRelayServerPort() == 0)
            {
                stationURL.Trace(TRACE_NAT_PROBE);
                m_completedProbes.push_back(
                    URLProbe(stationURL, ConnectionManager::s_iNATTraversalTimeout, PROBE_INTERVAL, PROBE_MAX_COUNT));
                return;
            }

            stationURL.SetAddress(stationURL.GetRelayAddress());
            stationURL.SetPortNumber(stationURL.GetRelayPort());

            InetAddress relayAddress;
            relayAddress.SetAddress(stationURL.GetRelayAddress());
            relayAddress.SetPortNumber(stationURL.GetRelayPort());

            InetAddress relayServerAddress;
            relayServerAddress.SetAddress(stationURL.GetRelayServerAddress());
            relayServerAddress.SetPortNumber(stationURL.GetRelayServerPort());

            RootTransport::GetInstance()->GetRelay()->AddRelay(relayAddress, relayServerAddress, true);
            SendDummyPacket(relayServerAddress);
        }

        m_activeProbes.push_back(
            URLProbe(stationURL, ConnectionManager::s_iNATTraversalTimeout, PROBE_INTERVAL, PROBE_MAX_COUNT));
    }
    else
    {
        rStationURL.Trace(TRACE_NAT_PROBE);
        pProbe->m_expirationTime = Time::GetTime() + ConnectionManager::s_iNATTraversalTimeout;
    }
}

void NATTraversalEngine::SendProbeRequests()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    while (!m_pendingURLs.empty()
           && (m_bUseRelayServer || (GetNumOfSendingProbes() < GetNumOfMaxProbes() && m_pCheckAndRelayJob == 0)))
    {
        StationURL& rStationURL = m_pendingURLs.front();
        String publicAddress = RootTransport::GetInstance()->GetNATProperties().GetPublicAddress();
        bool bSamePublicAddress = String::IsEqual(rStationURL.GetInetAddress()->GetAddressStr(), publicAddress);
        bool bNotInitiator = !rStationURL.GetProbeRequestInitiation();

        qList<StationURL> localURLs;
        {
            qProtectedList<StationURL>& rLocalURLs = Network::GetInstance()->GetLocalURLs();
            ScopedCS localLock(&rLocalURLs.GetCriticalSection());
            localURLs = rLocalURLs.GetList();
        }

        for (qList<StationURL>::iterator it = localURLs.begin(); it != localURLs.end(); ++it)
        {
            const StationURL& rLocalURL = *it;
            if (((rLocalURL.GetType() >> STATION_URL_TYPE_BIT) & 1) == 0)
            {
                if (!bSamePublicAddress)
                    continue;

                if (bNotInitiator && rStationURL.GetUseRelayServer())
                    continue;
            }

            StationURL probeURL(*it);
            probeURL.SetRVConnectionID(m_uiConnectionID);
            probeURL.SetProbeRequestInitiation(bNotInitiator);
            probeURL.SetProbeRequestID(rStationURL.GetProbeRequestID());
            if (Network::GetInstance()->GetTransport()->GetRelay()->IsEnabled())
            {
                InetAddress relayAddress(Network::GetInstance()->GetTransport()->GetRelay()->GetSrcPublicAddress());
                probeURL.SetRelayAddress(relayAddress.GetAddressStr());
                probeURL.SetRelayPort(relayAddress.GetPortNumber());
            }

            qList<StationURL> targetURLs;
            targetURLs.push_back(rStationURL);
            if (((rLocalURL.GetType() >> STATION_URL_TYPE_BIT) & 1) != 0
                && RootTransport::GetInstance()->GetNATProperties().GetNATMapping() == NAT_MAPPING_ENDPOINT_DEPENDENT
                && !rStationURL.GetUseRelayServer())
            {
                if (RootTransport::GetInstance()->GetNATProperties().m_bMappingPortIncrementValid)
                    SendDummyPacket(rStationURL, s_ucProbeFlags);

                CheckPortAndRelay(targetURLs, probeURL);
            }
            else
            {
                if (rStationURL.GetUseRelayServer() && ((rLocalURL.GetType() >> STATION_URL_TYPE_BIT) & 1) != 0)
                {
                    probeURL.SetRelayServerAddress(rStationURL.GetRelayServerAddress());
                    probeURL.SetRelayServerPort(rStationURL.GetRelayServerPort());
                    probeURL.SetUseRelayServer(true);
                    m_pRelay->RelayProbeRequest(targetURLs, probeURL);
                }
                else if (((rLocalURL.GetType() >> STATION_URL_TYPE_BIT) & 1) != 0)
                {
                    probeURL.SetPortNumber(RootTransport::GetInstance()->GetNATProperties().GetPublicPort());
                    m_pRelay->RelayProbeRequest(targetURLs, probeURL);
                }
                else
                {
                    probeURL.SetPortNumber(RootTransport::GetWellKnownPortNumber());
                    m_pRelay->RelayProbeRequest(targetURLs, probeURL);
                }

                if (!bNotInitiator)
                    StartSendProbe(rStationURL);
            }
        }

        if (bNotInitiator)
        {
            m_probeRequests[rStationURL.GetProbeRequestID()] = std::make_pair(Time::GetTime() + s_iProbeRequestTimeout,
                                                                              rStationURL);
        }

        m_pendingURLs.erase(m_pendingURLs.begin());
    }
}

void NATTraversalEngine::SendProbe(Msg msg, const StationURL& rStationURL, Time time, unsigned char uiFlags)
{
    ByteStream stream;
    unsigned char uiMessage = msg;
    stream.Append(&uiMessage, 1, 1);
    stream.Append(reinterpret_cast<const unsigned char*>(&m_uiConnectionID), 4, 1);
    stream.Append(&time);
    if (msg == MSG_PROBE_REPLY)
    {
        unsigned short uiPort = rStationURL.GetPortNumber();
        stream.Append(reinterpret_cast<const unsigned char*>(&uiPort), 2, 1);
    }

    if (m_pTransportAdapter != 0)
    {
        m_pTransportAdapter->Send(rStationURL, stream.GetBuffer()->GetData(), stream.GetBuffer()->GetLength(), uiFlags);
    }
    else
    {
        m_pStream->SendMsg(rStationURL, stream.GetBuffer(), uiFlags);
    }
}

void NATTraversalEngine::SendProbes()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    for (URLProbeList::iterator it = m_activeProbes.begin(); it != m_activeProbes.end(); ++it)
    {
        URLProbe& rProbe = *it;
        if (rProbe.UpdateIsNeeded(Time::GetTime()))
        {
            bool bNormalProbe = true;
            if (rProbe.m_lastUpdateTime == Time() && rProbe.m_uiProbeCount <= 7)
            {
                if (rProbe.m_stationURL.GetProbeRequestInitiation())
                {
                    if (!rProbe.m_stationURL.GetUseRelayServer())
                        bNormalProbe = false;
                }
            }

            unsigned char uiFlags = 0;
            if (!bNormalProbe)
                uiFlags = s_ucProbeFlags;

            unsigned char uiCount;
            if (rProbe.m_uiProbeCount == 0 && !bNormalProbe)
                uiCount = 3;
            else
                uiCount = 1;

            for (unsigned char i = 0; i < uiCount; ++i)
                SendProbe(MSG_PROBE, rProbe.m_stationURL, Time::GetTime(), uiFlags);

            ++rProbe.m_uiProbeCount;
        }
    }
}

void NATTraversalEngine::StopProbes()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    std::list<unsigned int, MemAllocator<unsigned int> > completedIDs;
    std::list<StationURL, MemAllocator<StationURL> > relayURLs;

    URLProbeList::iterator probe = m_activeProbes.begin();
    while (probe != m_activeProbes.end())
    {
        if (probe->UpdateIsNeeded(Time(Time::GetTime())) && probe->m_uiPingTime == URLProbe::UNREACHED_PING_TIME)
        {
            ++probe;
            continue;
        }

        probe->Trace(TRACE_NAT_PROBE);
        if (probe->m_uiPingTime == URLProbe::UNREACHED_PING_TIME)
        {
            if (!CanUseRelay(probe->m_stationURL, true) || probe->m_stationURL.GetProbeRequestInitiation())
                m_completedProbes.push_back(*probe);
            else
                relayURLs.push_back(probe->m_stationURL);

            bool bUseRelay = false;
            if (RootTransport::GetInstance()->GetNATProperties().GetNATMapping() == NAT_MAPPING_ENDPOINT_DEPENDENT)
            {
                bUseRelay = false;
                if (RootTransport::GetInstance()->GetRelay()->IsEnabled())
                {
                    if (!String::IsEqual(m_relayServerAddress, L"") && m_uiRelayServerPort != 0)
                        bUseRelay = true;
                }
            }

            if (bUseRelay)
            {
                m_bUseRelayServer = true;
                for (qList<StationURL>::iterator url = m_pendingURLs.begin(); url != m_pendingURLs.end(); ++url)
                {
                    if (!url->GetUseRelayServer() && ((url->GetType() >> STATION_URL_TYPE_BIT) & 1) != 0)
                    {
                        url->SetUseRelayServer(true);
                        url->SetRelayServerAddress(m_relayServerAddress);
                        url->SetRelayServerPort(m_uiRelayServerPort);
                    }
                }
            }

            URLProbeList::iterator failed = probe;
            probe = m_activeProbes.erase(failed);
        }
        else
        {
            unsigned int uiConnectionID = probe->m_stationURL.GetRVConnectionID();
            qList<StationURL>::iterator url = m_pendingURLs.begin();
            while (url != m_pendingURLs.end())
            {
                if (url->GetRVConnectionID() == uiConnectionID)
                {
                    url->Trace(TRACE_NAT_PROBE);
                    url = m_pendingURLs.erase(url);
                }
                else
                {
                    ++url;
                }
            }

            completedIDs.push_back(uiConnectionID);
            m_completedProbes.push_back(*probe);
            URLProbeList::iterator succeeded = probe;
            probe = m_activeProbes.erase(succeeded);
        }
    }

    probe = m_activeProbes.begin();
    while (!completedIDs.empty() && probe != m_activeProbes.end())
    {
        if (std::find(completedIDs.begin(), completedIDs.end(), probe->m_stationURL.GetRVConnectionID())
            != completedIDs.end())
        {
            probe->Trace(TRACE_NAT_PROBE);
            m_completedProbes.push_back(*probe);
            URLProbeList::iterator moved = probe;
            probe = m_activeProbes.erase(moved);
        }
        else
        {
            ++probe;
        }
    }

    probe = m_completedProbes.begin();
    while (probe != m_completedProbes.end())
    {
        if (Time::GetTime() <= probe->m_expirationTime)
        {
            ++probe;
        }
        else
        {
            probe->Trace(TRACE_NAT_PROBE);
            URLProbeList::iterator expired = probe;
            probe = m_completedProbes.erase(expired);
        }
    }

    for (std::list<StationURL, MemAllocator<StationURL> >::iterator url = relayURLs.begin(); url != relayURLs.end();
         ++url)
    {
        url->Trace(TRACE_NAT_PROBE);
        url->SetUseRelayServer(true);
        url->SetRelayServerAddress(m_relayServerAddress);
        url->SetRelayServerPort(m_uiRelayServerPort);
        m_pendingURLs.push_front(*url);
    }
}

void NATTraversalEngine::Execute()
{
    Time currentTime(g_lastTime);
    if (m_bSendingDummyPackets)
    {
        if (m_nextDummyPacketTime < currentTime)
        {
            SendDummyPacket(0);
            m_nextDummyPacketTime = currentTime + s_iDummyPacketInterval;
        }
    }

    if (currentTime > m_nextProbeTime)
    {
        SendProbeRequests();
        SendProbes();
        StopProbes();
        m_nextProbeTime = currentTime + s_iExecuteInterval;
        CallContext::State state = m_callContext.GetState();
        if (state == CallContext::CallFailed || state == CallContext::CallSucceeded)
        {
            m_pCheckAndRelayJob = 0;
            m_callContext.Reset();
        }

        TimeoutProbeRequest(currentTime);
    }
}

void NATTraversalEngine::ReceiveMessage(const StationURL& rStationURL, const unsigned char* pData, unsigned int uiSize)
{
    rStationURL.Trace(TRACE_NAT_PROBE);
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    Buffer buffer;
    buffer.AppendData(pData, uiSize, NAT_MESSAGE_APPEND_AT_END);
    ByteStream stream(&buffer);
    Time pingSendTime;
    unsigned char uiMessage;
    unsigned int uiConnectionID;
    unsigned short uiPublicPort;
    stream.ExtractPrimitive(&uiMessage);
    stream.ExtractPrimitive(&uiConnectionID);
    stream.Extract(&pingSendTime);
    if (uiMessage == MSG_PROBE_REPLY)
        stream.ExtractPrimitive(&uiPublicPort);

    if (!stream.HasError())
    {
        StationURL stationURL(rStationURL);
        stationURL.SetRVConnectionID(uiConnectionID);
        URLProbe* pProbe = m_activeProbes.FindProbe(stationURL);
        if (pProbe == 0)
            pProbe = m_completedProbes.FindProbe(stationURL);

        if (pProbe == 0)
        {
            if (m_activeProbes.FindProbe(uiConnectionID) != 0
                && ((stationURL.GetType() >> STATION_URL_TYPE_BIT) & 1) == 0)
                StartSendProbe(stationURL);
        }
        else
        {
            pProbe->Trace(TRACE_NAT_PROBE);
        }

        if (uiMessage == MSG_PROBE)
        {
            if (pProbe != 0)
                pProbe->Update(stationURL, Time());

            SendProbe(MSG_PROBE_REPLY, stationURL, pingSendTime, 0);
        }
        else if (uiMessage == MSG_PROBE_REPLY && pProbe != 0)
        {
            bool bUpdatePublicPort = false;
            if (RootTransport::GetInstance() != 0
                && RootTransport::GetInstance()->GetNATProperties().GetNATMapping() == NAT_MAPPING_ENDPOINT_DEPENDENT
                && ((pProbe->m_stationURL.GetType() >> STATION_URL_TYPE_BIT) & 1) != 0
                && !pProbe->m_stationURL.GetProbeRequestInitiation())
                bUpdatePublicPort = true;

            if (bUpdatePublicPort)
            {
                unsigned short uiCurrentPort = RootTransport::GetInstance()->GetNATProperties().GetPublicPort();
                if (uiCurrentPort != uiPublicPort)
                {
                    NATProperties properties = RootTransport::GetInstance()->GetNATProperties();
                    properties.SetPublicPort(uiPublicPort);
                    RootTransport::GetInstance()->SetNATProperties(properties);
                }
            }

            pProbe->Update(stationURL, pingSendTime);
        }
    }
}

bool NATTraversalEngine::Terminate()
{
    if (m_callContext.GetState() == CallContext::CallInProgress)
        m_callContext.Cancel(CallContext::CallCanceled);

    return true;
}

bool NATTraversalEngine::RegisterTransportAdapter(NATTransportAdapter* pTransportAdapter)
{
    if (m_pTransportAdapter != 0)
        return false;

    m_pTransportAdapter = pTransportAdapter;
    return true;
}

void NATTraversalEngine::StartSendDummyPacket()
{
    m_bSendingDummyPackets = true;
    m_nextDummyPacketTime = Time(Time::GetTime());
}

void NATTraversalEngine::StopSendDummyPacket()
{
    m_bSendingDummyPackets = false;
}

int NATTraversalEngine::GetNumOfSendingProbes()
{
    int iCount = 0;
    for (URLProbeList::iterator it = m_activeProbes.begin(); it != m_activeProbes.end(); ++it)
    {
        if ((it->m_stationURL.GetType() >> STATION_URL_TYPE_BIT) & 1)
            ++iCount;
    }
    return iCount;
}

void NATTraversalEngine::ClearProbeList()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    m_activeProbes.clear();
    m_completedProbes.clear();
}

int NATTraversalEngine::GetURLPingTime(const StationURL& rStationURL)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    URLProbe* pProbe = m_completedProbes.FindProbe(rStationURL);
    if (pProbe != 0)
        return pProbe->m_uiPingTime;

    return -1;
}

int NATTraversalEngine::GetCIDPingTime(unsigned int uiConnectionID)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    URLProbe* pProbe = m_completedProbes.FindProbe(uiConnectionID);
    if (pProbe != 0)
        return pProbe->m_uiPingTime;

    return -1;
}

bool NATTraversalEngine::GetUpdatedURL(const StationURL& rStationURL, StationURL* pUpdatedURL)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    URLProbe* pProbe = m_completedProbes.FindProbe(rStationURL);
    if (pProbe != 0 && pProbe->m_stationURL != rStationURL)
    {
        *pUpdatedURL = pProbe->m_stationURL;
        return true;
    }

    return false;
}

bool NATTraversalEngine::GetProbeFailed(const StationURL& rStationURL)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    URLProbe* pProbe = m_completedProbes.FindProbe(rStationURL);
    if (pProbe != 0)
        return pProbe->m_uiPingTime == URLProbe::UNREACHED_PING_TIME;

    if (m_activeProbes.FindProbe(rStationURL) != 0)
        return false;

    for (qList<StationURL>::iterator it = m_pendingURLs.begin(); it != m_pendingURLs.end(); ++it)
    {
        if (String::IsEqual(it->GetAddress(), rStationURL.GetAddress())
            && it->GetRVConnectionID() == rStationURL.GetRVConnectionID())
            return false;
    }

    for (ProbeRequestMap::iterator it = m_probeRequests.begin(); it != m_probeRequests.end(); it++)
    {
        const StationURL& rProbeURL = it->second.second;
        if (String::IsEqual(rProbeURL.GetAddress(), rStationURL.GetAddress())
            && rProbeURL.GetRVConnectionID() == rStationURL.GetRVConnectionID())
            return false;
    }

    return true;
}

NATTraversalResult NATTraversalEngine::GetNATTraversalResult(unsigned int uiConnectionID)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    URLProbe* pProbe = m_completedProbes.FindProbe(uiConnectionID);
    if (pProbe == 0)
        return NAT_TRAVERSAL_UNKNOWN;

    if (pProbe->m_uiPingTime != URLProbe::UNREACHED_PING_TIME)
    {
        if (pProbe->m_stationURL.GetUseRelayServer())
            return NAT_TRAVERSAL_SUCCEEDED_VIA_RELAY;

        return NAT_TRAVERSAL_SUCCEEDED;
    }

    if (pProbe->m_uiProbeCount == 0)
        return NAT_TRAVERSAL_NOT_ATTEMPTED;

    if (pProbe->m_stationURL.GetUseRelayServer())
        return NAT_TRAVERSAL_FAILED_VIA_RELAY;

    return NAT_TRAVERSAL_FAILED;
}

}
}
