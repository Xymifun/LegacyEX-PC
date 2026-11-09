#include "nn/pia/nat/NatTraversalProtocol.hpp"

#include "nn/pia/common/InetAddress.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/common/StationAddress.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/inet/Api.hpp"
#include "nn/pia/nat/Api.hpp"
#include "nn/pia/nat/NatPortDetecter.hpp"
#include "nn/pia/nat/NatProbe.hpp"
#include "nn/pia/nat/NatProbeData.hpp"
#include "nn/pia/nat/NatProbeRequest.hpp"
#include "nn/pia/nat/NatTraversalTime.hpp"
#include "nn/pia/nat/NatTraverser.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/ProcessUpdateMeshJob.hpp"
#include "nn/pia/transport/PacketHandler.hpp"
#include "nn/pia/transport/ProtocolMessageWriter.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"

namespace nn
{
namespace pia
{
namespace nat
{

namespace
{

inline void setDeadline(common::Time& deadline, int msec)
{
    const long long span = common::TimeSpan::GetTicksPerMSec().m_tick * msec;
    common::Time now;
    now.SetNow();
    deadline.m_tick = now.m_tick + span;
}

inline common::Time getDeadline(int msec)
{
    const common::TimeSpan span = common::TimeSpan::FromMSec(msec);
    common::Time now;
    now.SetNow();
    return now + span;
}

}

static void NatPortDetectionCompletionCallback(Result result, void* param);

NatTraversalProtocol::NatTraversalProtocol()
    : m_localServiceConnectionId(0),
      m_localLocation(),
      m_serverLocation(),
      m_callContext(),
      m_probeList(),
      m_edmProbeRequests(),
      m_probeRequests(),
      m_traversalTimes(),
      m_nextDispatchTime(),
      m_pendingDetectionDeadline(),
      m_fieldC58(false),
      m_fieldC59(false),
      m_fieldC5A(false),
      m_fieldC5B(false),
      m_fieldC5C(false),
      m_pendingConnectionId(0),
      m_portDetectionLocation(),
      m_serverKeepAliveEnabled(false),
      m_serverKeepAliveTime(),
      m_ownerNatTraverser(0),
      m_random()
{
    m_probeList.Clear();
    m_edmProbeRequests.Clear();
    m_probeRequests.Clear();
    m_traversalTimes.Clear();
}

NatTraversalProtocol::~NatTraversalProtocol()
{
    m_probeList.ReleaseAll();
    m_edmProbeRequests.ReleaseAll();
    m_probeRequests.ReleaseAll();
    m_traversalTimes.ReleaseAll();
}

Result NatTraversalProtocol::StartupNatTraversal(NatTraverser* traverser)
{
    if (traverser == 0)
        return cResultInvalidArgument;
    if (traverser->m_servers.begin() == traverser->m_servers.end())
        return cResultInvalidState;

    common::InetAddress address = *traverser->m_servers.begin();
    address.m_port = cNatServerPort;
    m_serverLocation.m_stationAddress.SetInetAddress(address);
    m_serverLocation.Trace(cTraceBrief);
    if (!m_serverLocation.m_stationAddress.m_inetAddress.IsValid())
    {
        m_serverLocation.Trace(cTraceError);
        return cResultInvalidState;
    }

    m_ownerNatTraverser = traverser;
    m_serverKeepAliveEnabled = false;
    m_fieldC58 = false;
    m_fieldC5A = false;
    m_fieldC5C = false;
    m_fieldC5B = false;
    m_pendingConnectionId = 0;
    m_fieldC59 = false;
    m_localServiceConnectionId = traverser->GetLocalServiceConnectionId();
    g_natLocalConnectionId = m_localServiceConnectionId;
    return 0;
}

void NatTraversalProtocol::CancelPortDetection()
{
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
    {
        unsigned int state = common::CallContext::cStateInProgress;
        if (m_ownerNatTraverser != 0)
        {
            if (m_ownerNatTraverser->GetNatPortDetecter() != 0)
                m_ownerNatTraverser->GetNatPortDetecter()->CancelDetectionJob();
            state = m_callContext.m_state;
        }
        if (state == common::CallContext::cStateInProgress)
            m_callContext.SignalCancel();
    }
}

void NatTraversalProtocol::CleanupNatTraversal()
{
    m_probeList.ReleaseAll();
    m_edmProbeRequests.ReleaseAll();
    m_probeRequests.ReleaseAll();
    m_traversalTimes.ReleaseAll();
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        CancelPortDetection();
    m_callContext.Reset();
    m_fieldC5C = false;
    m_pendingConnectionId = 0;
    m_serverKeepAliveEnabled = false;
    m_ownerNatTraverser = 0;
    m_fieldC58 = false;
    m_fieldC59 = false;
    m_fieldC5A = false;
    m_fieldC5B = false;
}

void NatTraversalProtocol::AddProbeRequest(const NatProbeRequest& request)
{
    if (request.m_stationLocation.m_field1c == 0)
    {
        request.Trace(cTraceBrief);
        return;
    }

    request.Trace(cTraceBrief);
    if (m_edmProbeRequests.GetFreeNum() != 0)
    {
        NatProbeRequest* element = m_edmProbeRequests.Allocate();
        if (element != 0)
        {
            element->m_stationLocation = request.m_stationLocation;
            element->m_stationConnectionInfo = request.m_stationConnectionInfo;
            element->m_probeRequested = request.m_probeRequested;
            element->m_addressValid = request.m_addressValid;
            element->m_deadline = request.m_deadline;
            element->m_callContext = request.m_callContext;
        }
    }
    if (request.m_probeRequested)
    {
        if (m_probeRequests.GetFreeNum() != 0)
        {
            NatProbeRequest* element = m_probeRequests.Allocate();
            if (element != 0)
            {
                element->m_stationLocation = request.m_stationLocation;
                element->m_stationConnectionInfo = request.m_stationConnectionInfo;
                element->m_probeRequested = request.m_probeRequested;
                element->m_addressValid = request.m_addressValid;
                element->m_deadline = request.m_deadline;
                element->m_callContext = request.m_callContext;
            }
        }
    }
    m_nextDispatchTime = common::Time();
}

bool NatTraversalProtocol::PrepareNatTraversal(const transport::StationLocation& location,
                                               const transport::StationConnectionInfo& connection,
                                               common::CallContext* context, bool passive, bool force)
{
    if (location.m_field1c == 0)
        return false;

    transport::StationLocation requestLocation(location);
    bool shiftPort = false;
    if (force)
    {
        unsigned char threshold = GetDebugSetting()->options[0];
        if (static_cast<int>((static_cast<unsigned long long>(m_random.getU32()) * 100) >> 32) < threshold)
            shiftPort = true;
    }
    else
    {
        unsigned char threshold = GetDebugSetting()->options[1];
        if (static_cast<int>((static_cast<unsigned long long>(m_random.getU32()) * 100) >> 32) < threshold)
            shiftPort = true;
    }
    if (shiftPort && !passive)
    {
        common::StationAddress address(location.m_stationAddress);
        common::InetAddress inetAddress(address.m_inetAddress);
        unsigned int port = static_cast<unsigned short>(inetAddress.m_port + 10);
        unsigned int minPort = m_ownerNatTraverser->GetRandomPortRangeMin();
        unsigned int maxPort = m_ownerNatTraverser->GetRandomPortRangeMax();
        if (port < minPort || port > maxPort)
            port = static_cast<unsigned short>(minPort + inetAddress.m_port - maxPort + 9);
        inetAddress.m_port = static_cast<unsigned short>(port);
        address.SetInetAddress(inetAddress);
        requestLocation.SetStationAddress(address);
        requestLocation.m_field22 = cInvalidIndex;
    }

    NatProbeRequest request(requestLocation, connection, true, passive, context);
    request.Trace(cTraceBrief);
    m_probeList.Trace(cTraceVerbose);
    NatProbe* probe = m_probeList.FindByAddressPortAndConnectionId(location);
    bool reset = false;
    if (probe != 0)
    {
        if (probe->m_rtt != -1)
            reset = true;
    }
    else
    {
        probe = m_probeList.FindByAddressAndConnectionId(location);
        if (probe != 0 && probe->m_sent)
            reset = true;
    }

    if (!reset)
    {
        AddProbeRequest(request);
        return true;
    }
    setDeadline(probe->m_deadline, g_natProbeDurationMsec);
    probe->m_probeCount = 0;
    probe->m_sent = 0;
    location.Trace(cTraceVerbose);
    return false;
}

bool NatTraversalProtocol::EraseProbeRequestByConnectionId(unsigned int connectionId)
{
    bool erased = false;
    common::ListNode* node = m_probeRequests.Begin();
    common::ListNode* next = node->next;
    while (node != m_probeRequests.End())
    {
        NatProbeRequest* request = NatProbeRequestList::RequestOf(node);
        if (static_cast<unsigned int>(request->m_stationLocation.m_field1c) == connectionId)
        {
            m_probeRequests.Release(request);
            erased = true;
        }
        node = next;
        next = node->next;
    }
    node = m_edmProbeRequests.Begin();
    next = node->next;
    while (node != m_edmProbeRequests.End())
    {
        NatProbeRequest* request = NatProbeRequestList::RequestOf(node);
        if (static_cast<unsigned int>(request->m_stationLocation.m_field1c) == connectionId)
        {
            m_edmProbeRequests.Release(request);
            erased = true;
        }
        node = next;
        next = node->next;
    }
    return erased;
}

void NatTraversalProtocol::ClearNatTraversal(const transport::StationLocation& location, bool relay)
{
    if (EraseProbeRequestByConnectionId(location.m_field1c))
    {
        if (m_pendingConnectionId == static_cast<unsigned int>(location.m_field1c))
        {
            if (m_fieldC59)
            {
                m_fieldC59 = false;
                m_pendingConnectionId = 0;
                m_fieldC58 = false;
            }
            if (m_fieldC5A)
            {
                m_fieldC5A = false;
                m_pendingConnectionId = 0;
                m_fieldC58 = false;
            }
            if (m_callContext.m_state == common::CallContext::cStateInProgress)
                CancelPortDetection();
            m_fieldC5C = false;
        }
    }
    if (relay)
    {
        m_probeList.RemoveSameAddressPortAndConnectionIdProbes(location);
        m_traversalTimes.Remove(location);
        return;
    }

    NatProbe* probe = m_probeList.FindByConnectionId(location.m_field1c);
    if (probe != 0 && probe->m_rtt != -1)
    {
        probe->m_sent = 1;
        probe->m_probeCount = 1;
        setDeadline(probe->m_deadline, g_natClearProbeDelayMsec);
        probe->Trace(cTraceBrief);
        return;
    }
    m_probeList.RemoveProbesByConnectionId(location.m_field1c);
    m_traversalTimes.Remove(location);
}

void NatTraversalProtocol::ProcessProbeRequest(const transport::StationLocation& location)
{
    if (!location.m_stationAddress.m_inetAddress.IsValid())
    {
        location.Trace(cTraceError);
        return;
    }

    bool pendingRequest = false;
    if (location.m_field26 != 0)
    {
        NatProbeRequest request(location);
        AddProbeRequest(request);
        NatTraversalTime* traversalTime = m_traversalTimes.Find(location);
        if (traversalTime != 0)
        {
            traversalTime->m_endTime.SetNow();
            traversalTime->m_count = 0;
        }
        else
        {
            NatTraversalTime newTraversalTime;
            newTraversalTime.m_location = location;
            newTraversalTime.m_endTime.SetNow();
            newTraversalTime.m_count = 0;
            newTraversalTime.m_port = 0;
            m_traversalTimes.Add(newTraversalTime);
        }
    }
    else
    {
        if (inet::IsEdmMapping(m_localLocation) && m_fieldC58
            && m_pendingConnectionId == static_cast<unsigned int>(location.m_field1c))
        {
            pendingRequest = true;
            m_fieldC58 = false;
        }
        NatTraversalTime* traversalTime = m_traversalTimes.Find(location);
        if (traversalTime != 0)
        {
            common::Time now;
            now.SetNow();
            traversalTime->m_count = static_cast<unsigned int>(
                static_cast<long long>(now.m_tick - traversalTime->m_endTime.m_tick)
                / common::TimeSpan::GetTicksPerMSec().m_tick);
        }
    }

    const unsigned char sprayRange = inet::IsEdmMapping(location) && !location.m_inetAddress.IsValid() ? 2 : 0;
    NatProbe probe(location, common::Time(), common::TimeSpan::FromMSec(g_natProbeDurationMsec), 0,
                   g_natProbeSprayCount, sprayRange);
    if (pendingRequest)
    {
        setDeadline(m_pendingDetectionDeadline, 5000);
        probe.m_received = 1;
        probe.Trace(cTraceBrief);
    }
    m_probeList.AddProbe(probe);
}

Result NatTraversalProtocol::SendMessage(transport::ProtocolId protocolId, const common::StationAddress& address,
                                         const void* payload, unsigned int payloadSize, bool relay, unsigned char type)
{
    transport::ProtocolMessageWriter* writer = mStationPacketHandler->AssignByStationAddress(protocolId, address,
                                                                                             payloadSize, relay, false);
    if (writer == 0)
        return 0;
    if (relay && type != 0)
        writer->m_priority = type;
    writer->SetPayload(payload);
    mStationPacketHandler->Commit();
    return 0;
}

void NatTraversalProtocol::SendProbe(ProbeType type, const transport::StationLocation& location, common::Time time,
                                     unsigned char count)
{
    NatProbeData data;
    data.m_time = time;
    data.m_connectionId = m_localServiceConnectionId;
    data.m_type = static_cast<unsigned char>(type);
    data.Trace(cTraceVerbose);
    location.Trace(cTraceBrief);
    unsigned short port = 1;
    if (data.m_type == 1)
        port = 2;
    unsigned int size;
    unsigned char buffer[16];
    Result result = data.Serialize(buffer, &size, sizeof(buffer));
    if (result == 0)
    {
        port |= cProtocolType << 8;
        SendMessage(transport::ProtocolId(port), location.m_stationAddress, buffer, sizeof(buffer), true, count);
    }
}

void NatTraversalProtocol::ReceiveMessage(const transport::ProtocolMessageReader& reader)
{
    reader.GetProtocolIdPort();
    if (reader.m_messageSize != cProbeMessageSize)
        return;

    NatProbeData data;
    Result result = data.Deserialize(reader.m_message + transport::ProtocolMessageReader::cMessageHeaderSize,
                                     cProbeMessageSize);
    if (result != 0)
        return;
    data.Trace(cTraceVerbose);
    if (data.m_connectionId == 0)
        return;

    transport::StationLocation location;
    location.SetStationAddress(reader.m_stationAddress);
    location.m_field1c = data.m_connectionId;
    location.Trace(cTraceDetail);
    if (!location.m_stationAddress.m_inetAddress.IsValid())
    {
        location.Trace(cTraceError);
        return;
    }

    switch (data.m_type)
    {
    case cProbeType0:
        if (m_probeList.ProcessProbe(location, reader.m_field39) && m_localServiceConnectionId != 0)
            SendProbe(cProbeType1, location, data.m_time, 0);
        break;
    case cProbeType1:
        m_probeList.ProcessProbeReply(location, data.m_time, reader.m_field39);
        if (m_pendingConnectionId == data.m_connectionId)
        {
            EraseProbeRequestByConnectionId(data.m_connectionId);
            m_pendingConnectionId = 0;
            m_fieldC59 = false;
            m_fieldC58 = false;
            m_fieldC5A = false;
        }
        break;
    }
}

void NatTraversalProtocol::SendDummyPacket(const transport::StationLocation& location, unsigned char type,
                                           unsigned char count)
{
    if (!location.m_stationAddress.m_inetAddress.IsValid())
    {
        location.Trace(cTraceBrief);
        return;
    }

    const char payload[] = "NATTestId_Dummy";
    for (int i = 0; i < count; ++i)
        SendMessage(transport::ProtocolId(cProtocolType, 3), location.m_stationAddress, payload, sizeof(payload), true,
                    type);
}

NatProbeRequest* NatTraversalProtocol::FindProbeRequestByConnectionId(unsigned int connectionId)
{
    for (common::ListNode* node = m_probeRequests.Begin(); node != m_probeRequests.End(); node = node->next)
    {
        NatProbeRequest* request = NatProbeRequestList::RequestOf(node);
        if (static_cast<unsigned int>(request->m_stationLocation.m_field1c) == connectionId)
            return request;
    }
    return 0;
}

unsigned short NatTraversalProtocol::GetPeer2PeerPort(const transport::StationLocation& location)
{
    session::ProcessUpdateMeshJob* job = session::g_piaMesh->GetProcessUpdateMeshJobPtr();
    if (job == 0)
        return 0;

    if (!job->m_started)
    {
        transport::StationConnectionInfo& host = transport::g_stationConnectionInfoTable->m_hostStationConnectionInfo;
        if (host.m_stationLocation == location)
        {
            host.Trace(cTraceDetail);
            if (inet::IsGlobal(host.m_stationLocation))
                return host.m_stationLocation.m_stationAddress.m_inetAddress.m_port;
            if (inet::IsEimMapping(host.m_localStationLocation)
                && host.m_stationLocation.m_stationAddress.m_inetAddress.m_port
                       != host.m_localStationLocation.m_stationAddress.m_inetAddress.m_port)
                return host.m_localStationLocation.m_stationAddress.m_inetAddress.m_port;
        }
        return 0;
    }

    location.Trace(cTraceDetail);
    unsigned int count = job->m_stationCount;
    for (unsigned int index = 0; index < count; ++index)
    {
        transport::StationConnectionInfo* info = index < job->m_stationCount ? &job->m_stationConnectionInfos[index]
                                                                             : 0;
        if (info == 0)
            continue;
        if (info->m_stationLocation == location)
        {
            info->Trace(cTraceDetail);
            if (inet::IsEdmMapping(info->m_stationLocation))
                return 0;
            return info->m_localStationLocation.m_stationAddress.m_inetAddress.m_port;
        }
        if (location.m_field26 != 0)
        {
            if (!inet::IsEdmMapping(info->m_stationLocation))
            {
                transport::StationLocation candidate(info->m_stationLocation);
                candidate.m_field26 = 1;
                candidate.m_stationAddress.m_inetAddress.m_port =
                    info->m_localStationLocation.m_stationAddress.m_inetAddress.m_port;
                if (candidate == location)
                {
                    info->Trace(cTraceDetail);
                    return candidate.m_stationAddress.m_inetAddress.m_port;
                }
            }
            else
            {
                transport::StationLocation candidate(info->m_stationLocation);
                candidate.m_stationAddress.m_inetAddress.m_port = location.m_stationAddress.m_inetAddress.m_port;
                candidate.m_field26 = 1;
                if (candidate == location)
                {
                    info->Trace(cTraceDetail);
                    return 0;
                }
            }
        }
    }
    m_fieldC5C = true;
    return 0;
}

void NatTraversalProtocol::CompletedNatPortDetection(Result)
{
    m_fieldC5B = true;
    m_nextDispatchTime.m_tick = 0;
    if (m_fieldC5A)
    {
        m_pendingDetectionDeadline = getDeadline(5000);
    }
}

void NatTraversalProtocol::DetectNatPort(const transport::StationLocation& local,
                                         const transport::StationLocation& remote)
{
    if (session::g_piaMesh == 0 || !session::g_piaMesh->IsProcessedAtMeshBegin())
        ++common::g_sessionBeginMonitoringContent.m_metric138;

    m_callContext.RegisterCompletionCallback(NatPortDetectionCompletionCallback, this);
    Result result = m_ownerNatTraverser->GetNatPortDetecter()->Startup(m_ownerNatTraverser, &m_callContext,
                                                                       common::InetAddress(), local, remote, true);
    if (result != 0)
        m_callContext.SignalFailure(result);
    m_ownerNatTraverser->GetNatPortDetecter()->StartDetectionJob();
    m_portDetectionLocation = local;
}

void NatTraversalProtocol::RelayProbeRequest(const transport::StationLocation& source,
                                             const transport::StationLocation& destination)
{
    NatTraversalTime* traversalTime = m_traversalTimes.Find(source);
    if (traversalTime != 0)
    {
        if (destination.m_field26 == 0)
        {
            traversalTime->m_startTime.SetNow();
        }
        else
        {
            traversalTime->m_endTime.SetNow();
            traversalTime->m_count = 0;
        }
    }
    else
    {
        NatTraversalTime newTraversalTime;
        newTraversalTime.m_location = source;
        newTraversalTime.m_endTime.SetNow();
        newTraversalTime.m_count = 0;
        newTraversalTime.m_port = 0;
        m_traversalTimes.Add(newTraversalTime);
    }
    m_ownerNatTraverser->RelayProbeRequest(source, destination, false, 0);
}

void NatTraversalProtocol::SendProbeRequest(transport::StationLocation location, const NatProbeRequest& request)
{
    const transport::StationLocation& requestLocation = request.m_stationLocation;
    bool fromLocal = false;
    if (requestLocation.m_field26 == 0)
        fromLocal = true;
    location.m_field1c = m_localServiceConnectionId;
    location.m_field26 = fromLocal;
    if (request.m_addressValid)
    {
        common::InetAddress& address = location.m_inetAddress;
        if (fromLocal)
            address = m_ownerNatTraverser->GetRelayServerAddress();
        else
            address = requestLocation.m_inetAddress;
    }
    location.Trace(cTraceBrief);
    requestLocation.Trace(cTraceBrief);

    if (inet::IsPublic(location) && inet::IsEdmMapping(m_localLocation) && !location.m_inetAddress.IsValid())
    {
        if (fromLocal)
        {
            m_pendingConnectionId = requestLocation.m_field1c;
            m_fieldC59 = true;
            m_fieldC58 = true;
            common::TimeSpan timeout = common::TimeSpan::FromMSec(15000);
            common::Time deadline;
            deadline.SetNow();
            deadline = deadline + timeout;
            m_pendingDetectionDeadline = deadline;
        }
        else
        {
            m_fieldC5A = true;
            m_pendingConnectionId = requestLocation.m_field1c;
            common::TimeSpan timeout = common::TimeSpan::FromMSec(15000);
            common::Time deadline;
            deadline.SetNow();
            deadline = deadline + timeout;
            m_pendingDetectionDeadline = deadline;
        }
        if (m_ownerNatTraverser->m_natProperty.m_flag != 0)
        {
            transport::StationLocation target(requestLocation);
            target.m_stationAddress.m_inetAddress.m_port = cNatServerPort;
            SendDummyPacket(target, 4, 5);
        }
        DetectNatPort(requestLocation, location);
        return;
    }

    if (!location.m_inetAddress.IsValid())
    {
        unsigned short port = 0;
        if (inet::IsPublic(location))
        {
            location.m_stationAddress.m_inetAddress.m_port = m_ownerNatTraverser->m_natProperty.m_publicPort;
        }
        else
        {
            if (m_ownerNatTraverser != 0)
                port = m_ownerNatTraverser->m_publicPort;
            location.m_stationAddress.m_inetAddress.m_port = port;
        }
    }
    RelayProbeRequest(requestLocation, location);
}

void NatTraversalProtocol::ScheduleProbeRequest(const NatProbeRequest& request)
{
    for (common::ListNode* node = m_edmProbeRequests.Begin(); node != m_edmProbeRequests.End(); node = node->next)
    {
        NatProbeRequest* element = NatProbeRequestList::RequestOf(node);
        if (inet::IsEdmMapping(element->m_stationLocation)
            && element->m_stationLocation.m_stationAddress.m_inetAddress.m_address
                   == request.m_stationLocation.m_stationAddress.m_inetAddress.m_address)
        {
            const common::TimeSpan span(common::TimeSpan::GetTicksPerMSec().m_tick * 1000);
            common::Time deadline;
            deadline.SetNow();
            deadline = deadline + span;
            element->m_deadline = deadline;
            request.Trace(cTraceBrief);
            element->Trace(cTraceBrief);
            return;
        }
    }
}

bool NatTraversalProtocol::SendProbeRequests()
{
    if ((inet::IsEdmMapping(m_localLocation)
         && (m_callContext.m_state == common::CallContext::cStateInProgress || m_fieldC59 || m_fieldC5A))
        || m_edmProbeRequests.GetSize() == 0)
        return false;

    NatProbeRequest request(*m_edmProbeRequests.Front());
    common::Time now;
    now.SetNow();
    if (request.m_deadline.m_tick != 0 && now < request.m_deadline)
    {
        request.Trace(cTraceBrief);
        return false;
    }

    transport::StationConnectionInfoTable* table = transport::g_stationConnectionInfoTable;
    bool sent = false;
    bool sendToLocal = true;
    if (!request.m_addressValid)
    {
        if (request.m_probeRequested)
        {
            if (request.m_stationConnectionInfo.m_stationLocation.m_stationAddress.m_inetAddress.m_address
                == table->m_localStationConnectionInfo.m_stationLocation.m_stationAddress.m_inetAddress.m_address)
                sendToLocal = false;
        }
        else if (!inet::IsPublic(request.m_stationLocation))
        {
            sendToLocal = false;
        }
    }

    if (!sendToLocal)
    {
        sent = true;
        SendProbeRequest(table->m_localStationConnectionInfo.m_localStationLocation, request);
        if (request.m_probeRequested && !inet::IsEdmMapping(m_localLocation)
            && !inet::IsEdmMapping(request.m_stationLocation))
            SendProbeRequest(table->m_localStationConnectionInfo.m_stationLocation, request);
    }
    else
    {
        SendProbeRequest(table->m_localStationConnectionInfo.m_stationLocation, request);
    }

    m_edmProbeRequests.Release(m_edmProbeRequests.Front());
    if (!sent && inet::IsEdmMapping(request.m_stationLocation) && !inet::IsEdmMapping(m_localLocation))
        ScheduleProbeRequest(request);
    return m_edmProbeRequests.GetSize() != 0 && !m_fieldC58;
}

void NatTraversalProtocol::SendProbes()
{
    if (m_localServiceConnectionId == 0)
        return;

    common::ListNode* node = m_probeList.Begin();
    while (node != m_probeList.End())
    {
        NatProbe* probe = NatProbeList::ProbeOf(node);
        common::Time now;
        now.SetNow();
        if (probe->m_deadline < now && (probe->m_rtt == -1 || probe->m_sent))
        {
            probe->Trace(cTraceVerbose);
            m_traversalTimes.Remove(probe->m_stationLocation);
            m_ownerNatTraverser->ClearNatMonitoringData(probe->m_stationLocation);
            node = node->next;
            m_probeList.Release(probe);
            continue;
        }
        common::Time updateTime;
        updateTime.SetNow();
        if (!probe->UpdateIsNeeded(updateTime))
        {
            node = node->next;
            continue;
        }
        if (m_fieldC58 && probe->m_probeCount == 0)
        {
            node = node->next;
            continue;
        }

        unsigned char repeat = 2;
        unsigned char count = 0;
        if (probe->m_probeCount <= 10 && probe->m_startTime.m_tick == 0 && probe->m_stationLocation.m_field26 == 1
            && inet::IsPublic(probe->m_stationLocation)
            && probe->m_stationLocation.m_stationAddress.m_inetAddress.m_address
                   != m_localLocation.m_stationAddress.m_inetAddress.m_address)
        {
            count = 4;
            repeat = 3;
        }

        NatProbeRequest* request = FindProbeRequestByConnectionId(probe->m_stationLocation.m_field1c);
        if (request != 0 && request->m_callContext != 0
            && request->m_callContext->m_state != common::CallContext::cStateInProgress)
        {
            request->m_callContext->InitiateCall();
            NatTraversalTime* traversalTime = m_traversalTimes.Find(request->m_stationLocation);
            if (traversalTime != 0)
            {
                traversalTime->m_startTime.SetNow();
            }
            else
            {
                NatTraversalTime newTraversalTime;
                newTraversalTime.m_location = request->m_stationLocation;
                newTraversalTime.m_startTime.SetNow();
                newTraversalTime.m_endTime.SetNow();
                newTraversalTime.m_count = 0;
                newTraversalTime.m_port = 0;
                m_traversalTimes.Add(newTraversalTime);
            }
        }
        if (probe->m_startTime.m_tick == 0)
            probe->SprayTargetPort();

        {
            transport::StationLocation target(probe->m_stationLocation);
            for (unsigned char i = 0; i < repeat; ++i)
            {
                common::Time sendTime;
                sendTime.SetNow();
                SendProbe(cProbeType0, target, sendTime, count);
            }
        }
        ++probe->m_probeCount;
        if (inet::IsEdmMapping(m_localLocation))
        {
            if (probe->m_probeCount >= probe->m_probeLimit)
                m_fieldC58 = true;
            else
                m_pendingConnectionId = probe->m_stationLocation.m_field1c;
            return;
        }
        node = node->next;
    }
}

Result NatTraversalProtocol::dispatchProbe(common::Time& now)
{
    if (m_fieldC58 || m_fieldC59 || m_fieldC5A)
    {
        if (m_pendingDetectionDeadline < now && m_fieldC5A)
        {
            NatProbe* probe = m_probeList.FindByConnectionId(m_pendingConnectionId);
            NatProbeRequest* request = FindProbeRequestByConnectionId(m_pendingConnectionId);
            m_pendingConnectionId = 0;
            m_fieldC5A = false;
            if (m_fieldC59)
            {
                probe = m_probeList.FindByConnectionId(m_pendingConnectionId);
                request = FindProbeRequestByConnectionId(m_pendingConnectionId);
                m_pendingConnectionId = 0;
                m_fieldC59 = false;
            }
            if (m_callContext.m_state == common::CallContext::cStateInProgress)
                CancelPortDetection();
            if (probe != 0)
            {
                probe->m_stationLocation.Trace(cTraceBrief);
                common::Time probeTime;
                probeTime.SetNow();
                probe->m_deadline = probeTime;
            }
            if (request != 0)
            {
                request->Trace(cTraceBrief);
                if (request->m_callContext != 0)
                {
                    Result failure = cResultProbeRequestTimedOut;
                    request->m_callContext->SignalFailure(failure);
                }
                m_probeRequests.Release(request);
            }
            m_fieldC58 = false;
        }
    }

    if (m_callContext.m_state >= common::CallContext::cStateSuccess
        && m_callContext.m_state <= common::CallContext::cStateCancelled)
    {
        if (m_callContext.m_result == 0)
        {
            if (m_fieldC5B)
            {
                unsigned short port = GetPeer2PeerPort(m_portDetectionLocation);
                if (port != 0)
                {
                    transport::StationLocation target(m_portDetectionLocation);
                    target.m_stationAddress.m_inetAddress.m_port = port;
                    SendDummyPacket(target, 4, 5);
                }
                m_fieldC5B = false;
                unsigned int index = m_ownerNatTraverser->GetNatMonitoringDataIndex(m_portDetectionLocation);
                if (index < cNatMonitoringEntryNum)
                    common::g_sessionBeginMonitoringContent.m_metric147[index] = port == 0 ? 0 : 1;
                if (m_fieldC5C)
                {
                    m_fieldC5C = false;
                    m_portDetectionLocation = transport::StationLocation();
                    m_callContext.Reset();
                    m_nextDispatchTime = common::Time();
                    return 0;
                }
            }
            m_callContext.Reset();
        }
        else
        {
            NatProbeRequest* request = FindProbeRequestByConnectionId(m_portDetectionLocation.m_field1c);
            if (request != 0)
            {
                if (request->m_callContext != 0)
                {
                    Result failure = cResultPortDetectionFailed;
                    request->m_callContext->SignalFailure(failure);
                }
                EraseProbeRequestByConnectionId(m_portDetectionLocation.m_field1c);
            }
            NatProbe* probe = m_probeList.FindByConnectionId(m_portDetectionLocation.m_field1c);
            if (probe != 0)
            {
                common::Time probeTime;
                probeTime.SetNow();
                probe->m_deadline = probeTime;
            }
            m_portDetectionLocation = transport::StationLocation();
            m_callContext.Reset();
        }
    }

    while (SendProbeRequests())
    {
    }
    if (m_callContext.m_state != common::CallContext::cStateInProgress)
        SendProbes();
    if (m_ownerNatTraverser != 0)
        m_ownerNatTraverser->UpdateRelayTable();
    return 0;
}

Result NatTraversalProtocol::Dispatch()
{
    if (m_ownerNatTraverser == 0 || !m_ownerNatTraverser->IsNatDetectionCompleted())
        return 0;

    transport::PacketHandler::Iterator& iterator = mStationPacketHandler->GetIterator(cProtocolType);
    iterator.BeginIteration();
    for (; !iterator.IsEndIteration(); iterator.NextIteration())
        ReceiveMessage(*iterator.GetMessageReader());

    common::Time now;
    now.SetNow();
    if (!(now <= m_nextDispatchTime))
    {
        const common::TimeSpan interval = common::TimeSpan::FromMSec(g_natDispatchIntervalMsec);
        m_nextDispatchTime = now + interval;
        if (m_serverKeepAliveEnabled && m_serverKeepAliveTime < now)
        {
            SendDummyPacket(m_serverLocation, 0, 3);
            m_serverKeepAliveTime = now + common::TimeSpan::FromMSec(15000);
        }
        Result result = dispatchProbe(now);
        return result;
    }
    return 0;
}

bool NatTraversalProtocol::GetLatestStationLocation(const transport::StationLocation& location,
                                                    transport::StationLocation* latest)
{
    NatProbe* probe = m_probeList.FindByConnectionId(location.m_field1c);
    if (probe != 0 && !(probe->m_stationLocation == location))
    {
        *latest = probe->m_stationLocation;
        if (latest->m_principalId == 0)
            latest->m_principalId = location.m_principalId;
        if (latest->m_field24 == 0)
            latest->m_field24 = location.m_field24;
        if (latest->m_field23 == 0)
            latest->m_field23 = location.m_field23;
        if (latest->m_field25 == 0)
            latest->m_field25 = location.m_field25;
        if (latest->m_field18 == 0)
            latest->m_field18 = location.m_field18;
        if (latest->m_field21 == 0)
            latest->m_field21 = location.m_field21;
        if (latest->m_field22 == 0)
            latest->m_field22 = location.m_field22;
        if (latest->m_field20 == 0)
            latest->m_field20 = location.m_field20;
        if (!latest->m_inetAddress.IsValid())
            latest->m_inetAddress = location.m_inetAddress;
        return true;
    }
    return false;
}

void NatTraversalProtocol::StartServerKeepAlive()
{
    m_serverKeepAliveEnabled = true;
    common::Time now;
    now.SetNow();
    m_serverKeepAliveTime = now;
}

void NatTraversalProtocol::StopServerKeepAlive()
{
    m_serverKeepAliveEnabled = false;
}

void NatTraversalProtocol::Trace(unsigned long long value) const
{
    m_probeList.Trace(value);
}

NatProbe* NatTraversalProtocol::GetNatProbe(const transport::StationLocation& location)
{
    return m_probeList.FindByConnectionId(location.m_field1c);
}

void NatTraversalProtocol::ResetProbeStatus(const transport::StationLocation& location, bool relay)
{
    NatProbe* probe;
    if (relay)
    {
        probe = m_probeList.FindByAddressAndConnectionId(location);
        if (probe == 0)
            return;
        if (probe->m_rtt == -1)
            return;
    }
    else
    {
        probe = m_probeList.FindByAddressPortAndConnectionId(location);
        if (probe == 0 || probe->m_rtt == -1)
        {
            probe = m_probeList.FindByAddressAndConnectionId(location);
            if (probe == 0)
                return;
        }
    }

    common::Time deadline = getDeadline(g_natProbeDurationMsec);
    probe->m_probeCount = 0;
    probe->m_sent = 0;
    probe->m_deadline = deadline;
    location.Trace(cTraceVerbose);
}

void NatTraversalProtocol::ResetNextProbeTime()
{
    m_nextDispatchTime = common::Time();
}

unsigned short NatTraversalProtocol::GetProtocolType() const
{
    return cProtocolType;
}

unsigned int NatTraversalProtocol::IsProtocolFilteringEnabled() const
{
    return 0;
}

static void NatPortDetectionCompletionCallback(Result result, void* param)
{
    static_cast<NatTraversalProtocol*>(param)->CompletedNatPortDetection(result);
}

}
}
}
