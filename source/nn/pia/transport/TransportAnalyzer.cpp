#include "nn/pia/transport/TransportAnalyzer.hpp"

#include <new>
#include <stdint.h>
#include <string.h>

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Math.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/common/SessionStateMonitoringContent.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/transport/PacketHandler.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/transport/ThreadStreamManager.hpp"
#include "nn/pia/transport/TransportSupport.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

TransportAnalyzer* g_piaTransportAnalyzer = 0;

namespace
{
unsigned char* g_transportAnalyzerStorage = 0;
}

TransportAnalyzer::TransportAnalyzer() : m_startedUp(false)
{
}

TransportAnalyzer::~TransportAnalyzer()
{
}

Result TransportAnalyzer::CreateInstance()
{
    if (!IsInitialized())
        return cResultNotInitialized;
    if (!IsInSetupMode())
        return cResultInvalidState;
    if (g_piaTransportAnalyzer != 0)
        return cResultAlreadyExists;

    g_transportAnalyzerStorage = common::NewArray<unsigned char>(sizeof(TransportAnalyzer), 8);
    if (g_transportAnalyzerStorage == 0)
        return cResultOutOfMemory;

    g_piaTransportAnalyzer = new (g_transportAnalyzerStorage) TransportAnalyzer();
    if (g_piaTransportAnalyzer == 0)
    {
        common::DeleteArray<unsigned char>(g_transportAnalyzerStorage);
        g_transportAnalyzerStorage = 0;
        return cResultOutOfMemory;
    }
    return 0;
}

void TransportAnalyzer::DestroyInstance()
{
    if (g_piaTransportAnalyzer != 0)
    {
        g_piaTransportAnalyzer->~TransportAnalyzer();
        g_piaTransportAnalyzer = 0;
    }
    if (g_transportAnalyzerStorage != 0)
    {
        common::DeleteArray<unsigned char>(g_transportAnalyzerStorage);
        g_transportAnalyzerStorage = 0;
    }
}

Result TransportAnalyzer::Initialize()
{
    return 0;
}

void TransportAnalyzer::Finalize()
{
}

Result TransportAnalyzer::Startup()
{
    if (m_startedUp)
        return cResultInvalidState;
    if (g_piaThreadStreamManager == 0)
        return cResultInvalidState;

    m_analysisData.m_connectionAnalysisData.Clear();
    m_startTime = g_piaTransport->m_time;
    m_startedUp = true;
    return 0;
}

void TransportAnalyzer::Cleanup()
{
    if (!m_startedUp)
        return;
    m_startedUp = false;
}

Result TransportAnalyzer::Update()
{
    if (!m_startedUp)
        return ResultInvalidState();
    if (g_piaThreadStreamManager == 0)
        return ResultInvalidState();

    Result result = UpdateConnectionAnalysisData();
    if (result != 0)
        return result;

    PacketHandler* handler = g_piaTransport->m_packetHandler;
    result = handler->GetPacketAnalysisData(
        &m_analysisData.m_packetAnalysisData, &m_analysisData.m_receivedPacketAnalysisData,
        &m_analysisData.m_sentPacketAnalysisData, &m_analysisData.m_retransmittedPacketAnalysisData);
    if (result != 0)
        return result;

    handler->ClearPacketAnalysisData();
    m_analysisData.m_dispatchCount = g_piaTransport->m_dispatchCount;
    g_piaTransport->m_dispatchCount = 0;
    return 0;
}

Result TransportAnalyzer::UpdateConnectionAnalysisData()
{
    if (!m_startedUp)
        return cResultInvalidState;
    StationManager* const manager = StationManager::s_pInstance;
    if (manager == 0)
        return cResultInvalidState;

    bool isUpdated[ConnectionAnalysisData::cStationNumMax] = {};
    Station* const localStation = manager->m_localStation;
    for (Station** station = manager->m_stations.begin(); station != manager->m_stations.end(); ++station)
    {
        if (localStation == *station)
            continue;
        const unsigned int stationIndex = (*station)->m_field10;
        if (stationIndex >= ConnectionAnalysisData::cStationNumMax)
            continue;
        const SequenceIdController* const sequence = &(*station)->m_sequenceIdController;
        if (sequence == 0)
            continue;

        isUpdated[stationIndex] = true;
        ConnectionAnalysisData::Entry& entry = m_analysisData.m_connectionAnalysisData.m_entries[stationIndex];
        entry.m_rtt = (*station)->GetRtt();
        entry.m_rttMin = (*station)->GetRttMin();
        entry.m_rttMax = (*station)->GetRttMax();
        (*station)->ResetRttMinMax();
        const unsigned int sendPacketNum = sequence->m_sequenceCount;
        const unsigned int lostPacketNum = sequence->m_missingSequenceCount;
        if (entry.m_isValid)
        {
            entry.m_prevSendPacketNum = entry.m_sendPacketNum;
            entry.m_sendPacketNum = sendPacketNum;
            entry.m_prevLostPacketNum = entry.m_lostPacketNum;
            entry.m_lostPacketNum = lostPacketNum;
        }
        else
        {
            entry.m_isValid = true;
            entry.m_prevSendPacketNum = entry.m_sendPacketNum = sendPacketNum;
            entry.m_prevLostPacketNum = entry.m_lostPacketNum = lostPacketNum;
        }
    }

    for (int index = 0; index < ConnectionAnalysisData::cStationNumMax; ++index)
    {
        if (!isUpdated[index])
            m_analysisData.m_connectionAnalysisData.m_entries[index].m_isValid = false;
    }

    common::TimeSpan elapsed;
    if (g_piaTransport != 0)
    {
        elapsed = g_piaTransport->m_time - m_startTime;
    }
    else
    {
        common::Time now;
        now.SetNow();
        elapsed = now - m_startTime;
    }
    m_analysisData.m_connectionAnalysisData.m_passedMilliSecond = elapsed.m_tick
                                                                  / common::TimeSpan::GetTicksPerMSec().m_tick;
    return 0;
}

