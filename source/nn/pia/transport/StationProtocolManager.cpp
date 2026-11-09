#include "nn/pia/transport/StationProtocolManager.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/transport/RttProtocol.hpp"
#include "nn/pia/transport/StationProtocol.hpp"
#include "nn/pia/transport/TransportSupport.hpp"

#include <new>

namespace nn { namespace pia { namespace transport {

StationProtocolManager* g_piaStationProtocolManager = 0;

namespace {

unsigned char* g_stationProtocolManagerStorage = 0;

}

StationProtocolManager::StationProtocolManager()
    : m_stationProtocolId(0), m_rttProtocolId(0)
{
}

StationProtocolManager::~StationProtocolManager()
{
}

Result StationProtocolManager::CreateInstance()
{
    if (IsInitialized() == 0)
        return cResultNotInitialized;
    if (IsInSetupMode() == 0)
        return cResultInvalidState;
    if (g_piaStationProtocolManager != 0)
        return cResultAlreadyExists;
    g_stationProtocolManagerStorage = common::NewArray<unsigned char>(sizeof(StationProtocolManager), 8);
    if (g_stationProtocolManagerStorage == 0)
        return cResultOutOfMemory;
    g_piaStationProtocolManager = new (g_stationProtocolManagerStorage) StationProtocolManager();
    return 0;
}

void StationProtocolManager::DestroyInstance()
{
    if (g_piaStationProtocolManager != 0)
    {
        g_piaStationProtocolManager->~StationProtocolManager();
        g_piaStationProtocolManager = 0;
    }
    if (g_stationProtocolManagerStorage != 0)
    {
        common::DeleteArray<unsigned char>(g_stationProtocolManagerStorage);
        g_stationProtocolManagerStorage = 0;
    }
}

Result StationProtocolManager::Initialize(unsigned short stationNum, int initialPulseInterval)
{
    Transport* transport = g_piaTransport;
    if (transport == 0)
        return cResultInvalidState;
    m_stationProtocolId = transport->m_protocolManager.CreateProtocol<StationProtocol>(0);
    if (m_stationProtocolId == 0)
        return cResultInvalidState;
    StationProtocol* stationProtocol = transport->m_protocolManager.GetProtocol<StationProtocol>(m_stationProtocolId);
    Result result = stationProtocol->Initialize();
    PIA_ASSERT(result == 0);
    m_rttProtocolId = transport->m_protocolManager.CreateProtocol<RttProtocol>(0);
    if (m_rttProtocolId == 0)
        return cResultInvalidState;
    RttProtocol* rttProtocol = transport->m_protocolManager.GetProtocol<RttProtocol>(m_rttProtocolId);
    result = rttProtocol->Initialize(stationNum, initialPulseInterval);
    PIA_ASSERT(result == 0);
    return 0;
}

void StationProtocolManager::Finalize()
{
    Transport* transport = g_piaTransport;
    RttProtocol* rttProtocol;
    StationProtocol* stationProtocol;
    if (transport == 0)
        return;
    if (m_rttProtocolId != 0)
    {
        rttProtocol = transport->m_protocolManager.GetProtocol<RttProtocol>(m_rttProtocolId);
        rttProtocol->Finalize();
        transport->m_protocolManager.DestroyProtocol(m_rttProtocolId);
        m_rttProtocolId = 0;
    }
    if (m_stationProtocolId != 0)
    {
        stationProtocol = transport->m_protocolManager.GetProtocol<StationProtocol>(m_stationProtocolId);
        stationProtocol->Finalize();
        transport->m_protocolManager.DestroyProtocol(m_stationProtocolId);
        m_stationProtocolId = 0;
    }
}

StationProtocol* StationProtocolManager::GetStationProtocol()
{
    return g_piaTransport->m_protocolManager.GetProtocol<StationProtocol>(m_stationProtocolId);
}

RttProtocol* StationProtocolManager::GetRttProtocol()
{
    return g_piaTransport->m_protocolManager.GetProtocol<RttProtocol>(m_rttProtocolId);
}

void StationProtocolManager::Trace(unsigned long long) const
{
}

}}}
