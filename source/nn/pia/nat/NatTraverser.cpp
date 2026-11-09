#include "nn/pia/nat/NatTraverser.hpp"

#include <new>

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/inet/IFacade.hpp"
#include "nn/pia/nat/Api.hpp"
#include "nn/pia/nat/NatDetecter.hpp"
#include "nn/pia/nat/NatPortDetecter.hpp"
#include "nn/pia/nat/NatProbe.hpp"
#include "nn/pia/nat/NatPropertyDetecter.hpp"
#include "nn/pia/nat/NatTraversalProtocol.hpp"
#include "nn/pia/nat/NatTraversalTime.hpp"
#include "nn/pia/transport/StationLocation.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn
{
namespace pia
{
namespace nat
{

unsigned int g_natLocalConnectionId;
int g_natClearProbeDelayMsec = 90000;
int g_natProbeDurationMsec = 90000;
unsigned char g_natProbeSprayCount = 50;
int g_natDispatchIntervalMsec = 300;

static void NatPropertyDetectionCompletionCallback(Result result, void* param);
static void NatPortDetectionCompletionCallback(Result result, void* param);

NatTraverser::NatTraverser()
    : m_callContext(0),
      m_natProperty(),
      m_servers(),
      m_state(0),
      m_publicPort(0),
      m_protocol(0),
      m_propertyContext(),
      m_portContext(),
      m_facade(0)
{
    void* portDetecterMemory = pead::allocateFromSeadHeap(sizeof(NatPortDetecter), common::HeapManager::GetHeap());
    if (portDetecterMemory != 0)
        m_natPortDetecter = new (portDetecterMemory) NatPortDetecter();
    else
        m_natPortDetecter = 0;
    void* propertyDetecterMemory = pead::allocateFromSeadHeap(sizeof(NatPropertyDetecter),
                                                              common::HeapManager::GetHeap());
    if (propertyDetecterMemory != 0)
        m_natPropertyDetecter = new (propertyDetecterMemory) NatPropertyDetecter();
    else
        m_natPropertyDetecter = 0;
}

NatTraverser::~NatTraverser()
{
    if (m_natPortDetecter != 0)
    {
        common::DeleteObj(m_natPortDetecter);
        m_natPortDetecter = 0;
    }
    if (m_natPropertyDetecter != 0)
    {
        common::DeleteObj(m_natPropertyDetecter);
        m_natPropertyDetecter = 0;
    }
    m_protocol = 0;
}

Result NatTraverser::Startup(inet::IFacade* facade, common::CallContext* context)
{
    if (facade == 0)
        return cResultInvalidArgument;
    if (context == 0)
        return cResultInvalidArgument;
    if (m_callContext != 0)
        return cResultInvalidState;
    if (m_propertyContext.m_state == common::CallContext::cStateInProgress)
        return cResultInvalidState;
    if (m_portContext.m_state == common::CallContext::cStateInProgress)
        return cResultInvalidState;
    if (m_state != 0)
        return cResultInvalidState;
    if (m_protocol == 0)
        return cResultInvalidState;

    m_facade = facade;
    m_callContext = context;
    Result result = m_protocol->StartupNatTraversal(this);
    if (result != 0)
        return result;
    if (m_natProperty.m_mappingType == 0)
        return StartNatPropertyDetection();
    return StartNatPortDetection();
}

void NatTraverser::Cleanup()
{
    if (m_state == 1)
        m_natPropertyDetecter->CancelDetectionJob();
    if (m_state == 2 && m_protocol != 0)
        m_natPortDetecter->CancelDetectionJob();
    m_state = 0;
    if (m_callContext != 0 && m_callContext->m_state == common::CallContext::cStateInProgress)
        m_callContext->SignalCancel();
    m_callContext = 0;
    if (m_protocol != 0)
    {
        m_protocol->StopServerKeepAlive();
        m_protocol->CleanupNatTraversal();
    }
    if (m_propertyContext.m_state == common::CallContext::cStateInProgress)
        m_propertyContext.SignalCancel();
    m_propertyContext.Reset();
    if (m_portContext.m_state == common::CallContext::cStateInProgress)
        m_portContext.SignalCancel();
    m_portContext.Reset();
    m_servers.size = 0;
}

void NatTraverser::Trace(unsigned long long) const
{
}

void NatTraverser::CompleteNatDetectionCommon()
{
    if (m_callContext == 0)
        return;
    if (m_callContext->m_cancelRequested)
    {
        m_callContext->SignalCancel();
        m_callContext = 0;
    }
    else
    {
        if (m_natProperty.m_privatePort != 0 && m_natProperty.m_mappingType != 2)
            m_publicPort = m_natProperty.m_privatePort;
        if (m_protocol == 0)
        {
            Result failure = cResultInvalidState;
            m_callContext->SignalFailure(failure);
            m_callContext = 0;
        }
        else
        {
            m_protocol->StartServerKeepAlive();
            Result success = 0;
            m_state = 3;
            m_callContext->SignalSuccess(success);
            m_callContext = 0;
        }
    }
}

void NatTraverser::CompletedNatPropertyDetection(bool success)
{
    if (m_state == 0 || m_callContext == 0)
        return;
    if (success)
    {
        if (m_propertyContext.m_state == common::CallContext::cStateCancelled)
        {
            m_propertyContext.Reset();
            return;
        }
        if (m_propertyContext.m_result != 0)
        {
            if (m_propertyContext.m_result == cResultCancelled)
                m_callContext->SignalCancel();
            else
                m_callContext->SignalFailure(m_propertyContext.m_result);
            m_callContext = 0;
            m_propertyContext.Reset();
            return;
        }
    }
    CompleteNatDetectionCommon();
}

Result NatTraverser::StartNatPropertyDetection()
{
    if (m_facade->IsBeginMonitoringDataAvailable())
    {
        unsigned short& propertyDetectionCount = common::g_sessionBeginMonitoringContent.m_metric137;
        if (propertyDetectionCount == cCounterMax)
            propertyDetectionCount = 1;
        else
            ++propertyDetectionCount;
    }

    common::InetAddress address;
    NatDetecter::SetRandomPortRange(m_facade->GetRandomPortRangeMin(), m_facade->GetRandomPortRangeMax());
    NatDetecter::SetReservedPort(0, m_facade->GetReservedPort());
    address.m_port = NatDetecter::GetRandomPortNumber();
    m_publicPort = address.m_port;
    m_propertyContext.RegisterCompletionCallback(NatPropertyDetectionCompletionCallback, this);
    Result result = m_natPropertyDetecter->Startup(this, &m_propertyContext, address);
    if (result != 0)
        return result;
    m_state = 1;
    m_natPropertyDetecter->StartDetectionJob();
    return 0;
}

void NatTraverser::CompletedNatPortDetection(bool success)
{
    if (m_state == 0 || m_callContext == 0)
        return;
    if (success)
    {
        if (m_portContext.m_state == common::CallContext::cStateCancelled)
        {
            m_portContext.Reset();
            return;
        }
        if (m_portContext.m_result != 0)
        {
            if (m_portContext.m_result == cResultCancelled)
                m_callContext->SignalCancel();
            else
                m_callContext->SignalFailure(m_portContext.m_result);
            m_callContext = 0;
            m_portContext.Reset();
            return;
        }
    }
    CompleteNatDetectionCommon();
}

Result NatTraverser::StartNatPortDetection()
{
    if (m_facade->IsBeginMonitoringDataAvailable())
    {
        unsigned short& portDetectionCount = common::g_sessionBeginMonitoringContent.m_metric138;
        if (portDetectionCount == cCounterMax)
            portDetectionCount = 1;
        else
            ++portDetectionCount;
    }

    NatDetecter::SetRandomPortRange(m_facade->GetRandomPortRangeMin(), m_facade->GetRandomPortRangeMax());
    NatDetecter::SetReservedPort(0, m_facade->GetReservedPort());
    m_publicPort = NatDetecter::GetRandomPortNumber();
    if (m_natProperty.m_mappingType == 2)
    {
        m_state = 2;
        CompletedNatPortDetection(false);
        return 0;
    }
    if (m_protocol == 0)
        return cResultInvalidState;

    m_portContext.RegisterCompletionCallback(NatPortDetectionCompletionCallback, this);
    transport::StationLocation location;
    location.m_stationAddress.Clear();
    common::InetAddress address(0, m_publicPort);
    Result result = m_natPortDetecter->Startup(this, &m_portContext, address, location, location, false);
    if (result != 0)
        return result;
    m_state = 2;
    m_natPortDetecter->StartDetectionJob();
    return 0;
}

unsigned int NatTraverser::GetNatMonitoringDataIndex(const transport::StationLocation& location)
{
    unsigned int connectionId = location.m_field1c;
    unsigned int result = cInvalidIndex;
    if (m_facade->IsBeginMonitoringDataAvailable())
    {
        for (unsigned int index = 0; index < cNatMonitoringEntryNum; ++index)
        {
            if (connectionId == common::g_sessionBeginMonitoringContent.m_metric139[index])
            {
                result = index;
                break;
            }
        }
        if (result == cInvalidIndex)
        {
            for (unsigned int index = 0; index < cNatMonitoringEntryNum; ++index)
            {
                if (common::g_sessionBeginMonitoringContent.m_metric139[index] == cInvalidId)
                {
                    result = index;
                    break;
                }
            }
        }
    }
    return result;
}

void NatTraverser::IncrementNatTraversalSuccessCount(unsigned int)
{
    if (m_facade->IsBeginMonitoringDataAvailable())
    {
        unsigned short& successCount = common::g_sessionBeginMonitoringContent.m_metric151;
        if (successCount == cCounterMax)
            successCount = 1;
        else
            ++successCount;
    }
}

void NatTraverser::IncrementNatTraversalFailureCount(unsigned int)
{
    if (m_facade->IsBeginMonitoringDataAvailable())
    {
        unsigned short& failureCount = common::g_sessionBeginMonitoringContent.m_metric152;
        if (failureCount == cCounterMax)
            failureCount = 1;
        else
            ++failureCount;
    }
}

void NatTraverser::ReportNatTraversalResult(const transport::StationLocation& location, Result result, bool relay)
{
    if (location.m_field1c == 0)
    {
        location.Trace(cTraceError);
        return;
    }

    NatTraversalTime* traversalTime = m_protocol->m_traversalTimes.Find(location);
    if (traversalTime == 0)
    {
        NatTraversalTime newTraversalTime;
        newTraversalTime.m_location = location;
        newTraversalTime.m_startTime.SetNow();
        newTraversalTime.m_endTime.SetNow();
        newTraversalTime.m_count = 0;
        newTraversalTime.m_port = 0;
        m_protocol->m_traversalTimes.Add(newTraversalTime);
    }

    unsigned int index;
    if (!relay && result != 0)
    {
        traversalTime = m_protocol->m_traversalTimes.Find(location);
        if (traversalTime != 0)
        {
            common::Time now;
            now.SetNow();
            traversalTime->m_count = static_cast<unsigned int>(
                static_cast<long long>(now.m_tick - traversalTime->m_endTime.m_tick)
                / common::TimeSpan::GetTicksPerMSec().m_tick);
        }
    }
    if (result == 0)
    {
        IncrementNatTraversalSuccessCount(location.m_field1c);
        index = GetNatMonitoringDataIndex(location);
    }
    else
    {
        IncrementNatTraversalFailureCount(location.m_field1c);
        index = GetNatMonitoringDataIndex(location);
    }

    if (index < cNatMonitoringEntryNum)
    {
        unsigned char probeFlag = 0;
        unsigned int elapsedTime = cInvalidId;
        unsigned char sprayCount = 0;
        unsigned char probeTimeout = 0;
        unsigned int attemptCount = cInvalidId;
        NatProbe* probe = m_protocol->GetNatProbe(location);
        if (probe != 0)
        {
            probeFlag = static_cast<unsigned char>(probe->m_probeCount);
            sprayCount = static_cast<unsigned char>(probe->GetPortSprayCount());
            probeTimeout = probe->m_timeout;
        }
        traversalTime = m_protocol->m_traversalTimes.Find(location);
        if (traversalTime != 0)
        {
            if (probeFlag != 0)
            {
                common::Time now;
                now.SetNow();
                elapsedTime = static_cast<unsigned int>(
                    static_cast<long long>(now.m_tick - traversalTime->m_startTime.m_tick)
                    / common::TimeSpan::GetTicksPerMSec().m_tick);
                attemptCount = traversalTime->m_count;
            }
            else
            {
                attemptCount = traversalTime->m_count;
                elapsedTime = 0;
            }
        }
        common::g_sessionBeginMonitoringContent.m_metric139[index] = location.m_field1c;
        common::g_sessionBeginMonitoringContent.m_metric140[index] = common::hashWithMd5(location.m_principalId);
        common::g_sessionBeginMonitoringContent.m_metric141[index] = elapsedTime;
        common::g_sessionBeginMonitoringContent.m_metric142[index] = attemptCount;
        common::g_sessionBeginMonitoringContent.m_metric143[index] = result;
        common::g_sessionBeginMonitoringContent.m_metric144[index] = probeFlag;
        common::g_sessionBeginMonitoringContent.m_metric145[index] = sprayCount;
        common::g_sessionBeginMonitoringContent.m_metric146[index] = probeTimeout;
        common::g_sessionBeginMonitoringContent.m_metric155[index] = location.m_inetAddress.m_address != 0 ? 1 : 0;
    }
    m_protocol->m_traversalTimes.Remove(location);
    if (m_protocol != 0)
        m_protocol->ResetNextProbeTime();
}

bool NatTraverser::PrepareNatTraversal(const transport::StationLocation& location,
                                       const transport::StationConnectionInfo& connection, common::CallContext* context,
                                       bool relay, bool passive, bool force)
{
    bool result = true;
    if (relay)
    {
        context->InitiateCall();
        m_protocol->GetNatProbeList()->Trace(cTraceVerbose);
        NatProbe* probe = m_protocol->GetNatProbeList()->FindByAddressPortAndConnectionId(location);
        bool reset = false;
        if (probe != 0 && probe->m_rtt != -1)
        {
            reset = true;
        }
        else
        {
            probe = m_protocol->GetNatProbeList()->FindByAddressAndConnectionId(location);
            if (probe != 0 && probe->m_sent)
                reset = true;
        }
        if (reset)
        {
            result = false;
            if (probe->m_sent)
            {
                const common::TimeSpan duration = common::TimeSpan::FromMSec(g_natProbeDurationMsec);
                common::Time now;
                now.SetNow();
                probe->m_deadline = now + duration;
                probe->m_probeCount = 0;
                probe->m_sent = 0;
                location.Trace(cTraceVerbose);
            }
        }
    }
    else
    {
        result = m_protocol->PrepareNatTraversal(location, connection, context, passive, force);
    }
    return result;
}

void NatTraverser::ClearNatTraversal(const transport::StationLocation& location, bool relay)
{
    if (location.m_field1c == 0)
    {
        location.Trace(cTraceError);
        return;
    }
    m_protocol->ClearNatTraversal(location, relay);
}

bool NatTraverser::CheckLatestStationLocation(transport::StationLocation& location)
{
    transport::StationLocation latest;
    if (m_protocol->GetLatestStationLocation(location, &latest))
    {
        location.Trace(cTraceBrief);
        location = latest;
        location.Trace(cTraceBrief);
        return true;
    }
    return false;
}

Result NatTraverser::CreateProtocols()
{
    transport::Transport* transport = transport::g_piaTransport;
    if (transport == 0)
        return cResultInvalidState;
    m_protocolId = transport->m_protocolManager.CreateProtocol<NatTraversalProtocol>(0);
    m_protocol = transport->m_protocolManager.GetProtocol<NatTraversalProtocol>(m_protocolId);
    return 0;
}

void NatTraverser::ClearNatMonitoringData(const transport::StationLocation& location)
{
    unsigned int index = GetNatMonitoringDataIndex(location);
    if (index < cNatMonitoringEntryNum && m_facade->IsBeginMonitoringDataAvailable())
    {
        common::g_sessionBeginMonitoringContent.m_metric139[index] = cInvalidId;
        common::g_sessionBeginMonitoringContent.m_metric140[index] = cInvalidId;
        common::g_sessionBeginMonitoringContent.m_metric141[index] = cInvalidId;
        common::g_sessionBeginMonitoringContent.m_metric142[index] = cInvalidId;
        common::g_sessionBeginMonitoringContent.m_metric143[index] = cInvalidId;
        common::g_sessionBeginMonitoringContent.m_metric144[index] = cInvalidIndex;
        common::g_sessionBeginMonitoringContent.m_metric145[index] = cInvalidIndex;
        common::g_sessionBeginMonitoringContent.m_metric146[index] = cInvalidIndex;
        common::g_sessionBeginMonitoringContent.m_metric147[index] = cInvalidIndex;
    }
}

bool NatTraverser::IsStartupCancelled()
{
    if (m_callContext == 0)
        return false;
    return m_callContext->m_cancelRequested;
}

void NatTraverser::RelayProbeRequest(const transport::StationLocation& source,
                                     const transport::StationLocation& destination, bool relay, unsigned short port)
{
    m_facade->RelayProbeRequest(source, destination);
    if (!relay)
        return;

    NatTraversalTime* traversalTime = m_protocol->m_traversalTimes.Find(source);
    if (traversalTime != 0)
    {
        traversalTime->m_port = port;
        traversalTime->m_endTime.SetNow();
        traversalTime->m_count = 0;
        return;
    }

    NatTraversalTime newTraversalTime;
    newTraversalTime.m_location = source;
    newTraversalTime.m_startTime.SetNow();
    newTraversalTime.m_endTime.SetNow();
    newTraversalTime.m_count = 0;
    newTraversalTime.m_port = port;
    m_protocol->m_traversalTimes.Add(newTraversalTime);
}

void NatTraverser::ProcessProbeRequest(const transport::StationLocation& location)
{
    if (m_protocol == 0)
        return;
    m_protocol->ProcessProbeRequest(location);
}

bool NatTraverser::IsTraversed(const transport::StationLocation& location) const
{
    if (m_protocol != 0)
    {
        NatProbe* probe = m_protocol->m_probeList.FindByAddressPortAndConnectionId(location);
        if (probe != 0 && probe->m_rtt != -1)
            return true;
    }
    return false;
}

bool NatTraverser::IsTraversed(unsigned int index) const
{
    if (m_protocol != 0)
    {
        NatProbe* probe = m_protocol->m_probeList.FindByConnectionId(index);
        if (probe != 0 && probe->m_rtt != -1)
            return true;
    }
    return false;
}

void NatTraverser::SetLocalPublicLocation(const transport::StationLocation& location)
{
    if (m_protocol == 0)
        return;
    m_protocol->m_localLocation = location;
}

NatPortDetecter* NatTraverser::GetNatPortDetecter()
{
    return m_natPortDetecter;
}

unsigned int NatTraverser::GetLocalServiceConnectionId() const
{
    return m_facade->GetLocalServiceConnectionId();
}

unsigned int NatTraverser::GetRandomPortRangeMin() const
{
    return m_facade->GetRandomPortRangeMin();
}

unsigned int NatTraverser::GetRandomPortRangeMax() const
{
    return m_facade->GetRandomPortRangeMax();
}

bool NatTraverser::IsNatDetectionCompleted() const
{
    return m_state == 3;
}

void NatTraverser::ResetProbeStatus(const transport::StationLocation& location, bool relay)
{
    if (m_protocol == 0)
        return;
    m_protocol->ResetProbeStatus(location, relay);
}

void NatTraverser::UpdateRelayTable()
{
    m_facade->UpdateRelayTable();
}

const common::InetAddress& NatTraverser::GetRelayServerAddress() const
{
    return m_facade->GetRelayServerAddress();
}

static void NatPropertyDetectionCompletionCallback(Result, void* param)
{
    static_cast<NatTraverser*>(param)->CompletedNatPropertyDetection(true);
}

static void NatPortDetectionCompletionCallback(Result, void* param)
{
    static_cast<NatTraverser*>(param)->CompletedNatPortDetection(true);
}

}
}
}
