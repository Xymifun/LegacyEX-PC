#include "nn/pia/session/ProcessUpdateMeshJob.hpp"

#include <coreinit/memory.h>
#include <new>
#include <string.h>

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/SessionStateMonitoringContent.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/session/JoinMeshJob.hpp"
#include "nn/pia/session/LeaveMeshJob.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/MeshProtocol.hpp"
#include "nn/pia/session/ProcessHostMigrationJob.hpp"
#include "nn/pia/transport/ConnectStationJob.hpp"
#include "nn/pia/transport/DisconnectStationJob.hpp"
#include "nn/pia/transport/IdentificationInfoTable.hpp"
#include "nn/pia/transport/MissingStationHandler.hpp"
#include "nn/pia/transport/RelayRouteManager.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationConnectionInfo.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/transport/StationProtocol.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn
{
namespace pia
{
namespace session
{

namespace
{
enum
{
    cStationIndexMax = 31,
    cStationIndexNone = 0xfd,
    cContextIndexNone = 0xff,
    cStationStateConnecting = 2,
    cStationStateConnected = 5,
    cStationStateReconnect = 6,
    cTraceStationLevel = 0x20000000,
    cUpdateMeshStartupResultStarted = 4,
    cUpdateMeshResultPartial = 1,
    cUpdateMeshResultOld = 2,
    cUpdateMeshResultInvalid = 0,
    cMessageFullList = 1,
    cMessagePartialList = 2,
    cMessageHeaderSize = 12,
    cEntryAlignMask = 3,
    cDefaultTimeLimitMsec = 20000,
    cDefaultReportTimeLimitMsec = 10000,
    cTimeLimitBaseMsec = 25000,
    cTimeLimitMediumMsec = 30000,
    cTimeLimitLargeMsec = 35000,
    cTimeLimitTopologyExtraMsec = 10000,
    cTimeLimitPerStationMsec = 1000,
    cStationCountSmallMax = 8,
    cStationCountMediumMax = 12,
    cFailureNoticeIntervalMsec = 500,
    cDummyMessageIntervalMsec = 300,
    cNetworkTopologyRelay = 1,
    cFailureRelayOffset = 0x10,
    cDisconnectReasonLeave = 2,
    cDisconnectReasonUpdateFailed = 6,
    cResultJoinFailed = 0x6439,
    cResultUpdateTimeout = 0x6c0e,
    cResultCancelled = 0x6c05,
    cResultCode643f = 0x643f,
    cResultCode6442 = 0x6442,
    cResultCode6443 = 0x6443,
    cResultCode6444 = 0x6444,
    cResultCode6445 = 0x6445,
    cResultCode644a = 0x644a,
    cResultCode6467 = 0x6467,
    cResultCode6468 = 0x6468,
    cResultCode6469 = 0x6469,
    cResultCode646a = 0x646a,
    cFailureCancelled = 1,
    cFailureCode643f = 2,
    cFailureCode6442 = 3,
    cFailureCode6443 = 5,
    cFailureCode6444 = 6,
    cFailureCode6445 = 7,
    cFailureCode644a = 8,
    cFailureTimeout = 9,
    cFailureCode6467 = 10,
    cFailureCode6468 = 11,
    cFailureCode6469 = 12,
    cFailureCode646a = 13,
    cFailureNoneMax = 0x10,
    cStepStateAny = 0xffff
};

const unsigned long long cPrincipalIdInvalid = 0xffffffffULL;

const char* const cStepNameUpdateFailed = "ProcessUpdateMeshJob::UpdateFailed";
const char* const cStepNameWaitDirectConnection = "ProcessUpdateMeshJob::WaitDirectConnection";
const char* const cStepNameCheckConnectionAll = "ProcessUpdateMeshJob::CheckConnectionAll";
const char* const cStepNameSendConnectionReport = "ProcessUpdateMeshJob::SendConnectionReport";
const char* const cStepNameStartRelayConnection = "ProcessUpdateMeshJob::StartRelayConnection";
const char* const cStepNameWaitLeaveMesh = "ProcessUpdateMeshJob::WaitLeaveMesh";
const char* const cStepNameCleanupByProcessFailure = "ProcessUpdateMeshJob::CleanupByProcessFailure";

typedef common::Job::ExecuteResult (ProcessUpdateMeshJob::*StepFunction)();

inline void SelectStep(common::StepSequenceJob::Step& slot, StepFunction function, const char* name)
{
    const common::StepSequenceJob::Step step(
        static_cast<common::Job::ExecuteResult (common::StepSequenceJob::*)()>(function), name);
    slot = step;
}

inline common::Time timeAfterNow(int msec)
{
    common::Time time;
    time.m_tick = common::g_piaJobScheduler->m_dispatchTime.m_tick + msec * common::TimeSpan::GetTicksPerMSec().m_tick;
    return time;
}

inline void destroyStation(transport::Station* station)
{
    station->m_disconnectStationJob->ProcessDisconnection(station);
    transport::g_stationConnectionInfoTable->EraseFromTable(station);
    transport::IdentificationInfoTable::GetInstance()->EraseFromTable(station);
    station->Cleanup();
    station->CleanupJobs();
    transport::StationManager::s_pInstance->DestroyStation(station);
}

inline bool isFinished(const common::CallContext& context)
{
    return context.m_state - common::CallContext::cStateSuccess < 3;
}

void readStationEntries(transport::StationConnectionInfo* infos, StationIndex* indices, const unsigned char* data,
                        unsigned int infoSize, unsigned int stride, unsigned int start, unsigned int count)
{
    for (unsigned int entry = 0; entry < count; ++entry)
    {
        const unsigned char* source = data + cMessageHeaderSize + stride * entry;
        const unsigned char index = source[infoSize];
        infos[start + entry].Deserialize(source);
        indices[start + entry] = static_cast<StationIndex>(index);
    }
}
}

ProcessUpdateMeshJob::ProcessUpdateMeshJob()
    : common::StepSequenceJob(),
      m_updateDeadline(),
      m_updateTimeLimitMsec(cDefaultTimeLimitMsec),
      m_updateExtraTimeMsec(cDefaultTimeLimitMsec),
      m_reportDeadline(),
      m_reportTimeLimitMsec(cDefaultReportTimeLimitMsec),
      m_failureNoticeTime(),
      m_callContext()
{
    OSBlockSet(m_contextIndex, cContextIndexNone, sizeof(m_contextIndex));
    m_started = false;
    m_partHostSlot = 0;
    m_partVersion = 0;
    m_relayRouteVersion = 0;
    m_partReceived[1] = false;
    m_updateVersion = 0;
    m_reportCount = 0;
    m_partCount = 0;
    m_partStationCount = 0;
    m_partReceived[0] = false;
    m_stationCount = 0;
    m_stationNum = g_piaMesh->m_unknown54;
    m_connectContexts = common::NewArray<common::CallContext>(m_stationNum);
    m_stationConnectionInfos = common::NewArray<transport::StationConnectionInfo>(m_stationNum);
    m_stationIndices = common::NewArray<StationIndex>(m_stationNum);
    m_partConnectionInfos = common::NewArray<transport::StationConnectionInfo>(m_stationNum);
    m_partStationIndices = common::NewArray<StationIndex>(m_stationNum);
    m_failureCodes = common::NewArray<unsigned char>(m_stationNum);
    for (unsigned short index = 0; index < m_stationNum; ++index)
        m_failureCodes[index] = 0;
    m_relayRequested = false;
    m_leaveMeshOnFailure = false;
    m_relayAttemptCount = 0;
    m_updateMeshSucceeded = true;
    m_callContext.Reset();
}

ProcessUpdateMeshJob::~ProcessUpdateMeshJob()
{
    common::DeleteArray(m_connectContexts);
    common::DeleteArray(m_stationConnectionInfos);
    common::DeleteArray(m_stationIndices);
    common::DeleteArray(m_partConnectionInfos);
    common::DeleteArray(m_partStationIndices);
    common::DeleteArray(m_failureCodes);
}

void ProcessUpdateMeshJob::Trace(unsigned long long) const
{
}

int ProcessUpdateMeshJob::SetStationDataList(const unsigned char* data, unsigned int size)
{
    const unsigned int infoSize = m_stationConnectionInfos[0].GetSerializedSize();
    const unsigned char stationCount = data[1];
    const unsigned char hostSlot = data[2];
    const unsigned int stride = infoSize + (~infoSize & cEntryAlignMask) + 1;
    const unsigned int version = common::deserializeU32(data + 4);
    if (stationCount > m_stationNum)
        return cUpdateMeshResultInvalid;

    if (data[8] == cMessageFullList)
    {
        if (m_partReceived[0] || m_partReceived[1])
        {
            if (version < m_partVersion)
                return cUpdateMeshResultOld;
            m_partStationCount = 0;
            m_partHostSlot = 0;
            m_partVersion = 0;
            m_partCount = 0;
            m_partReceived[0] = false;
            m_partReceived[1] = false;
        }
        for (unsigned int entry = 0; entry < stationCount; ++entry)
        {
            const unsigned char index = data[cMessageHeaderSize + stride * entry + infoSize];
            m_stationConnectionInfos[entry].Deserialize(data + cMessageHeaderSize + stride * entry);
            m_stationIndices[entry] = static_cast<StationIndex>(index);
        }
        m_updateVersion = version;
        m_stationCount = stationCount;
        for (unsigned short index = 0; index < m_stationNum; ++index)
            m_failureCodes[index] = 0;
        m_failureNoticeTime = common::g_piaJobScheduler->m_dispatchTime;
        return cUpdateMeshStartupResultStarted;
    }

    if (data[8] != cMessagePartialList)
        return cUpdateMeshResultInvalid;

    const unsigned char partIndex = data[9];
    if (!m_partReceived[0] && !m_partReceived[1])
    {
        m_partStationCount = stationCount;
        m_partHostSlot = hostSlot;
        m_partCount = cMessagePartialList;
        m_partVersion = version;
    }
    else
    {
        if (m_partStationCount != stationCount || m_partHostSlot != hostSlot || m_partVersion != version
            || m_partCount != cMessagePartialList)
        {
            if (m_partVersion >= version)
                return cUpdateMeshResultOld;
            m_partStationCount = stationCount;
            m_partHostSlot = hostSlot;
            m_partCount = cMessagePartialList;
            m_partVersion = version;
            m_partReceived[1] = false;
            m_partReceived[0] = false;
        }
        if (m_partReceived[partIndex])
            return cUpdateMeshResultPartial;
    }

    const unsigned char entryCount = data[10];
    const unsigned char entryStart = data[11];
    for (unsigned int entry = 0; entry < entryCount; ++entry)
    {
        const unsigned char index = data[cMessageHeaderSize + stride * entry + infoSize];
        m_stationConnectionInfos[entryStart + entry].Deserialize(data + cMessageHeaderSize + stride * entry);
        m_stationIndices[entryStart + entry] = static_cast<StationIndex>(index);
    }
    m_partReceived[partIndex] = true;
    if (!m_partReceived[0] || !m_partReceived[1])
        return cUpdateMeshResultPartial;

    m_updateVersion = m_partVersion;
    m_partVersion = 0;
    m_partHostSlot = 0;
    m_stationCount = m_partStationCount;
    m_partCount = 0;
    m_partStationCount = 0;
    m_partReceived[0] = false;
    m_partReceived[1] = false;
    for (unsigned short index = 0; index < m_stationNum; ++index)
        m_failureCodes[index] = 0;
    m_failureNoticeTime = common::g_piaJobScheduler->m_dispatchTime;
    return cUpdateMeshStartupResultStarted;
}

void ProcessUpdateMeshJob::CalcTimeLimit(bool relay)
{
    const int localStationIndex = g_piaMesh->GetLocalStationIndex();
    const int stationTimeMsec = (transport::g_piaTransport->m_stationNum - localStationIndex)
                                * cTimeLimitPerStationMsec;
    if (!relay)
    {
        int topologyExtraMsec = 0;
        if (g_piaMesh->GetNetworkTopologyType() == cNetworkTopologyRelay)
            topologyExtraMsec = cTimeLimitTopologyExtraMsec;
        int limitMsec;
        if (m_stationCount <= cStationCountSmallMax)
        {
            limitMsec = stationTimeMsec + cTimeLimitBaseMsec;
        }
        else
        {
            int baseMsec = cTimeLimitLargeMsec;
            if (m_stationCount <= cStationCountMediumMax)
                baseMsec = cTimeLimitMediumMsec;
            limitMsec = stationTimeMsec + baseMsec;
        }
        m_reportTimeLimitMsec = limitMsec;
        m_updateTimeLimitMsec = limitMsec + topologyExtraMsec;
    }
    else
    {
        const int limitMsec = m_updateExtraTimeMsec + stationTimeMsec;
        m_reportTimeLimitMsec = limitMsec;
        m_updateTimeLimitMsec = limitMsec;
    }
    m_reportDeadline = timeAfterNow(m_reportTimeLimitMsec);
    m_updateDeadline = timeAfterNow(m_updateTimeLimitMsec);

    if (!relay && !m_leaveMeshOnFailure)
    {
        const common::TimeSpan extraTime(m_updateExtraTimeMsec * common::TimeSpan::GetTicksPerMSec().m_tick);
        m_updateDeadline.m_tick += extraTime.m_tick;
    }
}

unsigned char ProcessUpdateMeshJob::GetContextIndex(StationIndex stationIndex) const
{
    return m_contextIndex[stationIndex & 0xff];
}

void ProcessUpdateMeshJob::ClearContextIndex(StationIndex stationIndex)
{
    m_contextIndex[stationIndex & 0xff] = cContextIndexNone;
}

unsigned char ProcessUpdateMeshJob::GetUsableContextIndex() const
{
    unsigned char contextIndex = cContextIndexNone;
    for (unsigned short candidate = 0; candidate < m_stationNum; ++candidate)
    {
        bool inUse = false;
        for (int index = 0; index < cStationNumMax; ++index)
        {
            if (m_contextIndex[index] == candidate)
            {
                inUse = true;
                break;
            }
        }
        if (!inUse && m_connectContexts[candidate].m_state != common::CallContext::cStateInProgress)
        {
            contextIndex = static_cast<unsigned char>(candidate);
            break;
        }
    }
    return contextIndex;
}

void ProcessUpdateMeshJob::SetContextIndex(StationIndex stationIndex, unsigned char contextIndex)
{
    m_contextIndex[stationIndex & 0xff] = contextIndex;
}

bool ProcessUpdateMeshJob::UpdateMesh()
{
    if (m_updateDeadline <= common::g_piaJobScheduler->m_dispatchTime)
        return false;

    const int localIndex = g_piaMesh->GetLocalStationIndex();
    const int hostIndex = g_piaMesh->GetHostStationIndex();
    if (localIndex > cStationIndexMax || hostIndex > cStationIndexMax
        || transport::StationManager::s_pInstance->m_localStation == 0)
        return false;

    const bool connectDirect = !m_relayRequested;
    unsigned int knownBitmap = 0;
    bool changed = false;
    Mesh* mesh = g_piaMesh;
    transport::StationConnectionInfoTable* table = transport::g_stationConnectionInfoTable;
    transport::StationConnectionInfo info;
    g_piaMesh->GetNetworkTopologyType();
    for (unsigned short entry = 0; entry < m_stationCount; ++entry)
    {
        knownBitmap |= 1 << m_stationIndices[entry];
        transport::Station* station = 0;
        if (mesh->IsValidStationIndex(m_stationIndices[entry]))
            station = transport::StationManager::s_pInstance->GetStation(m_stationIndices[entry]);

        if (station != 0)
        {
            if (table->GetStationConnectionInfo(station, &info) != 0)
            {
                station->Trace(cTraceStationLevel);
            }
            else
            {
                if (info == m_stationConnectionInfos[entry])
                    continue;
                if (m_stationIndices[entry] == localIndex)
                {
                    if (!m_leaveMeshOnFailure)
                        return false;
                    continue;
                }
                if (GetContextIndex(m_stationIndices[entry]) < m_stationNum)
                    ClearContextIndex(m_stationIndices[entry]);
                mesh->UnfixDisconnectedId(m_stationIndices[entry]);
                destroyStation(station);
                changed = true;
            }
        }
        station = table->GetStation(m_stationConnectionInfos[entry]);

        if (station == 0)
        {
            if (m_stationConnectionInfos[entry].m_stationLocation.m_stationAddress.IsValid())
            {
                station = transport::StationManager::s_pInstance->GetStation(
                    m_stationConnectionInfos[entry].m_stationLocation.m_stationAddress);
                if (station == 0
                    || (station == transport::StationManager::s_pInstance->m_localStation
                        && m_stationIndices[entry] != g_piaMesh->GetLocalStationIndex())
                    || table->GetStationConnectionInfo(station, &info) == 0)
                {
                    station = table->GetStation(
                        static_cast<unsigned int>(m_stationConnectionInfos[entry].m_stationLocation.m_field1c));
                    if (station != 0
                        && (station->m_field20 == cStationStateConnected
                            || station->m_field20 == cStationStateReconnect))
                    {
                        station->Trace(cTraceStationLevel);
                        if (GetContextIndex(m_stationIndices[entry]) < m_stationNum)
                            ClearContextIndex(m_stationIndices[entry]);
                        const int stationIndex = station->m_field10;
                        if (stationIndex != m_stationIndices[entry] && stationIndex <= cStationIndexMax)
                            mesh->UnfixDisconnectedId(static_cast<StationIndex>(stationIndex));
                        destroyStation(station);
                        changed = true;
                        station = 0;
                    }
                }
            }
        }

        if (station != 0)
        {
            const int stationIndex = station->m_field10;
            if (stationIndex != m_stationIndices[entry] && stationIndex <= cStationIndexMax)
            {
                if (mesh->IsValidStationIndex(static_cast<StationIndex>(stationIndex)))
                {
                    if (station->m_field10 == localIndex)
                        return false;
                    mesh->UnfixDisconnectedId(static_cast<StationIndex>(station->m_field10));
                }
                destroyStation(station);
                changed = true;
            }
            else
            {
                if (station->m_field20 == cStationStateConnected && stationIndex > cStationIndexMax)
                    changed = true;
                station->SetStationIdAndIndex(m_stationConnectionInfos[entry].m_stationLocation.m_principalId,
                                              m_stationIndices[entry]);
                continue;
            }
        }

        if (m_stationIndices[entry] == localIndex)
            return false;
        if (m_stationIndices[entry] == hostIndex)
        {
            if (!g_piaMesh->IsHostMigrationEnabled() || !g_piaMesh->m_processHostMigrationJob->m_started)
                return false;
        }
        if (localIndex <= static_cast<int>(m_stationIndices[entry]))
        {
            g_piaMesh->GetNetworkTopologyType();
            continue;
        }
        if (!connectDirect)
            continue;
        if (transport::StationManager::s_pInstance->GetValidStationNum() >= transport::g_piaTransport->m_stationNum)
            return false;

        transport::Station* newStation = transport::StationManager::s_pInstance->CreateStation();
        newStation->Startup(m_stationConnectionInfos[entry].m_stationLocation.m_principalId, m_stationIndices[entry],
                            m_stationConnectionInfos[entry].m_stationLocation.m_stationAddress);
        transport::ConnectStationJob* connectJob = newStation->m_connectStationJob;
        const unsigned char contextIndex = GetUsableContextIndex();
        if (connectJob->Startup(m_connectContexts + contextIndex, newStation, m_stationConnectionInfos[entry], false,
                                newStation->m_field28->m_timeoutMsec)
            != 0)
            return false;
        table->AddToTable(newStation, m_stationConnectionInfos[entry]);
        newStation->m_field20 = cStationStateConnecting;
        SetContextIndex(m_stationIndices[entry], contextIndex);
        connectJob->Ready(false);
    }

    const unsigned int staleBitmap = mesh->m_validStationBitmap & ~knownBitmap;
    if (staleBitmap != 0)
    {
        unsigned int bit = 1;
        for (unsigned char index = 0; index < cStationNumMax; ++index, bit <<= 1)
        {
            if ((staleBitmap & bit) == 0)
                continue;
            if (index == mesh->GetLocalStationIndex())
            {
                if (!m_leaveMeshOnFailure)
                    return false;
            }
            else
            {
                mesh->UnfixDisconnectedId(static_cast<StationIndex>(index));
                transport::Station* station = transport::StationManager::s_pInstance->GetStation(
                    static_cast<StationIndex>(index));
                if (station != 0)
                    destroyStation(station);
                transport::RelayRouteManager* relayRouteManager = transport::g_piaTransport->m_relayRouteManager;
                if (relayRouteManager != 0)
                    relayRouteManager->SearchRefugeRelayRoute(static_cast<StationIndex>(index));
            }
        }
        changed = true;
    }

    transport::Station* extraStations[cStationNumMax] = {};
    transport::StationManager* stationManager = transport::StationManager::s_pInstance;
    unsigned short extraCount = 0;
    for (transport::Station** it = stationManager->m_stations.data;
         it != stationManager->m_stations.data + stationManager->m_stations.size; ++it)
    {
        transport::Station* station = *it;
        if (station == stationManager->m_localStation || station->m_field20 == cStationStateReconnect)
            continue;
        bool listed = false;
        if (table->GetStationConnectionInfo(station, &info) == 0)
        {
            for (unsigned short entry = 0; entry < m_stationCount; ++entry)
            {
                if (info == m_stationConnectionInfos[entry])
                {
                    listed = true;
                    break;
                }
            }
        }
        if (listed)
            continue;
        extraStations[extraCount] = *it;
        ++extraCount;
    }

    for (unsigned short index = 0; index < extraCount; ++index)
    {
        transport::Station* station = extraStations[index];
        station->Trace(cTraceStationLevel);
        if (!station->m_connectStationJob->IsRunning() && !station->m_processConnectionRequestJob->IsRunning()
            && station->m_field20 != cStationStateConnected)
        {
            const int stationIndex = station->m_field10;
            destroyStation(station);
            transport::Station* remaining = stationManager->GetStation(static_cast<StationIndex>(stationIndex));
            if (stationIndex <= cStationIndexMax && (remaining == 0 || remaining->m_field20 == cStationStateConnected))
            {
                if (GetContextIndex(static_cast<StationIndex>(stationIndex)) < m_stationNum)
                    ClearContextIndex(static_cast<StationIndex>(stationIndex));
            }
            changed = true;
        }
    }

    if (changed)
        transport::g_piaTransport->OutputStreamUpdateEvent();
    return true;
}

int ProcessUpdateMeshJob::Startup(const unsigned char* data, unsigned int size, bool relay)
{
    int result = SetStationDataList(data, size);
    if (result == cUpdateMeshStartupResultStarted)
    {
        CalcTimeLimit(relay);
        m_relayRequested = relay;
        if (!relay)
        {
            m_relayRouteVersion = 0;
            m_reportCount = 0;
        }
        else
        {
            ++m_relayAttemptCount;
        }
        OSBlockSet(m_contextIndex, cContextIndexNone, sizeof(m_contextIndex));
        m_started = true;
        Reset(true);
        m_updateMeshSucceeded = UpdateMesh();

        if (m_updateMeshSucceeded)
        {
            if (g_piaMesh->GetNetworkTopologyType() == cNetworkTopologyRelay)
            {
                SelectStep(m_currentStep, &ProcessUpdateMeshJob::WaitDirectConnection, cStepNameWaitDirectConnection);
            }
            else
            {
                SelectStep(m_currentStep, &ProcessUpdateMeshJob::CheckConnectionAll, cStepNameCheckConnectionAll);
            }
        }
        else
        {
            SelectStep(m_currentStep, &ProcessUpdateMeshJob::UpdateFailed, cStepNameUpdateFailed);
        }
        result = cUpdateMeshStartupResultStarted;
    }
    return result;
}

void ProcessUpdateMeshJob::UpdateJoinMeshFailureResult(StationIndex stationIndex, unsigned char failure, bool relay)
{
    JoinMeshJob* joinMeshJob = g_piaMesh->m_joinMeshJob;
    const unsigned int current = joinMeshJob->m_joinResult;
    switch (failure)
    {
    case cFailureCode643f:
    case cFailureCode643f + cFailureRelayOffset:
        if (current != cResultCode6442 && current != cResultCode6445 && current != cResultCode644a
            && current != cResultCode6443 && current != cResultCode6444)
            joinMeshJob->m_joinResult = cResultCode643f;
        break;
    case cFailureCode6442:
    case cFailureCode6442 + cFailureRelayOffset:
        if (current != cResultCode6445 && current != cResultCode644a && current != cResultCode6443
            && current != cResultCode6444)
            joinMeshJob->m_joinResult = cResultCode6442;
        break;
    case 4:
    case 4 + cFailureRelayOffset:
    case cFailureCode6445:
    case cFailureCode6445 + cFailureRelayOffset:
        if (current != cResultCode644a)
            joinMeshJob->m_joinResult = cResultCode6445;
        break;
    case cFailureCode6443:
    case cFailureCode6443 + cFailureRelayOffset:
        if (current != cResultCode6445 && current != cResultCode644a)
            joinMeshJob->m_joinResult = relay ? cResultCode6444 : cResultCode6443;
        break;
    case cFailureCode6444:
    case cFailureCode6444 + cFailureRelayOffset:
        if (current != cResultCode6445 && current != cResultCode644a)
            joinMeshJob->m_joinResult = relay ? cResultCode6443 : cResultCode6444;
        break;
    case cFailureCode644a:
    case cFailureCode644a + cFailureRelayOffset:
        joinMeshJob->m_joinResult = cResultCode644a;
        break;
    case cFailureTimeout:
    case cFailureTimeout + cFailureRelayOffset:
        joinMeshJob->m_joinResult = cResultUpdateTimeout;
        break;
    case cFailureCode6467:
    case cFailureCode6467 + cFailureRelayOffset:
        if (current != cResultCode6442 && current != cResultCode6445 && current != cResultCode6443
            && current != cResultCode6444 && current != cResultCode644a && current != cResultCode646a
            && current != cResultCode6468 && current != cResultCode6469)
            joinMeshJob->m_joinResult = cResultCode6467;
        break;
    case cFailureCode6468:
    case cFailureCode6468 + cFailureRelayOffset:
        if (current != cResultCode6442 && current != cResultCode6445 && current != cResultCode6443
            && current != cResultCode6444 && current != cResultCode646a && current != cResultCode644a)
            joinMeshJob->m_joinResult = relay ? cResultCode6469 : cResultCode6468;
        break;
    case cFailureCode6469:
    case cFailureCode6469 + cFailureRelayOffset:
        if (current != cResultCode6442 && current != cResultCode6445 && current != cResultCode6443
            && current != cResultCode6444 && current != cResultCode646a && current != cResultCode644a)
            joinMeshJob->m_joinResult = relay ? cResultCode6468 : cResultCode6469;
        break;
    case cFailureCode646a:
    case cFailureCode646a + cFailureRelayOffset:
        if (current != cResultCode6445 && current != cResultCode644a)
            joinMeshJob->m_joinResult = cResultCode646a;
        break;
    default:
        break;
    }

    transport::Station* station = transport::StationManager::s_pInstance->GetStation(stationIndex);
    if (station == 0)
        g_piaMesh->m_joinMeshJob->m_principalId = cPrincipalIdInvalid;
    else
        g_piaMesh->m_joinMeshJob->m_principalId = transport::g_stationConnectionInfoTable->GetPrincipalIdByStation(
            station);
}

ProcessUpdateMeshJob::ExecuteResult ProcessUpdateMeshJob::WaitDirectConnection()
{
    if (m_updateDeadline <= common::g_piaJobScheduler->m_dispatchTime)
    {
        g_piaMesh->m_joinMeshJob->m_joinResult = cResultUpdateTimeout;
        SelectStep(m_currentStep, &ProcessUpdateMeshJob::UpdateFailed, cStepNameUpdateFailed);
        return MakeExecuteResult(cExecuteKindContinue);
    }
    const int localIndex = g_piaMesh->GetLocalStationIndex();
    if (localIndex > cStationIndexMax)
    {
        SelectStep(m_currentStep, &ProcessUpdateMeshJob::UpdateFailed, cStepNameUpdateFailed);
        return MakeExecuteResult(cExecuteKindContinue);
    }

    const bool noticeDue = m_failureNoticeTime < common::g_piaJobScheduler->m_dispatchTime;
    bool waiting = false;
    bool missing = false;
    bool changed = false;
    for (unsigned short entry = 0; entry < m_stationCount; ++entry)
    {
        const StationIndex index = m_stationIndices[entry];
        if (index == localIndex)
            continue;

        const unsigned char failureCode = m_failureCodes[entry];
        if (failureCode != 0)
        {
            UpdateJoinMeshFailureResult(index, failureCode, true);
            continue;
        }

        transport::Station* station = transport::StationManager::s_pInstance->GetStation(index);
        if (station == 0)
        {
            station = transport::g_stationConnectionInfoTable->GetStation(m_stationConnectionInfos[entry]);
            if (station != 0)
            {
                if (station->m_field20 == cStationStateConnected && station->m_field10 > cStationIndexMax)
                    changed = true;
                station->SetStationIdAndIndex(m_stationConnectionInfos[entry].m_stationLocation.m_principalId,
                                              m_stationIndices[entry]);
            }
        }

        const unsigned short contextIndex = GetContextIndex(m_stationIndices[entry]);
        if (contextIndex < m_stationNum)
        {
            common::CallContext& context = m_connectContexts[contextIndex];
            if (isFinished(context))
            {
                if (context.m_result == 0)
                {
                    ClearContextIndex(index);
                }
                else
                {
                    unsigned char failure = cFailureCancelled + cFailureRelayOffset;
                    if (context.m_result == cResultCancelled)
                        failure = cFailureCancelled + cFailureRelayOffset;
                    else if (context.m_result == cResultCode643f)
                        failure = cFailureCode643f + cFailureRelayOffset;
                    else if (context.m_result == cResultCode6442)
                        failure = cFailureCode6442 + cFailureRelayOffset;
                    else if (context.m_result == cResultCode6443)
                        failure = cFailureCode6443 + cFailureRelayOffset;
                    else if (context.m_result == cResultCode6444)
                        failure = cFailureCode6444 + cFailureRelayOffset;
                    else if (context.m_result == cResultCode6445)
                        failure = cFailureCode6445 + cFailureRelayOffset;
                    else if (context.m_result == cResultCode644a)
                        failure = cFailureCode644a + cFailureRelayOffset;
                    else if (context.m_result == cResultUpdateTimeout)
                        failure = cFailureTimeout + cFailureRelayOffset;
                    else if (context.m_result == cResultCode6467)
                        failure = cFailureCode6467 + cFailureRelayOffset;
                    else if (context.m_result == cResultCode6468)
                        failure = cFailureCode6468 + cFailureRelayOffset;
                    else if (context.m_result == cResultCode6469)
                        failure = cFailureCode6469 + cFailureRelayOffset;
                    else if (context.m_result == cResultCode646a)
                        failure = cFailureCode646a + cFailureRelayOffset;
                    UpdateJoinMeshFailureResult(index, failure, false);
                    if (noticeDue)
                    {
                        g_piaMesh->GetMeshProtocol()->SendConnectionFailureNotice(
                            g_piaMesh->GetHostStationIndex(), m_stationIndices[entry],
                            static_cast<StationIndex>(localIndex), failure, m_updateVersion);
                        m_failureNoticeTime = timeAfterNow(cFailureNoticeIntervalMsec);
                    }
                }
            }
            else
            {
                waiting = true;
            }
        }
        else if (station != 0)
        {
            if (station->m_field20 != cStationStateConnected)
                waiting = true;
        }
        else
        {
            missing = true;
        }
    }

    if (changed)
        transport::g_piaTransport->OutputStreamUpdateEvent();
    if (waiting)
        return MakeExecuteResult(cExecuteKindNextStep);
    if (missing && common::g_piaJobScheduler->m_dispatchTime < m_reportDeadline)
        return MakeExecuteResult(cExecuteKindNextStep);

    m_reportDeadline = common::g_piaJobScheduler->m_dispatchTime;
    SelectStep(m_currentStep, &ProcessUpdateMeshJob::SendConnectionReport, cStepNameSendConnectionReport);
    return MakeExecuteResult(cExecuteKindContinue);
}

bool ProcessUpdateMeshJob::SendConnectionReport(StationIndex stationIndex, unsigned int version,
                                                unsigned int* stationBitmap)
{
    const unsigned int reportCount = m_reportCount + 1;
    if (g_piaMesh->GetMeshProtocol()->SendConnectionReport(stationIndex, version, reportCount, stationBitmap))
    {
        m_reportCount = reportCount;
        return true;
    }
    return false;
}

ProcessUpdateMeshJob::ExecuteResult ProcessUpdateMeshJob::SendConnectionReport()
{
    const common::Time now = common::g_piaJobScheduler->m_dispatchTime;
    if (m_updateDeadline <= now)
    {
        g_piaMesh->m_joinMeshJob->m_joinResult = cResultUpdateTimeout;
        SelectStep(m_currentStep, &ProcessUpdateMeshJob::UpdateFailed, cStepNameUpdateFailed);
        return MakeExecuteResult(cExecuteKindContinue);
    }
    const int localIndex = g_piaMesh->GetLocalStationIndex();
    const int hostIndex = g_piaMesh->GetHostStationIndex();
    if (localIndex > cStationIndexMax || hostIndex > cStationIndexMax)
    {
        SelectStep(m_currentStep, &ProcessUpdateMeshJob::UpdateFailed, cStepNameUpdateFailed);
        return MakeExecuteResult(cExecuteKindContinue);
    }

    unsigned int unmeasuredBitmap = 0;
    if (!SendConnectionReport(static_cast<StationIndex>(hostIndex), m_updateVersion, &unmeasuredBitmap))
    {
        if (unmeasuredBitmap != 0 && m_reportDeadline <= common::g_piaJobScheduler->m_dispatchTime)
        {
            for (int index = 0; index < cStationNumMax; ++index)
            {
                if ((unmeasuredBitmap & 1) != 0)
                    g_piaMesh->GetMeshProtocol()->SendDummyMessage(static_cast<StationIndex>(index));
                unmeasuredBitmap >>= 1;
                if (unmeasuredBitmap == 0)
                    break;
            }
            m_reportDeadline = timeAfterNow(cDummyMessageIntervalMsec);
        }
    }
    else
    {
        SelectStep(m_currentStep, &ProcessUpdateMeshJob::StartRelayConnection, cStepNameStartRelayConnection);
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

ProcessUpdateMeshJob::ExecuteResult ProcessUpdateMeshJob::StartRelayConnection()
{
    if (m_updateDeadline <= common::g_piaJobScheduler->m_dispatchTime)
    {
        g_piaMesh->m_joinMeshJob->m_joinResult = cResultUpdateTimeout;
        SelectStep(m_currentStep, &ProcessUpdateMeshJob::UpdateFailed, cStepNameUpdateFailed);
        return MakeExecuteResult(cExecuteKindContinue);
    }
    const int localIndex = g_piaMesh->GetLocalStationIndex();
    if (localIndex > cStationIndexMax)
    {
        SelectStep(m_currentStep, &ProcessUpdateMeshJob::UpdateFailed, cStepNameUpdateFailed);
        return MakeExecuteResult(cExecuteKindContinue);
    }

    transport::RelayRouteManager* relayRouteManager = transport::g_piaTransport->m_relayRouteManager;
    transport::StationManager* stationManager = transport::StationManager::s_pInstance;
    transport::StationConnectionInfoTable* table = transport::g_stationConnectionInfoTable;
    if (relayRouteManager->m_unknown20 != m_updateVersion || relayRouteManager->m_unknown1c <= m_relayRouteVersion)
    {
        const common::Time noticeTime = m_failureNoticeTime;
        const common::Time now = common::g_piaJobScheduler->m_dispatchTime;
        for (unsigned short entry = 0; entry < m_stationCount; ++entry)
        {
            const StationIndex index = m_stationIndices[entry];
            if (index == localIndex)
                continue;
            const unsigned short contextIndex = GetContextIndex(index);
            if (contextIndex >= m_stationNum)
                continue;
            common::CallContext& context = m_connectContexts[contextIndex];
            if (!isFinished(context) || context.m_result == 0)
                continue;
            unsigned char failure = cFailureCancelled + cFailureRelayOffset;
            if (context.m_result == cResultCode643f)
                failure = cFailureCode643f + cFailureRelayOffset;
            else if (context.m_result == cResultCode6442)
                failure = cFailureCode6442 + cFailureRelayOffset;
            else if (context.m_result == cResultCode6443)
                failure = cFailureCode6443 + cFailureRelayOffset;
            else if (context.m_result == cResultCode6444)
                failure = cFailureCode6444 + cFailureRelayOffset;
            else if (context.m_result == cResultCode6445)
                failure = cFailureCode6445 + cFailureRelayOffset;
            else if (context.m_result == cResultCode644a)
                failure = cFailureCode644a + cFailureRelayOffset;
            else if (context.m_result == cResultCode6467)
                failure = cFailureCode6467 + cFailureRelayOffset;
            else if (context.m_result == cResultCode6468)
                failure = cFailureCode6468 + cFailureRelayOffset;
            else if (context.m_result == cResultCode6469)
                failure = cFailureCode6469 + cFailureRelayOffset;
            else if (context.m_result == cResultCode646a)
                failure = cFailureCode646a + cFailureRelayOffset;
            if (noticeTime < now)
            {
                g_piaMesh->GetMeshProtocol()->SendConnectionFailureNotice(
                    g_piaMesh->GetHostStationIndex(), m_stationIndices[entry], static_cast<StationIndex>(localIndex),
                    failure, m_updateVersion);
                m_failureNoticeTime = timeAfterNow(cFailureNoticeIntervalMsec);
            }
        }
        return MakeExecuteResult(cExecuteKindNextStep);
    }

    for (unsigned short entry = 0; entry < m_stationCount; ++entry)
    {
        const StationIndex index = m_stationIndices[entry];
        if (index == localIndex)
            continue;
        const unsigned short contextIndex = GetContextIndex(index);
        if (contextIndex < m_stationNum && isFinished(m_connectContexts[contextIndex]))
            ClearContextIndex(index);
    }

    bool changed = false;
    for (unsigned short entry = 0; entry < m_stationCount; ++entry)
    {
        if (m_stationIndices[entry] == localIndex)
            continue;

        transport::Station* station = stationManager->GetStation(m_stationIndices[entry]);
        StationIndex route;
        Result routeResult = 0;
        bool routeLookup = true;
        if (station == 0)
        {
            station = table->GetStation(m_stationConnectionInfos[entry]);
            if (station != 0)
            {
                station->Trace(cTraceStationLevel);
                if (station->m_field20 != cStationStateReconnect)
                {
                    if (station->m_field20 == cStationStateConnected && station->m_field10 > cStationIndexMax)
                        changed = true;
                    station->SetStationIdAndIndex(m_stationConnectionInfos[entry].m_stationLocation.m_principalId,
                                                  m_stationIndices[entry]);
                }
                else
                {
                    destroyStation(station);
                    changed = true;
                    station = 0;
                }
            }
        }
        if (routeLookup)
            routeResult = relayRouteManager->GetRelayRoute(static_cast<StationIndex>(localIndex),
                                                           m_stationIndices[entry], &route);
        if (routeResult != 0)
        {
            if (station != 0)
                station->Trace(cTraceStationLevel);
            continue;
        }

        StationIndex stationIndex = m_stationIndices[entry];
        if (route == stationIndex || route == localIndex)
            continue;

        if (station != 0 && station->m_field20 == cStationStateConnected)
        {
            if (station->IsConnectionRouteRelay())
                continue;
            if (stationIndex == g_piaMesh->GetHostStationIndex())
            {
                if (g_piaMesh->m_joinMeshJob->IsRunning())
                    g_piaMesh->m_joinMeshJob->Cleanup(Result(cResultJoinFailed));
                SelectStep(m_currentStep, &ProcessUpdateMeshJob::UpdateFailed, cStepNameUpdateFailed);
                return MakeExecuteResult(cExecuteKindContinue);
            }
            destroyStation(station);
            changed = true;
            stationIndex = m_stationIndices[entry];
            station = 0;
        }

        if (localIndex <= static_cast<int>(stationIndex))
            continue;

        if (station == 0)
        {
            if (stationManager->GetValidStationNum() >= transport::g_piaTransport->m_stationNum)
                continue;
            station = stationManager->CreateStation();
            station->Startup(m_stationConnectionInfos[entry].m_stationLocation.m_principalId, m_stationIndices[entry],
                             m_stationConnectionInfos[entry].m_stationLocation.m_stationAddress);
            stationIndex = m_stationIndices[entry];
        }

        const unsigned short existingContext = GetContextIndex(stationIndex);
        if (existingContext < m_stationNum)
        {
            common::CallContext& existing = m_connectContexts[existingContext];
            if (!isFinished(existing))
            {
                if (station->m_field20 == cStationStateConnecting || station->m_field20 == 4
                    || station->m_field20 == cStationStateConnected)
                    continue;
                existing.SignalCancel();
                existing.Reset();
            }
        }

        transport::ConnectStationJob* connectJob = station->m_connectStationJob;
        const unsigned char newContext = GetUsableContextIndex();
        Result startResult = connectJob->StartupRelayConnection(m_connectContexts + newContext, station,
                                                                m_stationConnectionInfos[entry], false,
                                                                station->m_field28->m_timeoutMsec);
        if (startResult == 0)
        {
            table->AddToTable(station, m_stationConnectionInfos[entry]);
            station->m_field20 = cStationStateConnecting;
            SetContextIndex(m_stationIndices[entry], newContext);
            connectJob->Ready(false);
        }
    }
    if (changed)
        transport::g_piaTransport->OutputStreamUpdateEvent();

    m_relayRouteVersion = relayRouteManager->m_unknown1c;
    g_piaMesh->m_joinMeshJob->m_principalId = cPrincipalIdInvalid;
    SelectStep(m_currentStep, &ProcessUpdateMeshJob::CheckConnectionAll, cStepNameCheckConnectionAll);
    return MakeExecuteResult(cExecuteKindContinue);
}

ProcessUpdateMeshJob::ExecuteResult ProcessUpdateMeshJob::CheckConnectionAll()
{
    if (m_updateDeadline <= common::g_piaJobScheduler->m_dispatchTime)
    {
        g_piaMesh->m_joinMeshJob->m_joinResult = cResultUpdateTimeout;
        SelectStep(m_currentStep, &ProcessUpdateMeshJob::UpdateFailed, cStepNameUpdateFailed);
        return MakeExecuteResult(cExecuteKindContinue);
    }
    const int localIndex = g_piaMesh->GetLocalStationIndex();
    if (localIndex > cStationIndexMax)
    {
        SelectStep(m_currentStep, &ProcessUpdateMeshJob::UpdateFailed, cStepNameUpdateFailed);
        return MakeExecuteResult(cExecuteKindContinue);
    }

    const bool noticeDue = m_failureNoticeTime < common::g_piaJobScheduler->m_dispatchTime;
    bool waiting = false;
    bool failed = false;
    bool changed = false;
    for (unsigned short entry = 0; entry < m_stationCount; ++entry)
    {
        const StationIndex index = m_stationIndices[entry];
        if (index == localIndex)
            continue;

        const unsigned char failureCode = m_failureCodes[entry];
        if (failureCode != 0 && failureCode <= cFailureNoneMax)
        {
            UpdateJoinMeshFailureResult(index, failureCode, true);
            waiting = true;
            failed = true;
            continue;
        }

        transport::Station* station = transport::StationManager::s_pInstance->GetStation(index);
        if (station == 0)
        {
            station = transport::g_stationConnectionInfoTable->GetStation(m_stationConnectionInfos[entry]);
            if (station != 0)
            {
                if (station->m_field20 == cStationStateConnected && station->m_field10 > cStationIndexMax)
                    changed = true;
                station->SetStationIdAndIndex(m_stationConnectionInfos[entry].m_stationLocation.m_principalId,
                                              m_stationIndices[entry]);
            }
        }

        const unsigned short contextIndex = GetContextIndex(m_stationIndices[entry]);
        if (contextIndex >= m_stationNum)
        {
            if (station != 0 && station->m_field20 == cStationStateConnected)
                continue;
            waiting = true;
            continue;
        }

        common::CallContext& context = m_connectContexts[contextIndex];
        if (!isFinished(context))
        {
            waiting = true;
            continue;
        }
        if (context.m_result == 0)
        {
            ClearContextIndex(m_stationIndices[entry]);
            if (station == 0)
                waiting = true;
            continue;
        }

        unsigned char failure = cFailureCancelled;
        if (context.m_result == cResultCancelled)
            failure = cFailureCancelled;
        else if (context.m_result == cResultCode643f)
            failure = cFailureCode643f;
        else if (context.m_result == cResultCode6442)
            failure = cFailureCode6442;
        else if (context.m_result == cResultCode6443)
            failure = cFailureCode6443;
        else if (context.m_result == cResultCode6444)
            failure = cFailureCode6444;
        else if (context.m_result == cResultCode6445)
            failure = cFailureCode6445;
        else if (context.m_result == cResultCode644a)
            failure = cFailureCode644a;
        else if (context.m_result == cResultUpdateTimeout)
            failure = cFailureTimeout;
        else if (context.m_result == cResultCode6467)
            failure = cFailureCode6467;
        else if (context.m_result == cResultCode6468)
            failure = cFailureCode6468;
        else if (context.m_result == cResultCode6469)
            failure = cFailureCode6469;
        else if (context.m_result == cResultCode646a)
            failure = cFailureCode646a;
        UpdateJoinMeshFailureResult(m_stationIndices[entry], failure, false);
        waiting = true;
        failed = true;
        if (noticeDue)
        {
            g_piaMesh->GetMeshProtocol()->SendConnectionFailureNotice(
                g_piaMesh->GetHostStationIndex(), m_stationIndices[entry], static_cast<StationIndex>(localIndex),
                failure, m_updateVersion);
            m_failureNoticeTime = timeAfterNow(cFailureNoticeIntervalMsec);
        }
    }

    if (m_leaveMeshOnFailure && failed)
    {
        SelectStep(m_currentStep, &ProcessUpdateMeshJob::UpdateFailed, cStepNameUpdateFailed);
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (waiting)
    {
        if (changed)
            transport::g_piaTransport->OutputStreamUpdateEvent();
        return MakeExecuteResult(cExecuteKindNextStep);
    }

    for (unsigned short entry = 0; entry < m_stationCount; ++entry)
    {
        if (!g_piaMesh->IsValidStationIndex(m_stationIndices[entry]))
            g_piaMesh->FixConnectedId(m_stationIndices[entry]);
    }
    m_partCount = 0;
    m_partStationCount = 0;
    m_partVersion = 0;
    m_started = false;
    m_partReceived[0] = false;
    m_partHostSlot = 0;
    m_updateVersion = 0;
    m_partReceived[1] = false;
    return MakeExecuteResult(cExecuteKindFinished);
}

ProcessUpdateMeshJob::ExecuteResult ProcessUpdateMeshJob::UpdateFailed()
{
    if (!m_leaveMeshOnFailure)
    {
        g_piaMesh->m_disconnectReason = cDisconnectReasonUpdateFailed;
        g_piaMesh->ProcessingAtMeshEnd(static_cast<Mesh::DisconnectReason>(cDisconnectReasonUpdateFailed));
        SelectStep(m_currentStep, &ProcessUpdateMeshJob::CleanupByProcessFailure, cStepNameCleanupByProcessFailure);
    }
    else if (m_callContext.m_state == common::CallContext::cStateInProgress
             || g_piaMesh->m_leaveMeshJob->Startup(&m_callContext) == 0)
    {
        g_piaMesh->m_disconnectReason = cDisconnectReasonLeave;
        g_piaMesh->ProcessingAtMeshEnd(static_cast<Mesh::DisconnectReason>(cDisconnectReasonLeave));
        SelectStep(m_currentStep, &ProcessUpdateMeshJob::CleanupByProcessFailure, cStepNameCleanupByProcessFailure);
    }
    else
    {
        g_piaMesh->m_leaveMeshJob->Ready(false);
        SelectStep(m_currentStep, &ProcessUpdateMeshJob::WaitLeaveMesh, cStepNameWaitLeaveMesh);
    }
    return MakeExecuteResult(cExecuteKindContinue);
}

ProcessUpdateMeshJob::ExecuteResult ProcessUpdateMeshJob::WaitLeaveMesh()
{
    if (m_callContext.m_state != common::CallContext::cStateInProgress)
    {
        SelectStep(m_currentStep, &ProcessUpdateMeshJob::CleanupByProcessFailure, cStepNameCleanupByProcessFailure);
        return MakeExecuteResult(cExecuteKindContinue);
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

void ProcessUpdateMeshJob::CleanupInternalContexts()
{
    for (unsigned short index = 0; index < m_stationNum; ++index)
    {
        if (m_connectContexts[index].m_state == common::CallContext::cStateInProgress)
            m_connectContexts[index].SignalCancel();
        m_connectContexts[index].Reset();
    }
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        m_callContext.SignalCancel();
    m_callContext.Reset();
}

ProcessUpdateMeshJob::ExecuteResult ProcessUpdateMeshJob::CleanupByProcessFailure()
{
    g_piaMesh->CleanupStationsJobs();
    g_piaMesh->CleanupStatus();
    m_updateVersion = 0;
    m_started = false;
    m_partStationCount = 0;
    m_partHostSlot = 0;
    m_partVersion = 0;
    m_partCount = 0;
    m_partReceived[0] = false;
    m_partReceived[1] = false;
    CleanupInternalContexts();
    return MakeExecuteResult(cExecuteKindFinished);
}

void ProcessUpdateMeshJob::UpdateDataTakeover(unsigned short stationCount)
{
    transport::StationManager* stationManager = transport::StationManager::s_pInstance;
    transport::StationConnectionInfoTable* table = transport::g_stationConnectionInfoTable;
    const unsigned int previousCount = m_stationCount;
    m_stationCount = stationCount;
    transport::StationConnectionInfo scratch;
    StationIndex previousIndices[cStationNumMax];
    for (unsigned short index = 0; index < m_stationNum; ++index)
    {
        previousIndices[index] = m_stationIndices[index];
        m_stationIndices[index] = m_partStationIndices[index];
    }

    for (unsigned int previous = 0; previous < previousCount; ++previous)
    {
        const StationIndex previousIndex = previousIndices[previous];
        unsigned short match = 0;
        bool matched = false;
        for (; match < m_stationCount; ++match)
        {
            if (m_partStationIndices[match] == previousIndex)
            {
                matched = true;
                break;
            }
        }

        if (matched && m_stationConnectionInfos[previous] == m_partConnectionInfos[match])
        {
            transport::Station* station = stationManager->GetStation(previousIndex);
            if (((station == 0 && table->GetStation(m_stationConnectionInfos[previous]) == 0)
                 || (station != 0 && station->m_field20 == cStationStateReconnect)))
            {
                const unsigned short contextIndex = GetContextIndex(previousIndex);
                if (contextIndex < m_stationNum && isFinished(m_connectContexts[contextIndex]))
                    ClearContextIndex(previousIndex);
            }
            continue;
        }

        const bool valid = g_piaMesh->IsValidStationIndex(previousIndex);
        const unsigned short contextIndex = GetContextIndex(previousIndex);
        if (valid)
        {
            if (contextIndex < m_stationNum)
                ClearContextIndex(previousIndex);
            continue;
        }

        transport::Station* station = stationManager->GetStation(previousIndex);
        if (station == 0)
            station = table->GetStation(m_stationConnectionInfos[previous]);
        if (station != 0)
        {
            if (contextIndex < m_stationNum)
            {
                m_connectContexts[contextIndex].Cancel();
                ClearContextIndex(previousIndex);
            }
            destroyStation(station);
            transport::g_piaTransport->OutputStreamUpdateEvent();
            continue;
        }
        if (contextIndex >= m_stationNum)
            continue;
        ClearContextIndex(previousIndex);
        if (g_piaMesh->m_unknown2c != 0)
            g_piaMesh->m_unknown2c->Execute(&m_stationConnectionInfos[previous]);
    }

    for (unsigned short index = 0; index < m_stationCount; ++index)
        m_stationConnectionInfos[index] = m_partConnectionInfos[index];
}

int ProcessUpdateMeshJob::UpdateStationDataList(const unsigned char* data, unsigned int size, bool relay)
{
    const unsigned int infoSize = m_stationConnectionInfos[0].GetSerializedSize();
    const unsigned char stationCount = data[1];
    const unsigned char hostSlot = data[2];
    const unsigned int stride = infoSize + (~infoSize & cEntryAlignMask) + 1;
    const unsigned int version = common::deserializeU32(data + 4);
    if (m_stationNum < stationCount || (version <= m_updateVersion && !relay))
        return cUpdateMeshResultInvalid;

    bool complete = false;
    if (data[8] == cMessageFullList)
    {
        if (m_partReceived[0] || m_partReceived[1])
        {
            if (version < m_partVersion && !relay)
                return cUpdateMeshResultOld;
            m_partCount = 0;
            m_partStationCount = 0;
            m_partVersion = 0;
            m_partHostSlot = 0;
            m_partReceived[1] = false;
            m_partReceived[0] = false;
        }
        readStationEntries(m_partConnectionInfos, m_partStationIndices, data, infoSize, stride, 0, stationCount);
        complete = true;
    }
    else if (data[8] == cMessagePartialList)
    {
        const unsigned char partIndex = data[9];
        bool partAlreadyReceived;
        if (!m_partReceived[0] && !m_partReceived[1])
        {
            m_partStationCount = stationCount;
            m_partVersion = version;
            m_partCount = cMessagePartialList;
            m_partHostSlot = hostSlot;
            partAlreadyReceived = false;
        }
        else
        {
            if (m_partStationCount == stationCount && m_partHostSlot == hostSlot)
            {
                if (version < m_partVersion)
                {
                    if (!relay)
                        return cUpdateMeshResultOld;
                    partAlreadyReceived = m_partReceived[partIndex];
                }
                else if (version <= m_partVersion)
                {
                    if (m_partCount != cMessagePartialList && !relay)
                        return cUpdateMeshResultOld;
                    partAlreadyReceived = m_partReceived[partIndex];
                }
                else
                {
                    m_partStationCount = stationCount;
                    m_partHostSlot = hostSlot;
                    m_partCount = cMessagePartialList;
                    m_partVersion = version;
                    m_partReceived[1] = false;
                    m_partReceived[0] = false;
                    partAlreadyReceived = m_partReceived[partIndex];
                }
            }
            else
            {
                if (m_partVersion < version)
                {
                    m_partStationCount = stationCount;
                    m_partHostSlot = hostSlot;
                    m_partCount = cMessagePartialList;
                    m_partVersion = version;
                    m_partReceived[1] = false;
                    m_partReceived[0] = false;
                    partAlreadyReceived = m_partReceived[partIndex];
                }
                else
                {
                    if (!relay)
                        return cUpdateMeshResultOld;
                    partAlreadyReceived = m_partReceived[partIndex];
                }
            }
            if (partAlreadyReceived)
                return cUpdateMeshResultPartial;
        }

        readStationEntries(m_partConnectionInfos, m_partStationIndices, data, infoSize, stride, data[11], data[10]);
        m_partReceived[partIndex] = true;
        if (!m_partReceived[0] || !m_partReceived[1])
            return cUpdateMeshResultPartial;
        m_partVersion = 0;
        m_partHostSlot = 0;
        m_partCount = 0;
        m_partStationCount = 0;
        m_partReceived[1] = false;
        m_partReceived[0] = false;
        complete = true;
    }
    else
    {
        return cUpdateMeshResultInvalid;
    }

    if (!complete || !m_updateMeshSucceeded)
        return 3;

    UpdateDataTakeover(stationCount);
    m_updateMeshSucceeded = UpdateMesh();
    CalcTimeLimit(false);
    m_relayRouteVersion = 0;
    m_updateVersion = version;
    m_reportCount = 0;
    for (unsigned short index = 0; index < m_stationNum; ++index)
        m_failureCodes[index] = 0;
    m_failureNoticeTime = common::g_piaJobScheduler->m_dispatchTime;
    m_relayRequested = false;
    if (!m_updateMeshSucceeded)
    {
        SelectStep(m_currentStep, &ProcessUpdateMeshJob::UpdateFailed, cStepNameUpdateFailed);
    }
    else if (g_piaMesh->GetNetworkTopologyType() == cNetworkTopologyRelay)
    {
        SelectStep(m_currentStep, &ProcessUpdateMeshJob::WaitDirectConnection, cStepNameWaitDirectConnection);
    }
    else
    {
        SelectStep(m_currentStep, &ProcessUpdateMeshJob::CheckConnectionAll, cStepNameCheckConnectionAll);
    }
    return cUpdateMeshStartupResultStarted;
}

void ProcessUpdateMeshJob::Cleanup()
{
    m_stationCount = 0;
    m_reportCount = 0;
    m_relayRouteVersion = 0;
    m_updateVersion = 0;
    m_started = false;
    m_partReceived[0] = false;
    m_partReceived[1] = false;
    m_partStationCount = 0;
    m_partHostSlot = 0;
    m_partVersion = 0;
    m_partCount = 0;
    CleanupInternalContexts();
    memset(m_contextIndex, cContextIndexNone, sizeof(m_contextIndex));
    m_updateDeadline = common::Time();
    m_leaveMeshOnFailure = false;
}

StationIndex ProcessUpdateMeshJob::GetStationIndexByPrincipalID(unsigned long long principalId) const
{
    StationIndex stationIndex = static_cast<StationIndex>(cStationIndexNone);
    for (unsigned short index = 0; index < m_stationCount; ++index)
    {
        const transport::StationConnectionInfo& info = m_stationConnectionInfos[index];
        if (info.m_stationLocation.m_principalId == principalId)
        {
            stationIndex = m_stationIndices[index];
            break;
        }
    }
    return stationIndex;
}

bool ProcessUpdateMeshJob::CheckEdmByStationIndex(StationIndex stationIndex) const
{
    for (unsigned short index = 0; index < m_stationCount; ++index)
    {
        if (m_stationIndices[index] == stationIndex)
        {
            const transport::StationConnectionInfo& info = m_stationConnectionInfos[index];
            if (info.m_stationLocation.m_field23 == 2)
                return true;
            return false;
        }
    }
    return false;
}

void ProcessUpdateMeshJob::SetConnectionFailureNotice(StationIndex stationIndex, unsigned char failure)
{
    for (unsigned short index = 0; index < m_stationNum; ++index)
    {
        if (m_stationIndices[index] == stationIndex)
        {
            m_failureCodes[index] = failure;
            return;
        }
    }
}

void ProcessUpdateMeshJob::InitializeMonitoringDataVar()
{
    m_relayAttemptCount = 0;
}

void ProcessUpdateMeshJob::SetMonitoringData()
{
    common::g_sessionStateMonitoringContent.m_metric184 = static_cast<unsigned short>(m_relayAttemptCount);
}

void ProcessUpdateMeshJob::ClearStationIndex(StationIndex stationIndex)
{
    if (stationIndex >= cStationNumMax)
        return;
    if (m_contextIndex[stationIndex & 0xff] >= m_stationNum)
        return;
    ClearContextIndex(stationIndex);
}

}
}
}
