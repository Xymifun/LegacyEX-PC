#include "nn/pia/session/JoinMeshJob.hpp"

#include <new>

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Packet.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/session/KickoutManageJob.hpp"
#include "nn/pia/session/LeaveMeshJob.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/MeshEventListener.hpp"
#include "nn/pia/session/MeshProtocol.hpp"
#include "nn/pia/session/ProcessUpdateMeshJob.hpp"
#include "nn/pia/transport/ConnectStationJob.hpp"
#include "nn/pia/transport/DisconnectStationJob.hpp"
#include "nn/pia/transport/IdentificationInfoTable.hpp"
#include "nn/pia/transport/ProcessConnectionRequestJob.hpp"
#include "nn/pia/transport/RelayRouteManager.hpp"
#include "nn/pia/transport/ResendingMessageManager.hpp"
#include "nn/pia/transport/Station.hpp"
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
enum JoinResult
{
    cResultCancelled = 0x6c05,
    cResultSetupLocalPlayerInfoFailed = 0x6c65,
    cResultJoinFailed = 0x6439,
    cResultJoinResponse0 = 0x6c3d,
    cResultJoinResponse1 = 0x6838,
    cResultJoinResponseUnknown = 0x10c3e,
    cResultConnectionRefused = 0xc24,
    cResultConnectionFailed10c25 = 0x10c25,
    cResultConnectionFailed643f = 0x643f,
    cResultConnectionFailed6442 = 0x6442,
    cResultConnectionFailed6443 = 0x6443,
    cResultConnectionFailed6444 = 0x6444,
    cResultConnectionFailed6445 = 0x6445,
    cResultConnectionFailed6467 = 0x6467,
    cResultConnectionFailed6468 = 0x6468,
    cResultConnectionFailed6469 = 0x6469,
    cResultConnectionFailed646a = 0x646a,
    cResultConnectionFailed644a = 0x644a
};

inline void SelectStep(common::StepSequenceJob::Step& slot, JoinMeshJob::ExecuteResult (JoinMeshJob::*function)(),
                       const char* name)
{
    const common::StepSequenceJob::Step step(
        static_cast<JoinMeshJob::ExecuteResult (common::StepSequenceJob::*)()>(function), name);
    slot = step;
}

enum MeshEventKind
{
    cMeshEvent14 = 0x14,
    cMeshEvent16 = 0x16,
    cMeshEvent17 = 0x17
};

enum JoinResponseLayout
{
    cResponseCount = 1,
    cResponseHostSlot = 2,
    cResponseLocalSlot = 3,
    cResponsePartCount = 4,
    cResponsePartIndex = 5,
    cResponseEntryCount = 6,
    cResponseEntryStart = 7,
    cResponseEventIdHigh = 8,
    cResponseEventIdLow = 9,
    cResponseEventIdByte = 10,
    cResponseSerialNumber = 12,
    cResponseFirstPartEntries = 16,
    cResponseNextPartEntries = 8,
    cResponseSlotNone = 0xff
};

enum
{
    cStationIndexInvalid = 0xfd,
    cLocalAndHostStationCount = 2,
    cConnectionIdModulo = 0xfe,
    cConnectionIdBase = 2,
    cDisconnectTimeoutMsec = 5000,
    cJoinRequestTimeoutMsec = 10000,
    cInvalidPrincipalId = 0xffffffff,
    cConnectedStationState = 5,
    cConnectingStationState = 2,
    cTraceStationLevel = 0x4000000,
    cTraceIdentificationLevel = 0x8000,
    cMonitoringByteUnset = 0xff,
    cMonitoredTargetMax = 15,
    cMonitoredRelayMax = 8
};
}

JoinMeshJob::JoinMeshJob()
    : common::StepSequenceJob(),
      m_callerContext(0),
      m_requestAckId(0),
      m_waitingJoinResponse(false),
      m_responseResult(3),
      m_joinState(0),
      m_joinResult(0),
      m_startTime(),
      m_disconnectDeadline(),
      m_callContext()
{
    for (int index = 0; index < cReceivedFlagNum; ++index)
        m_received[index] = false;
    m_lastUpdateCount = 0;
    void* contextStorage = pead::allocateFromSeadHeap(sizeof(common::CallContext), common::HeapManager::GetHeap());
    m_internalContext = new (contextStorage) common::CallContext();
    m_stationCapacity = g_piaMesh->m_unknown54;
    unsigned int count = m_stationCapacity;
    transport::StationConnectionInfo* infos = static_cast<transport::StationConnectionInfo*>(
        pead::allocateFromSeadHeap(count * sizeof(transport::StationConnectionInfo), common::HeapManager::GetHeap()));
    if (infos != 0)
    {
        transport::StationConnectionInfo* info = infos;
        for (; count != 0; --count)
        {
            new (info) transport::StationConnectionInfo();
            ++info;
        }
    }
    m_stationInfos = infos;
    count = m_stationCapacity;
    unsigned int* indices = static_cast<unsigned int*>(
        pead::allocateFromSeadHeap(count * sizeof(unsigned int), common::HeapManager::GetHeap()));
    if (indices != 0)
    {
        unsigned int* index = indices;
        for (; count != 0; --count)
        {
            new (index) unsigned int();
            ++index;
        }
    }
    m_stationIndices = indices;
    count = m_stationCapacity;
    unsigned char* connected = static_cast<unsigned char*>(
        pead::allocateFromSeadHeap(count, common::HeapManager::GetHeap()));
    if (connected != 0)
    {
        unsigned char* flag = connected;
        for (; count != 0; --count)
        {
            new (flag) unsigned char();
            ++flag;
        }
    }
    m_stationConnected = connected;
    for (unsigned short index = 0; index < m_stationCapacity; ++index)
        m_stationConnected[index] = 0;
}

JoinMeshJob::~JoinMeshJob()
{
    CleanupInternalContexts();
    if (m_internalContext != 0)
    {
        m_internalContext->~CallContext();
        pead::freeToSeadHeap(m_internalContext);
    }
    if (m_stationInfos != 0)
    {
        unsigned int count = pead::getArraySize(m_stationInfos) / sizeof(transport::StationConnectionInfo);
        transport::StationConnectionInfo* info = m_stationInfos;
        for (; count != 0; --count)
        {
            info->~StationConnectionInfo();
            ++info;
        }
        pead::freeToSeadHeap(m_stationInfos);
    }
    if (m_stationIndices != 0)
    {
        pead::getArraySize(m_stationIndices);
        pead::freeToSeadHeap(m_stationIndices);
    }
    if (m_stationConnected != 0)
    {
        pead::getArraySize(m_stationConnected);
        pead::freeToSeadHeap(m_stationConnected);
    }
}

