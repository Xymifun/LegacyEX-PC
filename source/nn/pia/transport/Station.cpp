#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"
#include "nn/pia/HeapManager.hpp"
#include "nn/pia/NetworkFactory.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/transport/ConnectStationJob.hpp"
#include "nn/pia/transport/DisconnectStationJob.hpp"
#include "nn/pia/transport/IdentificationInfoTable.hpp"
#include "nn/pia/transport/ProcessConnectionRequestJob.hpp"
#include "nn/pia/transport/ReliableSlidingWindow.hpp"
#include "nn/pia/transport/RttProtocol.hpp"
#include "platform/wiiu/pead.hpp"

#include <new>

namespace nn
{
namespace pia
{
namespace transport
{

namespace
{
enum
{
    cStationState5 = 5
};
}

Station::Station()
    : m_field0(0),
      m_stationAddress(),
      m_field10(0xfd),
      m_stationId(0),
      m_field20(0),
      m_reliableSlidingWindow(0),
      m_field28(0),
      m_rttProtocol(0),
      m_connectStationJob(0),
      m_disconnectStationJob(0),
      m_processConnectionRequestJob(0),
      m_sequenceIdController(),
      m_field54(0),
      m_field55(0),
      m_field56(1),
      m_lastSendTime(),
      m_lastReceiveTime(),
      m_field68(0)
{
    void* memory = pead::allocateFromSeadHeap(sizeof(ReliableSlidingWindow), common::HeapManager::GetHeap());
    if (memory != 0)
        m_reliableSlidingWindow = new (memory) ReliableSlidingWindow();
    else
        m_reliableSlidingWindow = 0;
    memory = pead::allocateFromSeadHeap(sizeof(ProcessConnectionRequestJob), common::HeapManager::GetHeap());
    if (memory != 0)
        m_processConnectionRequestJob = new (memory) ProcessConnectionRequestJob();
    else
        m_processConnectionRequestJob = 0;
}

Station::~Station()
{
    if (m_processConnectionRequestJob != 0)
    {
        common::DeleteObj(m_processConnectionRequestJob);
        m_processConnectionRequestJob = 0;
    }
    if (m_reliableSlidingWindow != 0)
    {
        common::DeleteObj(m_reliableSlidingWindow);
        m_reliableSlidingWindow = 0;
    }
}

Result Station::GetPrincipalId(unsigned long long* principalId)
{
    if (!g_stationConnectionInfoTable)
        return cResultInvalidState;
    if (!principalId)
        return ResultInvalidArgument();
    StationConnectionInfo info;
    Result result = g_stationConnectionInfoTable->GetStationConnectionInfo(this, &info);
    if (result != 0)
    {
        g_stationConnectionInfoTable->Trace(0x8000);
        Trace(0x8000);
        return result;
    }
    *principalId = info.m_stationLocation.m_principalId;
    return 0;
}

Result Station::Initialize(NetworkFactory* factory, StationProtocol* protocol, RttProtocol* rtt)
{
    if (factory == 0)
        return cResultInvalidArgument;
    if (protocol == 0)
        return cResultInvalidArgument;
    if (rtt == 0)
        return cResultInvalidArgument;
    m_rttProtocol = rtt;
    m_field28 = protocol;
    m_field0 = 0;
    m_connectStationJob = factory->CreateConnectStationJob();
    m_disconnectStationJob = factory->CreateDisconnectStationJob();
    m_reliableSlidingWindow->Initialize(4, 4);
    return 0;
}

void Station::Finalize()
{
    m_field0 = 0;
    if (m_reliableSlidingWindow != 0)
        m_reliableSlidingWindow->Finalize();
    if (m_disconnectStationJob != 0)
    {
        g_piaTransport->GetNetworkFactory()->DeleteDisconnectStationJob(m_disconnectStationJob);
        m_disconnectStationJob = 0;
    }
    if (m_connectStationJob != 0)
    {
        g_piaTransport->GetNetworkFactory()->DeleteConnectStationJob(m_connectStationJob);
        m_connectStationJob = 0;
    }
    m_rttProtocol = 0;
    m_field28 = 0;
}

void Station::SetStationIdAndIndex(unsigned long long id, StationIndex index)
{
    m_stationId = id;
    m_field10 = index;
}

void Station::helperStartupCleanup(bool startup, unsigned long long id, StationIndex index,
                                   const common::StationAddress& address, StationState state)
{
    m_field0 = startup;
    if (startup)
    {
        m_sequenceIdController.Startup();
        common::Time time;
        time.SetNow();
        m_lastSendTime = time;
        m_lastReceiveTime = time;
    }
    else
    {
        m_sequenceIdController.Cleanup();
        m_reliableSlidingWindow->Cleanup();
    }
    m_stationId = id;
    m_field10 = index;
    m_stationAddress = address;
    m_field20 = state;
    m_field54 = 0;
    m_field55 = 0;
    m_field68 = 0;
}

bool Station::Startup(unsigned long long id, StationIndex index, const common::StationAddress& address)
{
    if (!common::isValidSourceStationIndex(index))
        return false;
    helperStartupCleanup(true, id, index, address, static_cast<StationState>(1));
    return true;
}

bool Station::Startup(const common::StationAddress& address)
{
    helperStartupCleanup(true, 0, static_cast<StationIndex>(0xfd), address, static_cast<StationState>(1));
    return true;
}

bool Station::Startup()
{
    common::StationAddress address;
    address.Clear();
    helperStartupCleanup(true, 0, static_cast<StationIndex>(0xfd), address, static_cast<StationState>(1));
    return true;
}

void Station::Cleanup()
{
    common::StationAddress address;
    address.Clear();
    helperStartupCleanup(false, 0, static_cast<StationIndex>(0xfd), address, static_cast<StationState>(0));
}

int Station::GetRtt() const
{
    if (m_rttProtocol == 0 || m_field10 == 0xfd)
        return -1;
    return m_rttProtocol->GetRtt(static_cast<StationIndex>(m_field10));
}

int Station::GetRtt(unsigned int index) const
{
    if (m_rttProtocol == 0 || m_field10 == 0xfd)
        return -1;
    return m_rttProtocol->GetRtt(static_cast<StationIndex>(m_field10), index);
}

void Station::ResetRttMinMax()
{
    if (m_rttProtocol == 0 || m_field10 == 0xfd)
        return;
    m_rttProtocol->ResetRttMinMax(static_cast<StationIndex>(m_field10));
}

int Station::GetRttMin() const
{
    if (m_rttProtocol == 0 || m_field10 == 0xfd)
        return -1;
    return m_rttProtocol->GetRttMin(static_cast<StationIndex>(m_field10));
}

int Station::GetRttMax() const
{
    if (m_rttProtocol == 0 || m_field10 == 0xfd)
        return -1;
    return m_rttProtocol->GetRttMax(static_cast<StationIndex>(m_field10));
}

void Station::CleanupJobs()
{
    if (m_connectStationJob != 0)
    {
        m_connectStationJob->Cleanup();
        m_connectStationJob->Reset(false);
    }
    if (m_disconnectStationJob != 0)
    {
        m_disconnectStationJob->Cleanup();
        m_disconnectStationJob->Reset(false);
    }
    if (m_processConnectionRequestJob != 0)
    {
        m_processConnectionRequestJob->Cleanup();
        m_processConnectionRequestJob->Reset(false);
    }
}

Result Station::GetPlayerCount(unsigned char* count)
{
    if (IdentificationInfoTable::GetInstance() == 0)
        return cResultInvalidState;
    return IdentificationInfoTable::GetInstance()->GetPlayerCount(this, count);
}

Result Station::GetParticipantCount(unsigned char* count)
{
    if (IdentificationInfoTable::GetInstance() == 0)
        return cResultInvalidState;
    return IdentificationInfoTable::GetInstance()->GetParticipantCount(this, count);
}

bool Station::IsConnectionRouteRelay() const
{
    if (m_field20 == cStationState5 && m_field56 == 0)
        return true;
    return false;
}

bool Station::IsConnectionRouteDirect() const
{
    if (m_field20 == cStationState5 && m_field56 != 0)
        return true;
    return false;
}

void Station::Trace(unsigned long long) const
{
}

Station::PlayerInfo::~PlayerInfo()
{
}

}
}
}