void TransportAnalyzer::SetMonitoringData()
{
    PacketHandler* const handler = g_piaTransport->m_packetHandler;
    PacketAnalysisData data;
    const Result sendResult = handler->GetPacketAnalysisData(&data, 0, 0, 0);
    if (sendResult != 0)
        return;

    common::g_sessionStateMonitoringContent.m_metric1[3] = 0;
    common::g_sessionStateMonitoringContent.m_metric20[0] = data.m_totalPacketNum;
    common::g_sessionStateMonitoringContent.m_metric36[0] = data.m_totalPacketSize;
    const int sendEntryNum = common::minimum<int>(data.m_entryNum, 15);
    for (int index = 0; index < sendEntryNum; ++index)
    {
        common::g_sessionStateMonitoringContent.m_metric1[4 + index] = data.m_entries[index].m_protocolId.value();
        common::g_sessionStateMonitoringContent.m_metric20[1 + index] = data.m_entries[index].m_totalPacketNum;
        common::g_sessionStateMonitoringContent.m_metric36[1 + index] = data.m_entries[index].m_totalPacketSize;
    }

    const Result receiveResult = handler->GetPacketAnalysisData(0, 0, 0, &data);
    if (receiveResult != 0)
        return;

    common::g_sessionStateMonitoringContent.m_metric52[0] = 0;
    common::g_sessionStateMonitoringContent.m_metric68[0] = data.m_totalPacketNum;
    common::g_sessionStateMonitoringContent.m_metric84[0] = data.m_totalPacketSize;
    const int receiveEntryNum = common::minimum<int>(data.m_entryNum, 15);
    for (int index = 0; index < receiveEntryNum; ++index)
    {
        common::g_sessionStateMonitoringContent.m_metric52[1 + index] = data.m_entries[index].m_protocolId.value();
        common::g_sessionStateMonitoringContent.m_metric68[1 + index] = data.m_entries[index].m_totalPacketNum;
        common::g_sessionStateMonitoringContent.m_metric84[1 + index] = data.m_entries[index].m_totalPacketSize;
    }

    StationManager* const manager = StationManager::s_pInstance;
    if (manager == 0)
        return;
    Station* const localStation = manager->m_localStation;
    if (localStation == 0)
        return;

    int stationNum = 0;
    for (Station** station = manager->m_stations.begin(); station != manager->m_stations.end(); ++station)
    {
        if (stationNum >= 15)
            break;
        if (localStation == *station)
            continue;
        const SequenceIdController* const sequence = &(*station)->m_sequenceIdController;
        if (sequence == 0)
            continue;

        const unsigned int stationIndex = (*station)->m_field10;
        const int rtt = (*station)->GetRtt();
        const unsigned int sendPacketNum = sequence->m_sequenceCount;
        const unsigned int lostPacketNum = sequence->m_missingSequenceCount;
        unsigned short lossRate = 0xffff;
        if (sendPacketNum != 0)
            lossRate = static_cast<unsigned short>(static_cast<unsigned long long>(lostPacketNum) * 10000
                                                   / sendPacketNum);

        unsigned long long principalId;
        const Result principalResult = (*station)->GetPrincipalId(&principalId);
        if (principalResult != 0)
            common::g_sessionStateMonitoringContent.m_metric100[stationNum] = common::hashWithMd5(
                0xffffffffffffffffULL);
        else
            common::g_sessionStateMonitoringContent.m_metric100[stationNum] = common::hashWithMd5(principalId);
        common::g_sessionStateMonitoringContent.m_metric115[stationNum] = static_cast<unsigned char>(stationIndex);
        common::g_sessionStateMonitoringContent.m_metric130[stationNum] = static_cast<unsigned short>(rtt);
        common::g_sessionStateMonitoringContent.m_metric145[stationNum] = lossRate;
        ++stationNum;
    }
}

}
}
}