bool JoinMeshJob::Startup(const transport::StationConnectionInfo& info, common::CallContext* callerContext)
{
    if (callerContext != 0)
    {
        m_callerContext = callerContext;
        callerContext->InitiateCall();
    }
    if (StartupImpl() == 0)
        return false;
    *m_stationInfos = info;
    m_joinResult = 0;
    m_waitingJoinResponse = false;
    for (int index = 0; index < cReceivedFlagNum; ++index)
        m_received[index] = false;
    m_requestAckId = 0;
    m_lastUpdateCount = 0;
    for (unsigned int index = 0; index < m_stationCapacity; ++index)
        m_stationConnected[index] = 0;
    g_piaMesh->CleanupMonitoringBeginData();
    SetHostInfoToMonitoringData(info);
    m_principalId = info.m_stationLocation.m_principalId;
    g_piaMesh->GetProcessUpdateMeshJobPtr()->m_leaveMeshOnFailure = true;
    m_startTime = common::g_piaJobScheduler->m_dispatchTime;
    return true;
}

JoinMeshJob::ExecuteResult JoinMeshJob::SetupLocalPlayerInfo()
{
    if (g_piaMesh->SetupLocalPlayerInfo() != 0)
        SelectStep(m_currentStep, &JoinMeshJob::SetupLocalPlayerInfoFailure,
                   "JoinMeshJob::SetupLocalPlayerInfoFailure");
    else
        SetNextStepAtSetupLocalPlayerInfoImpl();
    return MakeExecuteResult(cExecuteKindEntryForeground);
}

JoinMeshJob::ExecuteResult JoinMeshJob::SetupLocalStation()
{
    Result result = g_piaMesh->SetupProtocols();
    if (result != 0)
    {
        if (m_callerContext->m_state - common::CallContext::cStateSuccess > 2u)
            m_callerContext->SignalFailure(cResultJoinFailed);
        common::g_sessionBeginMonitoringContent.m_joinResult = cResultJoinFailed;
        common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
        g_piaMesh->ProcessingAtMeshBegin();
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    transport::Station* station = transport::StationManager::s_pInstance->CreateLocalStation();
    if (!station->Startup())
    {
        if (m_callerContext->m_state - common::CallContext::cStateSuccess > 2u)
            m_callerContext->SignalFailure(cResultJoinFailed);
        common::g_sessionBeginMonitoringContent.m_joinResult = cResultJoinFailed;
        common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
        g_piaMesh->ProcessingAtMeshBegin();
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    g_piaMesh->ResetStationIndexUsage();
    transport::StationConnectionInfoTable* table = transport::g_stationConnectionInfoTable;
    station->m_stationAddress = table->m_localStationConnectionInfo.m_stationLocation.m_stationAddress;
    result = table->AddToTable(station, table->m_localStationConnectionInfo);
    if (result != 0)
    {
        if (m_callerContext->m_state - common::CallContext::cStateSuccess > 2u)
            m_callerContext->SignalFailure(cResultJoinFailed);
        common::g_sessionBeginMonitoringContent.m_joinResult = cResultJoinFailed;
        common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
        g_piaMesh->ProcessingAtMeshBegin();
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    transport::IdentificationInfoTable* identificationInfoTable = transport::IdentificationInfoTable::GetInstance();
    transport::IdentificationInfo info;
    result = identificationInfoTable->GetLocalIdentificationInfo(&info);
    if (result != 0)
    {
        if (m_callerContext->m_state - common::CallContext::cStateSuccess > 2u)
            m_callerContext->SignalFailure(cResultJoinFailed);
        common::g_sessionBeginMonitoringContent.m_joinResult = cResultJoinFailed;
        common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
        g_piaMesh->ProcessingAtMeshBegin();
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (g_piaMesh->m_getSessionIdCallback != 0)
        info.m_field10c = g_piaMesh->m_getSessionIdCallback();
    else
        info.m_field10c = 0;
    info.m_playerCount = static_cast<unsigned char>(g_piaMesh->GetLocalPlayerCount());
    info.m_participantCount = g_piaMesh->IsPlayersCountedAsParticipants() ? info.m_playerCount : 1;
    transport::IdentificationInfoTable::GetInstance()->SetLocalIdentificationInfo(&info);
    result = transport::IdentificationInfoTable::GetInstance()->AddToTable(station, &info, 0);
    if (result != 0)
    {
        transport::IdentificationInfoTable::GetInstance()->Trace(cTraceIdentificationLevel);
        if (m_callerContext->m_state - common::CallContext::cStateSuccess > 2u)
            m_callerContext->SignalFailure(cResultJoinFailed);
        common::g_sessionBeginMonitoringContent.m_joinResult = cResultJoinFailed;
        common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
        g_piaMesh->ProcessingAtMeshBegin();
        m_callerContext = 0;
        return MakeExecuteResult(cExecuteKindFinished);
    }
    SelectStep(m_currentStep, &JoinMeshJob::StartConnectingToHost, "JoinMeshJob::StartConnectingToHost");
    return MakeExecuteResult(cExecuteKindContinue);
}

void JoinMeshJob::CleanupStations()
{
    transport::Station* localStation = transport::StationManager::s_pInstance->m_localStation;
    if (localStation != 0)
    {
        transport::Station** station = transport::StationManager::s_pInstance->m_stations.data;
        if (station != station + transport::StationManager::s_pInstance->m_stations.size)
        {
            do
            {
                if (*station != localStation)
                {
                    if (g_piaMesh->IsValidStationIndex((*station)->GetStationIndex()))
                        g_piaMesh->UnfixDisconnectedId((*station)->GetStationIndex());
                    (*station)->m_connectStationJob->Cleanup();
                    (*station)->m_processConnectionRequestJob->Cleanup();
                }
                ++station;
            } while (station
                     != transport::StationManager::s_pInstance->m_stations.data
                            + transport::StationManager::s_pInstance->m_stations.size);
        }
    }
}

void JoinMeshJob::CalcProcessingTimeOfJoin() const
{
    const common::TimeSpan elapsed(common::g_piaJobScheduler->m_dispatchTime.m_tick - m_startTime.m_tick);
    const int elapsedMsec = static_cast<int>(elapsed.m_tick / common::TimeSpan::GetTicksPerMSec().m_tick);
    if (elapsedMsec >= 0)
    {
        common::g_sessionBeginMonitoringContent.m_joinProcessingTime = elapsedMsec;
        common::g_sessionBeginMonitoringContent.m_joinMeshState = static_cast<unsigned char>(m_joinState);
    }
}

void JoinMeshJob::CheckRelayConnectionForMonitoring() const
{
    const transport::RelayRouteManager* relayRouteManager = transport::g_piaTransport->m_relayRouteManager;
    if (relayRouteManager == 0)
        return;
    const transport::StationManager* stationManager = transport::StationManager::s_pInstance;
    if (stationManager == 0)
        return;
    const transport::StationConnectionInfoTable* connectionInfoTable = transport::g_stationConnectionInfoTable;
    if (connectionInfoTable == 0)
        return;

    const Mesh* mesh = g_piaMesh;
    const StationIndex localStationIndex = mesh->GetLocalStationIndex();
    unsigned char targetCount = 0;
    unsigned char relayCount = 0;
    unsigned long long targetPrincipalIds[cMonitoredTargetMax] = {0};
    unsigned long long relayPrincipalIds[cMonitoredRelayMax] = {0};
    StationIndex relayStations[transport::StationManager::cStationNumMax];
    transport::Station* const* station = stationManager->m_stations.data;
    if (station != stationManager->m_stations.data + stationManager->m_stations.size)
    {
        do
        {
            if ((*station)->m_field20 == cConnectedStationState)
            {
                StationIndex relay;
                if (mesh->IsValidStationIndex((*station)->GetStationIndex()))
                {
                    const Result result = relayRouteManager->GetRelayRoute(localStationIndex,
                                                                           (*station)->GetStationIndex(), &relay);
                    if (result == 0 && relay != cStationIndexInvalid
                        && static_cast<unsigned int>(relay) != (*station)->GetStationIndex())
                    {
                        if (targetCount >= cMonitoredTargetMax)
                            return;
                        if (relayCount >= cMonitoredRelayMax)
                            return;
                        targetPrincipalIds[targetCount++] = connectionInfoTable->GetPrincipalIdByStation(*station);
                        for (int known = 0; known <= relayCount; ++known)
                        {
                            if (known == relayCount)
                            {
                                relayStations[known] = relay;
                                ++relayCount;
                                break;
                            }
                            if (relayStations[known] == relay)
                                break;
                        }
                    }
                }
            }
            ++station;
        } while (station != stationManager->m_stations.data + stationManager->m_stations.size);
        if (targetCount >= cMonitoredTargetMax)
            return;
        if (relayCount >= cMonitoredRelayMax)
            return;
    }

    for (int index = 0; index < relayCount; ++index)
        relayPrincipalIds[index] = connectionInfoTable->GetPrincipalIdByStation(
            stationManager->GetStation(relayStations[index]));

    common::g_sessionBeginMonitoringContent.m_metric50 = targetCount;
    for (int index = 0; index <= relayCount; ++index)
    {
        unsigned int hash = 0xffffffff;
        if (targetPrincipalIds[index] != 0)
            hash = common::hashWithMd5(targetPrincipalIds[index]);
        common::g_sessionBeginMonitoringContent.m_metric51[index] = hash;
    }

    common::g_sessionBeginMonitoringContent.m_metric52 = relayCount;
    for (int index = 0; index < relayCount; ++index)
    {
        unsigned int hash = 0xffffffff;
        if (relayPrincipalIds[index] != 0)
            hash = common::hashWithMd5(relayPrincipalIds[index]);
        common::g_sessionBeginMonitoringContent.m_metric53[index] = hash;
    }
}

unsigned int JoinMeshJob::CheckContextCallCancelled()
{
    if (m_callerContext != 0 && m_callerContext->m_cancelRequested)
    {
        CleanupInternalContexts();
        CleanupStations();
        ProcessUpdateMeshJob* processUpdateMeshJob = g_piaMesh->m_processUpdateMeshJob;
        if (processUpdateMeshJob->m_started)
        {
            processUpdateMeshJob->Cleanup();
            processUpdateMeshJob->Reset(false);
        }
        if (m_callerContext->m_state == common::CallContext::cStateInProgress)
            m_callerContext->SignalCancel();
        m_callerContext = 0;
        CalcProcessingTimeOfJoin();
        CheckRelayConnectionForMonitoring();
        common::g_sessionBeginMonitoringContent.m_joinResult = cResultCancelled;
        common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
        common::g_sessionBeginMonitoringContent.m_joinMeshState = static_cast<unsigned char>(m_joinState);
        g_piaMesh->ProcessingAtMeshBegin();
        return 1;
    }
    return 0;
}

JoinMeshJob::ExecuteResult JoinMeshJob::StartConnectingToHost()
{
    if (CheckContextCallCancelled() != 0)
        return MakeExecuteResult(cExecuteKindFinished);
    m_joinState = 1;
    transport::Station* station = transport::StationManager::s_pInstance->CreateStation();
    if (station->Startup())
    {
        station->SetStationIdAndIndex(m_stationInfos->m_stationLocation.m_principalId,
                                      g_piaMesh->GetHostStationIndex());
        station->m_stationAddress = m_stationInfos->m_stationLocation.m_stationAddress;
        transport::g_stationConnectionInfoTable->AddToTable(station, *m_stationInfos);
        common::Time now;
        now.SetNow();
        station->m_field54 = static_cast<unsigned char>(now.m_tick % cConnectionIdModulo + cConnectionIdBase);
    }
    transport::ConnectStationJob* connectJob = station->m_connectStationJob;
    if (connectJob->GetState() != cStateNotEntried && connectJob->GetState() != cStateCompleted)
    {
        connectJob->Cleanup();
        connectJob->Reset(true);
    }
    if (connectJob->Startup(m_internalContext, station, *m_stationInfos, false, station->m_field28->m_timeoutMsec) != 0)
    {
        CalcProcessingTimeOfJoin();
        if (m_callerContext != 0)
        {
            if (m_callerContext->m_state - common::CallContext::cStateSuccess > 2u)
                m_callerContext->SignalFailure(cResultJoinFailed);
            common::g_sessionBeginMonitoringContent.m_joinResult = cResultJoinFailed;
            common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
            g_piaMesh->ProcessingAtMeshBegin();
            m_callerContext = 0;
        }
        transport::g_stationConnectionInfoTable->EraseFromTable(station);
        station->Cleanup();
        station->CleanupJobs();
        transport::StationManager::s_pInstance->DestroyStation(station);
        CleanupInternalContexts();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    station->m_field20 = cConnectingStationState;
    connectJob->Ready(false);
    SelectStep(m_currentStep, &JoinMeshJob::WaitUntilConnectToHost, "JoinMeshJob::WaitUntilConnectToHost");
    return MakeExecuteResult(cExecuteKindNextStep);
}

unsigned int JoinMeshJob::CheckTransportConnectionStatus()
{
    if (transport::g_piaTransport->m_connectionError == 0)
        return 0;
    CalcProcessingTimeOfJoin();
    CheckRelayConnectionForMonitoring();
    if (m_callerContext != 0)
    {
        if (m_callerContext->m_state - common::CallContext::cStateSuccess > 2u)
            m_callerContext->SignalFailure(cResultJoinFailed);
        common::g_sessionBeginMonitoringContent.m_joinResult = cResultJoinFailed;
        common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
        g_piaMesh->ProcessingAtMeshBegin();
        m_callerContext = 0;
    }
    CleanupInternalContexts();
    return 1;
}

JoinMeshJob::ExecuteResult JoinMeshJob::WaitUntilConnectToHost()
{
    if (CheckContextCallCancelled() != 0)
        return MakeExecuteResult(cExecuteKindFinished);
    common::CallContext* context = m_internalContext;
    if (context->m_state - common::CallContext::cStateSuccess > 2u)
    {
        if (CheckTransportConnectionStatus() != 0)
            return MakeExecuteResult(cExecuteKindFinished);
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    if (context->m_result != 0)
    {
        Result result;
        if (context->m_result == cResultConnectionRefused)
            result = cResultJoinResponse0;
        else if (context->m_result == cResultConnectionFailed10c25)
            result = cResultConnectionFailed10c25;
        else if (context->m_result == cResultConnectionFailed643f)
            result = cResultConnectionFailed643f;
        else if (context->m_result == cResultConnectionFailed6442)
            result = cResultConnectionFailed6442;
        else if (context->m_result == cResultConnectionFailed6443)
            result = cResultConnectionFailed6443;
        else if (context->m_result == cResultConnectionFailed6444)
            result = cResultConnectionFailed6444;
        else if (context->m_result == cResultConnectionFailed6445)
            result = cResultConnectionFailed6445;
        else if (context->m_result == cResultConnectionFailed6467)
            result = cResultConnectionFailed6467;
        else if (context->m_result == cResultConnectionFailed6468)
            result = cResultConnectionFailed6468;
        else if (context->m_result == cResultConnectionFailed6469)
            result = cResultConnectionFailed6469;
        else if (context->m_result == cResultConnectionFailed646a)
            result = cResultConnectionFailed646a;
        else if (context->m_result == cResultConnectionFailed644a)
            result = cResultConnectionFailed644a;
        else
            result = cResultJoinFailed;
        if (result == cResultConnectionFailed643f || result == cResultConnectionFailed6443
            || result == cResultConnectionFailed6444 || result == cResultConnectionFailed6445
            || result == cResultConnectionFailed6442 || result == cResultConnectionFailed6468
            || result == cResultConnectionFailed6469 || result == cResultConnectionFailed646a
            || result == cResultConnectionFailed6467 || result == cResultConnectionFailed644a)
        {
            if (g_piaMesh->m_meshEventListener != 0)
            {
                MeshEventListener::MeshEvent event;
                event.type = cMeshEvent16;
                event.stationIndex = static_cast<StationIndex>(cStationIndexInvalid);
                event.stationId = 0;
                g_piaMesh->m_meshEventListener->MeshEventListener::NoticeEvent(event);
            }
        }
        CalcProcessingTimeOfJoin();
        if (m_callerContext != 0)
        {
            if (m_callerContext->m_state - common::CallContext::cStateSuccess > 2u)
                m_callerContext->SignalFailure(result);
            common::g_sessionBeginMonitoringContent.m_joinResult = result;
            common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
            g_piaMesh->ProcessingAtMeshBegin();
            m_callerContext = 0;
        }
        CleanupInternalContexts();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    transport::Station* station = transport::StationManager::s_pInstance->GetStation(g_piaMesh->GetHostStationIndex());
    if (station == 0)
    {
        CalcProcessingTimeOfJoin();
        if (m_callerContext != 0)
        {
            if (m_callerContext->m_state - common::CallContext::cStateSuccess > 2u)
                m_callerContext->SignalFailure(cResultJoinFailed);
            common::g_sessionBeginMonitoringContent.m_joinResult = cResultJoinFailed;
            common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
            g_piaMesh->ProcessingAtMeshBegin();
            m_callerContext = 0;
        }
        CleanupInternalContexts();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    if (!(transport::g_stationConnectionInfoTable->m_hostStationAddress == station->m_stationAddress))
    {
        station->m_stationAddress.Trace(cTraceStationLevel);
        transport::g_stationConnectionInfoTable->m_hostStationAddress.Trace(cTraceStationLevel);
    }
    transport::StationConnectionInfo hostInfo;
    transport::g_stationConnectionInfoTable->GetStationConnectionInfo(station, &hostInfo);
    transport::g_stationConnectionInfoTable->m_hostStationConnectionInfo = hostInfo;
    transport::g_stationConnectionInfoTable->m_hostStationAddress = station->m_stationAddress;
    SelectStep(m_currentStep, &JoinMeshJob::SendJoinRequest, "JoinMeshJob::SendJoinRequest");
    return MakeExecuteResult(cExecuteKindContinue);
}

JoinMeshJob::ExecuteResult JoinMeshJob::SendJoinRequest()
{
    if (CheckContextCallCancelled() != 0)
        return MakeExecuteResult(cExecuteKindFinished);
    if (transport::StationManager::s_pInstance->m_localStation == 0)
    {
        CalcProcessingTimeOfJoin();
        if (m_callerContext != 0)
        {
            if (m_callerContext->m_state - common::CallContext::cStateSuccess > 2u)
                m_callerContext->SignalFailure(cResultJoinFailed);
            common::g_sessionBeginMonitoringContent.m_joinResult = cResultJoinFailed;
            common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
            g_piaMesh->ProcessingAtMeshBegin();
            m_callerContext = 0;
        }
        CleanupInternalContexts();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    unsigned char data[common::Packet::cPayloadSizeMax] = {0};
    g_piaMesh->GetMeshProtocol()->MakeJoinRequestData(data);
    common::StationAddress hostAddress;
    Result result = transport::StationManager::s_pInstance->GetStationAddress(&hostAddress,
                                                                              g_piaMesh->GetHostStationIndex());
    if (result == 0)
        result = transport::g_piaResendingMessageManager->SetSendMessageByStationAddress(
            &m_requestAckId, data, g_piaMesh->GetMeshProtocol()->GetJoinRequestDataSize(), hostAddress,
            g_piaMesh->GetMeshProtocol()->GetProtocolId(),
            (common::g_piaJobScheduler->m_dispatchTime + common::TimeSpan::FromMSec(cJoinRequestTimeoutMsec)).m_tick);
    if (result != 0)
    {
        CalcProcessingTimeOfJoin();
        if (m_callerContext != 0)
        {
            if (m_callerContext->m_state - common::CallContext::cStateSuccess > 2u)
                m_callerContext->SignalFailure(cResultJoinFailed);
            common::g_sessionBeginMonitoringContent.m_joinResult = cResultJoinFailed;
            common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
            g_piaMesh->ProcessingAtMeshBegin();
            m_callerContext = 0;
        }
        CleanupInternalContexts();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    g_piaMesh->GetMeshProtocol()->SetWaitingJoinResponseJob(this);
    m_waitingJoinResponse = true;
    for (int index = 0; index < cReceivedFlagNum; ++index)
        m_received[index] = true;
    SelectStep(m_currentStep, &JoinMeshJob::WaitRequestAck, "JoinMeshJob::WaitRequestAck");
    return MakeExecuteResult(cExecuteKindNextStep);
}

unsigned int JoinMeshJob::CheckConnectionStateWithHostStation()
{
    transport::Station* station = transport::StationManager::s_pInstance->GetStation(g_piaMesh->GetHostStationIndex());
    if (station != 0 && station->m_field20 == cConnectedStationState)
        return 0;
    CalcProcessingTimeOfJoin();
    CheckRelayConnectionForMonitoring();
    if (m_callerContext != 0)
    {
        if (m_callerContext->m_state - common::CallContext::cStateSuccess > 2u)
            m_callerContext->SignalFailure(cResultJoinFailed);
        common::g_sessionBeginMonitoringContent.m_joinResult = cResultJoinFailed;
        common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
        g_piaMesh->ProcessingAtMeshBegin();
        m_callerContext = 0;
    }
    CleanupInternalContexts();
    return 1;
}

JoinMeshJob::ExecuteResult JoinMeshJob::WaitRequestAck()
{
    if (transport::g_piaResendingMessageManager->CheckNowResending(m_requestAckId))
    {
        if (CheckContextCallCancelled() != 0)
        {
            transport::g_piaResendingMessageManager->StopResending(m_requestAckId);
            m_requestAckId = 0;
            m_waitingJoinResponse = false;
            for (int index = 0; index < cReceivedFlagNum; ++index)
                m_received[index] = false;
            return MakeExecuteResult(cExecuteKindFinished);
        }
        if (CheckTransportConnectionStatus() != 0 || CheckConnectionStateWithHostStation() != 0)
        {
            transport::g_piaResendingMessageManager->StopResending(m_requestAckId);
            m_requestAckId = 0;
            m_waitingJoinResponse = false;
            for (int index = 0; index < cReceivedFlagNum; ++index)
                m_received[index] = false;
            return MakeExecuteResult(cExecuteKindFinished);
        }
        if (!m_waitingJoinResponse)
        {
            transport::g_piaResendingMessageManager->StopResending(m_requestAckId);
            SelectStep(m_currentStep, &JoinMeshJob::AnalyzeJoinResponse, "JoinMeshJob::AnalyzeJoinResponse");
            m_requestAckId = 0;
            return MakeExecuteResult(cExecuteKindContinue);
        }
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    m_requestAckId = 0;
    SelectStep(m_currentStep, &JoinMeshJob::WaitJoinResponse, "JoinMeshJob::WaitJoinResponse");
    return MakeExecuteResult(cExecuteKindContinue);
}

JoinMeshJob::ExecuteResult JoinMeshJob::WaitJoinResponse()
{
    if (m_waitingJoinResponse)
    {
        if (CheckContextCallCancelled() != 0)
        {
            m_waitingJoinResponse = false;
            return MakeExecuteResult(cExecuteKindFinished);
        }
        if (CheckTransportConnectionStatus() != 0 || CheckConnectionStateWithHostStation() != 0)
        {
            m_waitingJoinResponse = false;
            return MakeExecuteResult(cExecuteKindFinished);
        }
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    SelectStep(m_currentStep, &JoinMeshJob::AnalyzeJoinResponse, "JoinMeshJob::AnalyzeJoinResponse");
    return MakeExecuteResult(cExecuteKindContinue);
}

unsigned int JoinMeshJob::UpdateLocalAndHostInformation(StationIndex localIndex, StationIndex hostIndex)
{
    transport::Station* hostStation = transport::StationManager::s_pInstance->GetStation(
        g_piaMesh->GetHostStationIndex());
    transport::Station* localStation = transport::StationManager::s_pInstance->m_localStation;
    if (hostStation == 0 || localStation == 0)
        return 0;
    localStation->SetStationIdAndIndex(
        transport::g_stationConnectionInfoTable->m_localStationConnectionInfo.m_stationLocation.m_principalId,
        localIndex);
    g_piaMesh->m_unknown5c = localIndex;
    g_piaMesh->StartUse(localIndex);
    if (transport::g_piaTransport->m_protocolManager.StartupProtocolsWithStationAddress(localStation->m_stationAddress)
        != 0)
        return 0;
    localStation->m_field20 = cConnectedStationState;
    hostStation->SetStationIdAndIndex(hostStation->m_stationId, hostIndex);
    g_piaMesh->m_unknown58 = hostIndex;
    g_piaMesh->NoticeMeshEvent(Mesh::cEventTypeJoin, localIndex);
    g_piaMesh->FixConnectedId(hostIndex);
    transport::StationManager::s_pInstance->m_invalidStationIndex = hostIndex;
    if (transport::g_piaTransport->m_relayRouteManager != 0)
        transport::g_piaTransport->m_relayRouteManager->SetRelayRoute(localIndex, hostIndex, hostIndex);
    return 1;
}

unsigned int JoinMeshJob::SetStationDataList(const unsigned char* data, unsigned int)
{
    const unsigned int infoSize = m_stationInfos->GetSerializedSize();
    const unsigned int entrySize = infoSize + (~infoSize & 3) + 1;
    const unsigned char hostSlot = data[cResponseHostSlot];
    const unsigned char count = data[cResponseCount];
    const unsigned char localSlot = data[cResponseLocalSlot];
    if (count == 0 || hostSlot == cResponseSlotNone || localSlot == cResponseSlotNone)
    {
        m_stationCount = 0;
        m_responseResult = data[cResponsePartCount];
        g_piaMesh->GetMeshProtocol()->SetWaitingJoinResponseJob(0);
        m_waitingJoinResponse = false;
        return 1;
    }
    if (m_stationCapacity < count)
    {
        m_stationCount = 0;
        m_responseResult = 3;
        g_piaMesh->GetMeshProtocol()->SetWaitingJoinResponseJob(0);
        m_waitingJoinResponse = false;
        return 1;
    }
    const unsigned char partCount = data[cResponsePartCount];
    if (partCount == 1)
    {
        if (g_piaMesh->m_meshEventListener != 0)
        {
            MeshEventListener::MeshEvent event;
            event.type = cMeshEvent17;
            event.stationIndex = static_cast<StationIndex>(cStationIndexInvalid);
            event.stationId = static_cast<unsigned long long>(data[cResponseEventIdHigh]) << 8
                              | data[cResponseEventIdLow]
                              | static_cast<unsigned long long>(data[cResponseEventIdByte]) << 16;
            g_piaMesh->m_meshEventListener->MeshEventListener::NoticeEvent(event);
        }
        g_piaMesh->m_unknownc8 = common::deserializeU32(data + cResponseSerialNumber);
        const StationIndex hostIndex = static_cast<StationIndex>(
            data[cResponseFirstPartEntries + entrySize * hostSlot + infoSize]);
        const StationIndex localIndex = static_cast<StationIndex>(
            data[cResponseFirstPartEntries + entrySize * localSlot + infoSize]);
        transport::StationConnectionInfo receivedLocalInfo;
        transport::StationConnectionInfo localInfo;
        receivedLocalInfo.Deserialize(data + cResponseFirstPartEntries + entrySize * localSlot);
        Result result = transport::g_piaTransport->GetLocalStationConnectionInfo(&localInfo);
        if (hostIndex < transport::StationManager::cStationNumMax
            && localIndex < transport::StationManager::cStationNumMax && result == 0 && receivedLocalInfo == localInfo)
        {
            if (UpdateLocalAndHostInformation(localIndex, hostIndex) == 0)
                return 0;
            m_stationCount = count;
            for (unsigned short entry = 0; entry < m_stationCount; ++entry)
            {
                (m_stationInfos + entry)->Deserialize(data + cResponseFirstPartEntries + entry * entrySize);
                m_stationIndices[entry] = data[cResponseFirstPartEntries + entry * entrySize + infoSize];
            }
            g_piaMesh->GetMeshProtocol()->SetWaitingJoinResponseJob(0);
            m_waitingJoinResponse = false;
            return 1;
        }
        localInfo.Trace(cTraceStationLevel);
        receivedLocalInfo.Trace(cTraceStationLevel);
        m_stationCount = 0;
        m_responseResult = 3;
        g_piaMesh->GetMeshProtocol()->SetWaitingJoinResponseJob(0);
        m_waitingJoinResponse = false;
        return 1;
    }
    else
    {
        if (partCount < 2 || partCount > 3)
            return 0;
        const unsigned char partIndex = data[cResponsePartIndex];
        if (!m_received[partIndex])
            return 1;
        unsigned int entryOffset = cResponseNextPartEntries;
        if (partIndex == 0)
        {
            if (g_piaMesh->m_meshEventListener != 0)
            {
                MeshEventListener::MeshEvent event;
                event.type = cMeshEvent17;
                event.stationIndex = static_cast<StationIndex>(cStationIndexInvalid);
                event.stationId = static_cast<unsigned long long>(data[cResponseEventIdHigh]) << 8
                                  | data[cResponseEventIdLow]
                                  | static_cast<unsigned long long>(data[cResponseEventIdByte]) << 16;
                g_piaMesh->m_meshEventListener->MeshEventListener::NoticeEvent(event);
            }
            g_piaMesh->m_unknownc8 = common::deserializeU32(data + cResponseSerialNumber);
            entryOffset = cResponseFirstPartEntries;
        }
        bool firstPart = true;
        for (int index = 0; index < cReceivedFlagNum; ++index)
            firstPart = firstPart & m_received[index];
        if (!firstPart)
        {
            if (m_responseStationCount != count)
                return 0;
            if (m_responseHostSlot != hostSlot)
                return 0;
            if (m_responseLocalSlot != localSlot)
                return 0;
            if (m_responsePartCount != partCount)
                return 0;
        }
        else
        {
            m_responseHostSlot = hostSlot;
            m_responseLocalSlot = localSlot;
            m_responseStationCount = count;
            m_responsePartCount = partCount;
        }
        const unsigned char entryCount = data[cResponseEntryCount];
        const unsigned char entryStart = data[cResponseEntryStart];
        for (unsigned int entry = 0; entry < entryCount; ++entry)
        {
            (m_stationInfos + entryStart + entry)->Deserialize(data + entryOffset + entry * entrySize);
            m_stationIndices[entryStart + entry] = data[entryOffset + entry * entrySize + infoSize];
        }
        m_received[partIndex] = false;
        bool pending = false;
        for (unsigned char index = 0; index < m_responsePartCount; ++index)
            pending = pending | m_received[index];
        if (pending)
            return 1;
        const StationIndex hostIndex = static_cast<StationIndex>(m_stationIndices[m_responseHostSlot]);
        const StationIndex localIndex = static_cast<StationIndex>(m_stationIndices[m_responseLocalSlot]);
        transport::StationConnectionInfo receivedLocalInfo(m_stationInfos[m_responseLocalSlot]);
        transport::StationConnectionInfo localInfo;
        Result result = transport::g_piaTransport->GetLocalStationConnectionInfo(&localInfo);
        if (hostIndex < transport::StationManager::cStationNumMax
            && localIndex < transport::StationManager::cStationNumMax && result == 0 && receivedLocalInfo == localInfo)
        {
            if (UpdateLocalAndHostInformation(localIndex, hostIndex) == 0)
                return 0;
            m_stationCount = m_responseStationCount;
            g_piaMesh->GetMeshProtocol()->SetWaitingJoinResponseJob(0);
            m_waitingJoinResponse = false;
            return 1;
        }
        localInfo.Trace(cTraceStationLevel);
        receivedLocalInfo.Trace(cTraceStationLevel);
        m_stationCount = 0;
        m_responseResult = 3;
        g_piaMesh->GetMeshProtocol()->SetWaitingJoinResponseJob(0);
        m_waitingJoinResponse = false;
        return 1;
    }
}

JoinMeshJob::ExecuteResult JoinMeshJob::AnalyzeJoinResponse()
{
    if (CheckContextCallCancelled() != 0)
        return MakeExecuteResult(cExecuteKindFinished);
    if (m_stationCount == 0)
    {
        CalcProcessingTimeOfJoin();
        if (m_callerContext != 0)
        {
            switch (m_responseResult)
            {
            case 1:
                if (m_callerContext->m_state - common::CallContext::cStateSuccess > 2u)
                    m_callerContext->SignalFailure(cResultJoinResponse1);
                common::g_sessionBeginMonitoringContent.m_joinResult = cResultJoinResponse1;
                common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
                g_piaMesh->ProcessingAtMeshBegin();
                m_callerContext = 0;
                break;
            case 0:
                if (m_callerContext->m_state - common::CallContext::cStateSuccess > 2u)
                    m_callerContext->SignalFailure(cResultJoinResponse0);
                common::g_sessionBeginMonitoringContent.m_joinResult = cResultJoinResponse0;
                common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
                g_piaMesh->ProcessingAtMeshBegin();
                m_callerContext = 0;
                break;
            case 2:
                if (g_piaMesh->m_meshEventListener != 0)
                {
                    MeshEventListener::MeshEvent event;
                    event.type = cMeshEvent14;
                    event.stationIndex = static_cast<StationIndex>(cStationIndexInvalid);
                    event.stationId = 0;
                    g_piaMesh->m_meshEventListener->MeshEventListener::NoticeEvent(event);
                }
                if (m_callerContext->m_state - common::CallContext::cStateSuccess > 2u)
                    m_callerContext->SignalFailure(cResultJoinFailed);
                common::g_sessionBeginMonitoringContent.m_joinResult = cResultJoinFailed;
                common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
                g_piaMesh->ProcessingAtMeshBegin();
                m_callerContext = 0;
                break;
            case 4:
                if (m_callerContext->m_state - common::CallContext::cStateSuccess > 2u)
                    m_callerContext->SignalFailure(cResultJoinFailed);
                common::g_sessionBeginMonitoringContent.m_joinResult = cResultJoinFailed;
                common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
                g_piaMesh->ProcessingAtMeshBegin();
                m_callerContext = 0;
                break;
            default:
                if (m_callerContext->m_state - common::CallContext::cStateSuccess > 2u)
                    m_callerContext->SignalFailure(cResultJoinResponseUnknown);
                common::g_sessionBeginMonitoringContent.m_joinResult = cResultJoinResponseUnknown;
                common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
                g_piaMesh->ProcessingAtMeshBegin();
                m_callerContext = 0;
                break;
            }
        }
        CleanupInternalContexts();
        return MakeExecuteResult(cExecuteKindFinished);
    }
    common::g_sessionBeginMonitoringContent.m_metric47 = static_cast<unsigned char>(m_stationCount);
    m_principalId = cInvalidPrincipalId;
    SelectStep(m_currentStep, &JoinMeshJob::WaitAllConnection, "JoinMeshJob::WaitAllConnection");
    m_joinState = 2;
    return MakeExecuteResult(cExecuteKindContinue);
}

JoinMeshJob::ExecuteResult JoinMeshJob::WaitAllConnection()
{
    if (CheckContextCallCancelled() != 0)
    {
        SelectStep(m_currentStep, &JoinMeshJob::StartDisconnectStations, "JoinMeshJob::StartDisconnectStations");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (g_piaMesh->m_leaveMeshJob->IsRunning())
        return MakeExecuteResult(cExecuteKindNextStep);
    if (g_piaMesh->m_kickoutManageJob->IsRunning())
        return MakeExecuteResult(cExecuteKindNextStep);
    const int localStationIndex = g_piaMesh->GetLocalStationIndex();
    if (localStationIndex >= transport::StationManager::cStationNumMax)
    {
        CalcProcessingTimeOfJoin();
        if (m_callerContext != 0)
        {
            Result result;
            switch (m_joinResult)
            {
            case cResultConnectionFailed643f:
            case cResultConnectionFailed6442:
            case cResultConnectionFailed6443:
            case cResultConnectionFailed6444:
            case cResultConnectionFailed6445:
            case cResultConnectionFailed6467:
            case cResultConnectionFailed6468:
            case cResultConnectionFailed6469:
            case cResultConnectionFailed646a:
            case cResultConnectionFailed644a:
                result = m_joinResult;
                break;
            default:
                result = cResultJoinFailed;
                break;
            }
            if (m_callerContext->m_state - common::CallContext::cStateSuccess > 2u)
                m_callerContext->SignalFailure(result);
            common::g_sessionBeginMonitoringContent.m_joinResult = result;
            common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
            g_piaMesh->ProcessingAtMeshBegin();
            m_callerContext = 0;
        }
        CleanupInternalContexts();
        SelectStep(m_currentStep, &JoinMeshJob::StartDisconnectStations, "JoinMeshJob::StartDisconnectStations");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    unsigned short connectedCount = 0;
    bool waiting = false;
    for (unsigned int index = 0; index < m_stationCount; ++index)
    {
        if (m_stationIndices[index] == static_cast<unsigned int>(localStationIndex))
        {
            ++connectedCount;
        }
        else if (m_stationConnected[index] == 0)
        {
            if (transport::g_stationConnectionInfoTable->GetStation(m_stationInfos[index]) != 0
                && g_piaMesh->IsValidStationIndex(static_cast<StationIndex>(m_stationIndices[index])))
            {
                ++connectedCount;
            }
            else
            {
                waiting = true;
                break;
            }
        }
    }
    if (waiting
        || (connectedCount == cLocalAndHostStationCount
            && (g_piaMesh->m_unknown80 == 0 || g_piaMesh->m_processUpdateMeshJob->m_started)))
    {
        unsigned int meshVersion = g_piaMesh->m_unknown80;
        if (meshVersion != 0)
        {
            ProcessUpdateMeshJob* updateJob = g_piaMesh->m_processUpdateMeshJob;
            if (updateJob->m_started)
                meshVersion = updateJob->m_updateVersion;
            if (m_lastUpdateCount < meshVersion)
            {
                for (unsigned int index = 0; index < m_stationCount; ++index)
                {
                    if (m_stationConnected[index] == 0)
                    {
                        m_stationConnected[index] = 1;
                        if (m_stationIndices[index] == static_cast<unsigned int>(localStationIndex))
                        {
                            m_stationConnected[index] = 0;
                        }
                        else
                        {
                            for (unsigned int entry = 0; entry < updateJob->m_stationCount; ++entry)
                            {
                                if (updateJob->m_stationIndices[entry] == m_stationIndices[index])
                                {
                                    if (updateJob->m_stationConnectionInfos[entry] == m_stationInfos[index])
                                        m_stationConnected[index] = 0;
                                    break;
                                }
                            }
                        }
                    }
                }
                m_lastUpdateCount = meshVersion;
            }
        }
        if (CheckTransportConnectionStatus() != 0)
            return MakeExecuteResult(cExecuteKindFinished);
        if (CheckConnectionStateWithHostStation() != 0)
        {
            SelectStep(m_currentStep, &JoinMeshJob::StartDisconnectStations, "JoinMeshJob::StartDisconnectStations");
            return MakeExecuteResult(cExecuteKindContinue);
        }
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    if (g_piaMesh->GetTime() < 0)
        return MakeExecuteResult(cExecuteKindNextStep);
    g_piaMesh->SetMeshState(Mesh::cStateCreated);
    g_piaMesh->m_disconnectReason = 0;
    g_piaMesh->GetProcessUpdateMeshJobPtr()->m_leaveMeshOnFailure = false;
    return SetNextStepAfterWaitAllConnection();
}

JoinMeshJob::ExecuteResult JoinMeshJob::CompleteProcess()
{
    if (m_callerContext != 0)
    {
        m_callerContext->SignalSuccess(0);
        m_callerContext = 0;
    }
    m_joinState = 4;
    g_piaMesh->SetMeshStartTime(common::g_piaJobScheduler->m_dispatchTime);
    CalcProcessingTimeOfJoin();
    CheckRelayConnectionForMonitoring();
    common::g_sessionBeginMonitoringContent.m_joinResult = 0;
    common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = 0xffffffff;
    common::g_sessionBeginMonitoringContent.m_joinMeshState = cMonitoringByteUnset;
    transport::g_piaTransport->SetMonitoringNetworkRtt(true);
    g_piaMesh->ProcessingAtMeshBegin();
    return MakeExecuteResult(cExecuteKindFinished);
}

JoinMeshJob::ExecuteResult JoinMeshJob::SetupLocalPlayerInfoFailure()
{
    if (m_callerContext->m_state - common::CallContext::cStateSuccess > 2u)
        m_callerContext->SignalFailure(cResultSetupLocalPlayerInfoFailed);
    common::g_sessionBeginMonitoringContent.m_joinResult = cResultSetupLocalPlayerInfoFailed;
    common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
    g_piaMesh->ProcessingAtMeshBegin();
    return MakeExecuteResult(cExecuteKindFinished);
}

JoinMeshJob::ExecuteResult JoinMeshJob::StartDisconnectStations()
{
    transport::Station* localStation = transport::StationManager::s_pInstance->m_localStation;
    if (localStation == 0)
        return MakeExecuteResult(cExecuteKindFinished);
    transport::Station** station = transport::StationManager::s_pInstance->m_stations.data;
    if (station != station + transport::StationManager::s_pInstance->m_stations.size)
    {
        do
        {
            if (*station != localStation)
            {
                if (g_piaMesh->IsValidStationIndex(static_cast<StationIndex>((*station)->m_field10)))
                    g_piaMesh->UnfixDisconnectedId(static_cast<StationIndex>((*station)->m_field10));
                (*station)->m_connectStationJob->Cleanup();
                (*station)->m_processConnectionRequestJob->Cleanup();
                transport::DisconnectStationJob* disconnectJob = (*station)->m_disconnectStationJob;
                if (disconnectJob != 0 && disconnectJob->Startup(*station) == 0)
                    disconnectJob->Ready(false);
                else
                    (*station)->Trace(cTraceStationLevel);
            }
            ++station;
        } while (station
                 != transport::StationManager::s_pInstance->m_stations.data
                        + transport::StationManager::s_pInstance->m_stations.size);
    }
    const common::TimeSpan timeout = common::TimeSpan::FromMSec(cDisconnectTimeoutMsec);
    m_disconnectDeadline = common::g_piaJobScheduler->m_dispatchTime + timeout;
    SelectStep(m_currentStep, &JoinMeshJob::WaitDisconnectStations, "JoinMeshJob::WaitDisconnectStations");
    return MakeExecuteResult(cExecuteKindContinue);
}

JoinMeshJob::ExecuteResult JoinMeshJob::WaitDisconnectStations()
{
    bool connected = false;
    transport::Station** station = transport::StationManager::s_pInstance->m_stations.data;
    if (station != station + transport::StationManager::s_pInstance->m_stations.size)
    {
        do
        {
            if ((*station)->m_field20 == cConnectedStationState
                && *station != transport::StationManager::s_pInstance->m_localStation)
            {
                connected = true;
                break;
            }
            ++station;
        } while (station
                 != transport::StationManager::s_pInstance->m_stations.data
                        + transport::StationManager::s_pInstance->m_stations.size);
    }
    if (connected && !(m_disconnectDeadline <= common::g_piaJobScheduler->m_dispatchTime))
        return MakeExecuteResult(cExecuteKindNextStep);
    return MakeExecuteResult(cExecuteKindFinished);
}

JoinMeshJob::ExecuteResult JoinMeshJob::LeaveMeshWithHostMigration()
{
    if (g_piaMesh->GetStationNum() > 1)
    {
        g_piaMesh->LeaveMeshWithHostMigration(&m_callContext);
        SelectStep(m_currentStep, &JoinMeshJob::WaitLeaveMesh, "JoinMeshJob::WaitLeaveMesh");
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    SelectStep(m_currentStep, &JoinMeshJob::CompleteCancelWithLeaveMesh, "JoinMeshJob::CompleteCancelWithLeaveMesh");
    return MakeExecuteResult(cExecuteKindContinue);
}

JoinMeshJob::ExecuteResult JoinMeshJob::LeaveMesh()
{
    g_piaMesh->LeaveMesh(&m_callContext);
    SelectStep(m_currentStep, &JoinMeshJob::WaitLeaveMesh, "JoinMeshJob::WaitLeaveMesh");
    return MakeExecuteResult(cExecuteKindNextStep);
}

JoinMeshJob::ExecuteResult JoinMeshJob::WaitLeaveMesh()
{
    if (m_callContext.m_state - common::CallContext::cStateSuccess <= 2u)
    {
        SelectStep(m_currentStep, &JoinMeshJob::CompleteCancelWithLeaveMesh,
                   "JoinMeshJob::CompleteCancelWithLeaveMesh");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    return MakeExecuteResult(cExecuteKindNextStep);
}

JoinMeshJob::ExecuteResult JoinMeshJob::CompleteCancelWithLeaveMesh()
{
    g_piaMesh->m_disconnectReason = 2;
    CleanupInternalContexts();
    m_callerContext->SignalCancel();
    m_callerContext = 0;
    CalcProcessingTimeOfJoin();
    CheckRelayConnectionForMonitoring();
    common::g_sessionBeginMonitoringContent.m_joinResult = cResultCancelled;
    common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
    common::g_sessionBeginMonitoringContent.m_joinMeshState = static_cast<unsigned char>(m_joinState);
    g_piaMesh->ProcessingAtMeshBegin();
    return MakeExecuteResult(cExecuteKindFinished);
}

void JoinMeshJob::Cleanup(Result result)
{
    if (IsRunning() && IsBackground())
    {
        m_cancelRequested = true;
        do
        {
            WaitForCompletion(2);
        } while (IsRunning());
    }
    CleanupImpl();
    CleanupInternalContexts();
    CleanupStations();
    m_waitingJoinResponse = false;
    for (int index = 0; index < cReceivedFlagNum; ++index)
        m_received[index] = false;
    if (m_requestAckId != 0)
    {
        if (transport::g_piaResendingMessageManager != 0)
            transport::g_piaResendingMessageManager->StopResending(m_requestAckId);
        m_requestAckId = 0;
    }
    m_lastUpdateCount = 0;
    for (unsigned int index = 0; index < m_stationCapacity; ++index)
        m_stationConnected[index] = 0;
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        m_callContext.Cancel();
    if (m_callerContext != 0)
    {
        if (m_callerContext->m_state == common::CallContext::cStateInProgress)
        {
            CalcProcessingTimeOfJoin();
            CheckRelayConnectionForMonitoring();
            if (result != 0)
            {
                if (m_callerContext->m_state != common::CallContext::cStateSuccess
                    && m_callerContext->m_state != common::CallContext::cStateFailure
                    && m_callerContext->m_state != common::CallContext::cStateCancelled)
                    m_callerContext->SignalFailure(result);
                common::g_sessionBeginMonitoringContent.m_joinResult = result;
            }
            else
            {
                if (m_callerContext->m_state != common::CallContext::cStateSuccess
                    && m_callerContext->m_state != common::CallContext::cStateFailure
                    && m_callerContext->m_state != common::CallContext::cStateCancelled)
                    m_callerContext->SignalFailure(cResultCancelled);
                common::g_sessionBeginMonitoringContent.m_joinResult = cResultCancelled;
            }
            common::g_sessionBeginMonitoringContent.m_joinTargetIdHash = common::hashWithMd5(m_principalId);
            g_piaMesh->ProcessingAtMeshBegin();
        }
        m_callerContext = 0;
    }
    m_joinState = 0;
}

int JoinMeshJob::GetJoinMeshState() const
{
    return m_joinState;
}

void JoinMeshJob::Trace(unsigned long long) const
{
}

unsigned int JoinMeshJob::StartupImpl()
{
    Reset(true);
    SelectStep(m_currentStep, &JoinMeshJob::SetupLocalPlayerInfo, "JoinMeshJob::SetupLocalPlayerInfo");
    return 1;
}

void JoinMeshJob::CleanupImpl()
{
}

void JoinMeshJob::SetHostInfoToMonitoringData(const transport::StationConnectionInfo&)
{
}

void JoinMeshJob::SetNextStepAtSetupLocalPlayerInfoImpl()
{
    SelectStep(m_currentStep, &JoinMeshJob::SetupLocalStation, "JoinMeshJob::SetupLocalStation");
}

JoinMeshJob::ExecuteResult JoinMeshJob::SetNextStepAfterWaitAllConnection()
{
    SelectStep(m_currentStep, &JoinMeshJob::CompleteProcess, "JoinMeshJob::CompleteProcess");
    return MakeExecuteResult(cExecuteKindContinue);
}

void JoinMeshJob::CleanupInternalContexts()
{
    m_internalContext->SignalCancel();
    m_internalContext->Reset();
}

}
}
}
