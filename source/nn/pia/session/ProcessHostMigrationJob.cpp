#include "nn/pia/session/ProcessHostMigrationJob.hpp"

#include <coreinit/memory.h>

#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/SessionStateMonitoringContent.hpp"
#include "nn/pia/session/JoinMeshJob.hpp"
#include "nn/pia/session/LeaveMeshJob.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/MeshEventListener.hpp"
#include "nn/pia/session/MeshProtocol.hpp"
#include "nn/pia/session/ProcessDestroyMeshJob.hpp"
#include "nn/pia/session/ProcessJoinRequestJob.hpp"
#include "nn/pia/session/ProcessUpdateMeshJob.hpp"
#include "nn/pia/session/RelayRouteManageJob.hpp"
#include "nn/pia/transport/DisconnectStationJob.hpp"
#include "nn/pia/transport/IdentificationInfoTable.hpp"
#include "nn/pia/transport/RelayRouteManager.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationConnectionInfo.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"
#include "nn/pia/transport/StationManager.hpp"

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
    cResultConnectionError6c50 = 0x6c50,
    cResultInvalidState = 0x10c08,
    cResultInvalidParameter = 0x10c07,
    cMeshEventKind2 = 2,
    cMeshEventKind3 = 3,
    cMeshEventKind4 = 4,
    cMeshEventKind19 = 0x13,
    cResultJoinFailed = 0x6439,
    cJoinMeshStateCompleted = 3,
    cDisconnectReason7 = 7,
    cStationState5 = 5,
    cStationState6 = 6,
    cStationCountUnset = 0xff,
    cWaitMarginMsec = 500,
    cSequenceSendGreeting = 7,
    cSequenceWaitGreetingResponse = 8,
    cSequenceSendMigrationFinish = 10,
    cSequenceWaitNewHostGreeting = 11,
    cSequenceWaitNewHostFinished = 12,
    cTraceStation = 0x40000000,
    cGreetingResendIntervalMsec = 500,
    cBitmapTopBit = 0x80000000,
    cRankDecisionValue1 = 1,
    cRankDecisionValue2 = 2,
    cStationLocationField23Value2 = 2,
    cScoreWeightEdm = 0x800000,
    cScoreWeightNoRelay = 0x100,
    cScoreWeightRelayCount = 0x1000,
    cScoreWeightCallback = 0x1000000,
    cScoreUnset = 0xffffffff
};

const unsigned long long cTimeInvalid = 0xffffffffffffffffULL;

inline void SelectStep(common::StepSequenceJob::Step& slot,
                       ProcessHostMigrationJob::ExecuteResult (ProcessHostMigrationJob::*function)(), const char* name)
{
    const common::StepSequenceJob::Step step(
        static_cast<ProcessHostMigrationJob::ExecuteResult (common::StepSequenceJob::*)()>(function), name);
    slot = step;
}
}

ProcessHostMigrationJob::ProcessHostMigrationJob()
    : common::StepSequenceJob(),
      m_waitDeadline(),
      m_waitTimeMsec(10000),
      m_greetingDeadline(),
      m_startTime(),
      m_elapsedTime(),
      m_hostMigrationCount(0),
      m_stationCountAtStart(0),
      m_stationCountAtEnd(0),
      m_started(false),
      m_restartRequested(false)
{
    OSBlockSet(m_greetingPending, 0, sizeof(m_greetingPending));
    m_greetingSent = false;
    m_finishReceived = false;
    m_finishSuccess = false;
    m_unknownA5 = false;
    m_greetingSenderIndex = static_cast<StationIndex>(cStationIndexInvalid);
    m_oldHostIndex = static_cast<StationIndex>(cStationIndexInvalid);
    m_newHostIndex = static_cast<StationIndex>(cStationIndexInvalid);
    m_oldHostPrincipalId = 0;
    m_greetingSenderPrincipalId = 0;
    m_unknownB8 = false;
    m_unknownE4 = 0;
    m_unknownE0 = 0;
    m_unknownBC = g_piaMesh->m_hostMigrationMode == 2;
    OSBlockSet(m_rankDecision, 0, sizeof(m_rankDecision));
    OSBlockSet(m_candidateRanking, cStationIndexInvalid, sizeof(m_candidateRanking));
}

ProcessHostMigrationJob::~ProcessHostMigrationJob()
{
}

void ProcessHostMigrationJob::Trace(unsigned long long) const
{
}

unsigned int ProcessHostMigrationJob::StartupMultiImpl(bool, unsigned short)
{
    return 0;
}

void ProcessHostMigrationJob::ProcessingAtEndHM(bool stationCountValid)
{
    common::Time now;
    now.SetNow();
    m_elapsedTime = common::TimeSpan(now.m_tick - m_startTime.m_tick);
    if (stationCountValid)
        m_stationCountAtEnd = static_cast<unsigned char>(g_piaMesh->GetStationNum());
}

void ProcessHostMigrationJob::ProcessingAtStartHM()
{
    common::Time now;
    now.SetNow();
    m_startTime = now;
    m_elapsedTime.m_tick = 0;
    const unsigned char stationNum = static_cast<unsigned char>(g_piaMesh->GetStationNum());
    m_unknown6e = 0;
    m_stationCountAtStart = stationNum;
    ++m_hostMigrationCount;
    m_stationCountAtEnd = cStationCountUnset;
}

void ProcessHostMigrationJob::SearchRefugeRelayRoute()
{
    transport::RelayRouteManager* manager = transport::g_piaTransport->m_relayRouteManager;
    if (manager != 0)
    {
        const StationIndex host = g_piaMesh->GetHostStationIndex();
        if (host <= cStationIndexMax)
            manager->SearchRefugeRelayRoute(host);
    }
}

bool ProcessHostMigrationJob::Startup(bool multi, StationIndex station)
{
    if (m_started)
    {
        if (m_finishReceived)
        {
            if (IsRunning())
                Reset(false);
            ProcessingAtEndHM(true);
            CleanupStatus();
            g_piaMesh->ProcessingAtEndHM();
        }
        else
        {
            m_restartRequested = true;
            return false;
        }
    }
    if (g_piaMesh->m_leaveMeshJob->IsRunning() || g_piaMesh->m_processDestroyMeshJob->m_started)
        return false;
    if (g_piaMesh->m_joinMeshJob->IsRunning())
    {
        if (g_piaMesh->m_meshEventListener != 0)
        {
            MeshEventListener::MeshEvent event;
            event.type = cMeshEventKind19;
            event.stationIndex = static_cast<StationIndex>(cStationIndexInvalid);
            event.stationId = 0;
            g_piaMesh->m_meshEventListener->MeshEventListener::NoticeEvent(event);
        }
        else
        {
            g_piaMesh->m_joinMeshJob->Cleanup(Result(cResultJoinFailed));
            g_piaMesh->m_joinMeshJob->Reset(true);
            g_piaMesh->m_processUpdateMeshJob->Cleanup();
            g_piaMesh->m_processUpdateMeshJob->Reset(true);
        }
        return false;
    }
    if (g_piaMesh->GetMeshState() != Mesh::cStateCreated || g_piaMesh->GetLocalStationIndex() > cStationIndexMax)
        return false;
    OSBlockSet(m_greetingPending, 0, sizeof(m_greetingPending));
    m_unknownA5 = false;
    m_unknownB8 = false;
    m_oldHostPrincipalId = 0;
    m_finishReceived = false;
    m_greetingSenderPrincipalId = 0;
    m_greetingSenderIndex = static_cast<StationIndex>(cStationIndexInvalid);
    m_newHostIndex = static_cast<StationIndex>(cStationIndexInvalid);
    m_oldHostIndex = static_cast<StationIndex>(cStationIndexInvalid);
    m_finishSuccess = false;
    m_greetingSent = false;
    Reset(true);
    if (!StartupImpl(multi, station))
        return false;
    SearchRefugeRelayRoute();
    ProcessingAtStartHM();
    m_restartRequested = false;
    m_started = true;
    g_piaMesh->NoticeMeshEvent(static_cast<Mesh::EventType>(cMeshEventKind4),
                               static_cast<StationIndex>(cStationIndexInvalid));
    return true;
}

void ProcessHostMigrationJob::SetOldHostStation(StationIndex station, unsigned long long principalId)
{
    m_oldHostIndex = station;
    m_oldHostPrincipalId = principalId;
    if (principalId == 0)
    {
        transport::Station* oldHost = transport::StationManager::s_pInstance->GetStation(m_oldHostIndex);
        if (oldHost != 0)
        {
            unsigned long long oldHostPrincipalId;
            if (oldHost->GetPrincipalId(&oldHostPrincipalId) == 0)
                m_oldHostPrincipalId = oldHostPrincipalId;
        }
    }
}

unsigned int ProcessHostMigrationJob::DecideNextHostCommonProc()
{
    SetOldHostStation(g_piaMesh->GetHostStationIndex(), 0);
    if (g_piaMesh->GetLocalStationIndex() > cStationIndexMax || m_newHostIndex > cStationIndexMax)
        return 0;
    if (m_newHostIndex == g_piaMesh->GetLocalStationIndex())
    {
        const common::TimeSpan waitSpan = common::TimeSpan::FromMSec(m_waitTimeMsec - cWaitMarginMsec);
        m_waitDeadline = common::g_piaJobScheduler->m_dispatchTime + waitSpan;
        return 1;
    }
    m_unknownA5 = true;
    m_greetingSenderIndex = static_cast<StationIndex>(cStationIndexInvalid);
    m_greetingSenderPrincipalId = 0;
    const common::TimeSpan waitSpan = common::TimeSpan::FromMSec(m_waitTimeMsec);
    m_waitDeadline = common::g_piaJobScheduler->m_dispatchTime + waitSpan;
    return 2;
}

unsigned int ProcessHostMigrationJob::CleanupOldHostInfoCommonProc()
{
    g_piaMesh->GetMeshProtocol()->SendMigrationResponse(m_oldHostIndex);
    DisconnectStation(m_oldHostIndex);
    return 1;
}

unsigned int ProcessHostMigrationJob::PreprocessForBecomingHostCommon()
{
    if (g_piaMesh->GetTime() == cTimeInvalid)
        return 0;
    g_piaMesh->m_unknown56 = 0;
    if (g_piaMesh->m_unknown0c->Startup())
        g_piaMesh->m_unknown0c->Ready(false);
    else
        return 0;
    if (g_piaMesh->m_processUpdateMeshJob->m_started)
    {
        g_piaMesh->m_processUpdateMeshJob->Cleanup();
        g_piaMesh->m_processUpdateMeshJob->Reset(false);
    }
    return 1;
}

void ProcessHostMigrationJob::PostprocessForBecomingHostCommon()
{
    g_piaMesh->m_unknown58 = m_newHostIndex;
    transport::StationManager* stationManager = transport::StationManager::s_pInstance;
    stationManager->m_invalidStationIndex = m_newHostIndex;
    for (transport::Station** station = stationManager->m_stations.data;
         station != stationManager->m_stations.data + stationManager->m_stations.size; ++station)
    {
        if (((*station) == stationManager->m_localStation || (*station)->m_field20 == cStationState5)
            && static_cast<StationIndex>((*station)->m_field10) < cStationIndexMax)
        {
            if (!g_piaMesh->IsValidStationIndex(static_cast<StationIndex>((*station)->m_field10)))
                g_piaMesh->FixConnectedId(static_cast<StationIndex>((*station)->m_field10));
        }
        else
        {
            (*station)->m_field20 = cStationState6;
        }
    }
    g_piaMesh->m_connectedStationBitmap = g_piaMesh->m_validStationBitmap;
    if (g_piaMesh->m_unknown24 != 0)
        g_piaMesh->m_unknown24->PrepareForBecomingNewHost();
}

}
}
}

namespace
{
bool UpdateHostStationConnectionInfo(nn::pia::transport::Station* station)
{
    nn::pia::transport::StationConnectionInfoTable* table = nn::pia::transport::g_stationConnectionInfoTable;
    nn::pia::transport::StationConnectionInfo connectionInfo;
    if (table->GetStationConnectionInfo(station, &connectionInfo) != 0)
        return false;
    table->m_hostStationConnectionInfo = connectionInfo;
    table->m_hostStationAddress = station->m_stationAddress;
    return true;
}
}

namespace nn
{
namespace pia
{
namespace session
{

ProcessHostMigrationJob::ExecuteResult ProcessHostMigrationJob::SendGreetingMessage()
{
    m_hostMigrationSequence = cSequenceSendGreeting;
    if (g_piaMesh->CheckConnectionError() == cResultConnectionError6c50)
    {
        SelectStep(m_currentStep, &ProcessHostMigrationJob::HostMigrationFailure,
                   "ProcessHostMigrationJob::HostMigrationFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    const StationIndex localIndex = g_piaMesh->GetLocalStationIndex();
    transport::Station* localStation = transport::StationManager::s_pInstance->m_localStation;
    bool updated = false;
    if (localStation != 0)
    {
        updated = UpdateHostStationConnectionInfo(localStation);
        if (!updated)
            localStation->Trace(cTraceStation);
    }
    if (!updated)
    {
        SelectStep(m_currentStep, &ProcessHostMigrationJob::HostMigrationFailure,
                   "ProcessHostMigrationJob::HostMigrationFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    transport::Station* oldHost = transport::StationManager::s_pInstance->GetStation(m_oldHostIndex);
    if (oldHost != 0)
    {
        if (g_piaMesh->IsValidStationIndex(m_oldHostIndex))
            DisconnectStation(m_oldHostIndex);
        else
            oldHost->m_field20 = cStationState6;
    }
    int failedStations[cStationNumMax] = {0};
    int failedCount = 0;
    for (int index = 0; index < cStationNumMax; ++index)
    {
        if (index == localIndex || !g_piaMesh->IsValidStationIndex(static_cast<StationIndex>(index)))
        {
            m_greetingPending[index] = 0;
        }
        else
        {
            const bool sent = g_piaMesh->GetMeshProtocol()->SendGreeting(static_cast<StationIndex>(index));
            m_greetingPending[index] = 1;
            if (!sent && transport::StationManager::s_pInstance->GetStation(static_cast<StationIndex>(index)) == 0)
            {
                failedStations[failedCount] = index;
                ++failedCount;
                m_greetingPending[index] = 0;
            }
        }
    }
    for (int failed = 0; failed < failedCount; ++failed)
    {
        g_piaMesh->UnfixDisconnectedId(static_cast<StationIndex>(failedStations[failed]));
        transport::Station* station = transport::StationManager::s_pInstance->GetStation(
            static_cast<StationIndex>(failedStations[failed]));
        if (station != 0)
        {
            station->m_disconnectStationJob->ProcessDisconnection(station);
            transport::g_stationConnectionInfoTable->EraseFromTable(station);
            transport::IdentificationInfoTable::GetInstance()->EraseFromTable(station);
            station->CleanupJobs();
            station->Cleanup();
            transport::StationManager::s_pInstance->DestroyStation(station);
            transport::g_piaTransport->OutputStreamUpdateEvent();
        }
    }
    common::Time deadline;
    deadline.m_tick = common::g_piaJobScheduler->m_dispatchTime.m_tick
                      + common::TimeSpan::GetTicksPerMSec().m_tick * cGreetingResendIntervalMsec;
    m_greetingDeadline = deadline;
    m_greetingSent = true;
    SelectStep(m_currentStep, &ProcessHostMigrationJob::WaitGreetingResponse,
               "ProcessHostMigrationJob::WaitGreetingResponse");
    return MakeExecuteResult(cExecuteKindNextStep);
}

ProcessHostMigrationJob::ExecuteResult ProcessHostMigrationJob::SendMigrationFinish()
{
    m_hostMigrationSequence = cSequenceSendMigrationFinish;
    if (g_piaMesh->CheckConnectionError() == cResultConnectionError6c50)
    {
        SelectStep(m_currentStep, &ProcessHostMigrationJob::HostMigrationFailure,
                   "ProcessHostMigrationJob::HostMigrationFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    g_piaMesh->GetMeshProtocol()->SendMigrationFinish(m_unknownBB);
    m_greetingSent = false;
    g_piaMesh->GetMeshProtocol()->SendUpdateMesh(true, 1);
    SelectStep(m_currentStep, &ProcessHostMigrationJob::HostMigrationSuccess,
               "ProcessHostMigrationJob::HostMigrationSuccess");
    return MakeExecuteResult(cExecuteKindContinue);
}

ProcessHostMigrationJob::ExecuteResult ProcessHostMigrationJob::WaitGreetingResponse()
{
    m_hostMigrationSequence = cSequenceWaitGreetingResponse;
    if (g_piaMesh->CheckConnectionError() == cResultConnectionError6c50)
    {
        SelectStep(m_currentStep, &ProcessHostMigrationJob::HostMigrationFailure,
                   "ProcessHostMigrationJob::HostMigrationFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    for (int index = 0; index < cStationNumMax; ++index)
    {
        if (m_greetingPending[index] != 0)
        {
            if (common::g_piaJobScheduler->m_dispatchTime < m_waitDeadline)
            {
                const common::Time now = common::g_piaJobScheduler->m_dispatchTime;
                if (m_greetingDeadline < now)
                {
                    for (int pending = 0; pending < cStationNumMax; ++pending)
                    {
                        if (m_greetingPending[pending] != 0)
                        {
                            if (!g_piaMesh->GetMeshProtocol()->SendGreeting(static_cast<StationIndex>(pending)))
                            {
                                if (!g_piaMesh->IsValidStationIndex(static_cast<StationIndex>(pending)))
                                {
                                    m_greetingPending[pending] = 0;
                                }
                                else if (transport::StationManager::s_pInstance->GetStation(
                                             static_cast<StationIndex>(pending))
                                         == 0)
                                {
                                    g_piaMesh->UnfixDisconnectedId(static_cast<StationIndex>(pending));
                                    m_greetingPending[pending] = 0;
                                }
                            }
                            else
                            {
                                transport::StationManager::s_pInstance->GetStation(static_cast<StationIndex>(pending));
                            }
                        }
                    }
                    m_greetingDeadline.m_tick =
                        now.m_tick + common::TimeSpan::GetTicksPerMSec().m_tick * cGreetingResendIntervalMsec;
                }
                return MakeExecuteResult(cExecuteKindNextStep);
            }
            m_unknownBB = false;
            PreprocessForSendingMigrationFinish();
            return MakeExecuteResult(cExecuteKindContinue);
        }
    }
    m_unknownBB = true;
    PreprocessForSendingMigrationFinish();
    return MakeExecuteResult(cExecuteKindContinue);
}

ProcessHostMigrationJob::ExecuteResult ProcessHostMigrationJob::WaitNewHostGreeting()
{
    m_hostMigrationSequence = cSequenceWaitNewHostGreeting;
    if (g_piaMesh->CheckConnectionError() == cResultConnectionError6c50)
    {
        SelectStep(m_currentStep, &ProcessHostMigrationJob::HostMigrationFailure,
                   "ProcessHostMigrationJob::HostMigrationFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_greetingSenderIndex == cStationIndexInvalid)
    {
        if (m_waitDeadline <= common::g_piaJobScheduler->m_dispatchTime)
        {
            if (CheckWhetherReselectNewHost() == 0)
                SelectStep(m_currentStep, &ProcessHostMigrationJob::HostMigrationFailure,
                           "ProcessHostMigrationJob::HostMigrationFailure");
            return MakeExecuteResult(cExecuteKindContinue);
        }
        return MakeExecuteResult(cExecuteKindNextStep);
    }
    transport::Station* newHost;
    if (m_unknownBC)
    {
        m_newHostIndex = m_greetingSenderIndex;
        g_piaMesh->m_unknown58 = m_greetingSenderIndex;
        transport::StationManager::s_pInstance->m_invalidStationIndex = m_newHostIndex;
        newHost = transport::StationManager::s_pInstance->GetStation(m_newHostIndex);
    }
    else
    {
        if (m_greetingSenderIndex != m_newHostIndex)
        {
            SelectStep(m_currentStep, &ProcessHostMigrationJob::HostMigrationFailure,
                       "ProcessHostMigrationJob::HostMigrationFailure");
            return MakeExecuteResult(cExecuteKindContinue);
        }
        g_piaMesh->m_unknown58 = m_newHostIndex;
        transport::StationManager::s_pInstance->m_invalidStationIndex = m_newHostIndex;
        newHost = transport::StationManager::s_pInstance->GetStation(m_newHostIndex);
    }
    if (newHost == 0)
    {
        SelectStep(m_currentStep, &ProcessHostMigrationJob::HostMigrationFailure,
                   "ProcessHostMigrationJob::HostMigrationFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (!UpdateHostStationConnectionInfo(newHost))
    {
        newHost->Trace(cTraceStation);
        SelectStep(m_currentStep, &ProcessHostMigrationJob::HostMigrationFailure,
                   "ProcessHostMigrationJob::HostMigrationFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (m_oldHostIndex != m_newHostIndex)
    {
        if (transport::StationManager::s_pInstance->GetStation(m_oldHostIndex) != 0)
            DisconnectStation(m_oldHostIndex);
        g_piaMesh->NoticeMeshEvent(static_cast<Mesh::EventType>(cMeshEventKind2), m_newHostIndex);
    }
    g_piaMesh->GetMeshProtocol()->SendGreetingResponse(m_newHostIndex);
    m_unknownA5 = false;
    m_unknownB8 = true;
    SelectStep(m_currentStep, &ProcessHostMigrationJob::WaitNewHostFinished,
               "ProcessHostMigrationJob::WaitNewHostFinished");
    return MakeExecuteResult(cExecuteKindNextStep);
}

ProcessHostMigrationJob::ExecuteResult ProcessHostMigrationJob::WaitNewHostFinished()
{
    m_hostMigrationSequence = cSequenceWaitNewHostFinished;
    if (g_piaMesh->CheckConnectionError() == cResultConnectionError6c50)
    {
        SelectStep(m_currentStep, &ProcessHostMigrationJob::HostMigrationFailure,
                   "ProcessHostMigrationJob::HostMigrationFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (!m_finishReceived)
    {
        if (common::g_piaJobScheduler->m_dispatchTime < m_waitDeadline && !m_restartRequested
            && g_piaMesh->IsValidStationIndex(m_newHostIndex))
            return MakeExecuteResult(cExecuteKindNextStep);
        if (CheckWhetherReselectNewHost() == 0)
            SelectStep(m_currentStep, &ProcessHostMigrationJob::HostMigrationFailure,
                       "ProcessHostMigrationJob::HostMigrationFailure");
        m_restartRequested = false;
        return MakeExecuteResult(cExecuteKindContinue);
    }
    SelectStep(m_currentStep, &ProcessHostMigrationJob::HostMigrationSuccess,
               "ProcessHostMigrationJob::HostMigrationSuccess");
    m_finishSuccess = false;
    return MakeExecuteResult(cExecuteKindContinue);
}

ProcessHostMigrationJob::ExecuteResult ProcessHostMigrationJob::HostMigrationSuccess()
{
    if (g_piaMesh->CheckConnectionError() == cResultConnectionError6c50)
    {
        SelectStep(m_currentStep, &ProcessHostMigrationJob::HostMigrationFailure,
                   "ProcessHostMigrationJob::HostMigrationFailure");
        return MakeExecuteResult(cExecuteKindContinue);
    }
    if (g_piaMesh->IsHost())
    {
        g_piaMesh->NoticeMeshEvent(static_cast<Mesh::EventType>(cMeshEventKind2), g_piaMesh->GetLocalStationIndex());
        g_piaMesh->m_unknown56 = 1;
    }
    ProcessingAtEndHM(true);
    CleanupStatus();
    g_piaMesh->ProcessingAtEndHM();
    return MakeExecuteResult(cExecuteKindFinished);
}

ProcessHostMigrationJob::ExecuteResult ProcessHostMigrationJob::HostMigrationFailure()
{
    g_piaMesh->NoticeMeshEvent(static_cast<Mesh::EventType>(cMeshEventKind3),
                               static_cast<StationIndex>(cStationIndexInvalid));
    ProcessingAtEndHM(false);
    g_piaMesh->m_disconnectReason = cDisconnectReason7;
    g_piaMesh->ProcessingAtMeshEnd(static_cast<Mesh::DisconnectReason>(cDisconnectReason7));
    g_piaMesh->CleanupStatus();
    CleanupStatus();
    return MakeExecuteResult(cExecuteKindFinished);
}

void ProcessHostMigrationJob::ReceiveGreetingResponse(StationIndex station)
{
    if (station > cStationIndexMax)
        return;
    m_greetingPending[station] = 0;
}

void ProcessHostMigrationJob::ReceiveGreeting(StationIndex station)
{
    m_greetingSenderIndex = station;
    transport::Station* sender = transport::StationManager::s_pInstance->GetStation(station);
    if (sender != 0)
    {
        unsigned long long senderPrincipalId;
        if (sender->GetPrincipalId(&senderPrincipalId) == 0)
            m_greetingSenderPrincipalId = senderPrincipalId;
    }
}

void ProcessHostMigrationJob::ReceiveFinish(bool success)
{
    m_finishSuccess = success;
    m_finishReceived = true;
    m_unknownB8 = false;
}

void ProcessHostMigrationJob::Cleanup()
{
    CleanupStatus();
    CleanupImpl();
}

bool ProcessHostMigrationJob::StartupMulti(bool multi, const unsigned char* ranking, unsigned int valueE0,
                                           unsigned int valueE4)
{
    if (m_started)
    {
        if (m_finishReceived)
        {
            if (IsRunning())
                Reset(false);
            ProcessingAtEndHM(true);
            CleanupStatus();
            g_piaMesh->ProcessingAtEndHM();
        }
        else
        {
            m_restartRequested = true;
            return false;
        }
    }
    if (g_piaMesh->m_leaveMeshJob->IsRunning() || g_piaMesh->m_processDestroyMeshJob->m_started)
        return false;
    if (g_piaMesh->m_joinMeshJob->IsRunning())
    {
        if (g_piaMesh->GetJoinMeshState() < cJoinMeshStateCompleted)
        {
            if (g_piaMesh->m_meshEventListener != 0)
            {
                MeshEventListener::MeshEvent event;
                event.type = cMeshEventKind19;
                event.stationIndex = static_cast<StationIndex>(cStationIndexInvalid);
                event.stationId = 0;
                g_piaMesh->m_meshEventListener->MeshEventListener::NoticeEvent(event);
            }
            else
            {
                g_piaMesh->m_joinMeshJob->Cleanup(Result(cResultJoinFailed));
                g_piaMesh->m_joinMeshJob->Reset(true);
                g_piaMesh->m_processUpdateMeshJob->Cleanup();
                g_piaMesh->m_processUpdateMeshJob->Reset(true);
            }
            return false;
        }
    }
    else if (g_piaMesh->GetMeshState() != Mesh::cStateCreated)
    {
        return false;
    }
    if (g_piaMesh->GetLocalStationIndex() > cStationIndexMax)
        return false;
    transport::StationManager* stationManager = transport::StationManager::s_pInstance;
    for (transport::Station** station = stationManager->m_stations.data;
         station != stationManager->m_stations.data + stationManager->m_stations.size; ++station)
    {
        if (((*station) != stationManager->m_localStation && (*station)->m_field20 != cStationState5)
            || !g_piaMesh->IsValidStationIndex(static_cast<StationIndex>((*station)->m_field10)))
        {
            if (g_piaMesh->IsValidStationIndex(static_cast<StationIndex>((*station)->m_field10)))
                g_piaMesh->UnfixDisconnectedId(static_cast<StationIndex>((*station)->m_field10));
            (*station)->m_field20 = cStationState6;
        }
    }
    OSBlockSet(m_greetingPending, 0, sizeof(m_greetingPending));
    m_greetingSenderPrincipalId = 0;
    m_oldHostPrincipalId = 0;
    m_greetingSenderIndex = static_cast<StationIndex>(cStationIndexInvalid);
    m_finishSuccess = false;
    m_newHostIndex = static_cast<StationIndex>(cStationIndexInvalid);
    m_greetingSent = false;
    m_oldHostIndex = static_cast<StationIndex>(cStationIndexInvalid);
    m_unknownA5 = false;
    m_finishReceived = false;
    m_unknownB8 = false;
    if (ranking != 0)
    {
        SetHostCandidateRanking(ranking, g_piaMesh->m_unknown54, valueE0, valueE4);
    }
    else
    {
        m_unknownE0 = 0;
        m_unknownE4 = 0;
    }
    Reset(true);
    if (!StartupMultiImpl(multi, g_piaMesh->m_unknown54))
        return false;
    SearchRefugeRelayRoute();
    ProcessingAtStartHM();
    m_restartRequested = false;
    m_started = true;
    g_piaMesh->NoticeMeshEvent(static_cast<Mesh::EventType>(cMeshEventKind4),
                               static_cast<StationIndex>(cStationIndexInvalid));
    return true;
}

Result ProcessHostMigrationJob::MakeHostCandidateRanking(StationIndex station, unsigned char* ranking,
                                                         unsigned int* meshValue, unsigned int* relayValue,
                                                         bool fromConnectionInfo)
{
    Mesh* mesh = g_piaMesh;
    transport::RelayRouteManager* relayManager = transport::g_piaTransport->m_relayRouteManager;
    unsigned int scores[cStationNumMax] = {0};
    if (fromConnectionInfo)
    {
        transport::StationConnectionInfoTable* connectionInfoTable = transport::g_stationConnectionInfoTable;
        transport::StationConnectionInfo connectionInfo;
        for (int index = 0; index < mesh->m_unknown54; ++index)
        {
            scores[index] = cScoreUnset;
            transport::Station* candidate = 0;
            if (index != station && mesh->IsValidStationIndex(static_cast<StationIndex>(index))
                && (candidate = transport::StationManager::s_pInstance->GetStation(static_cast<StationIndex>(index)))
                       != 0
                && connectionInfoTable->GetStationConnectionInfo(candidate, &connectionInfo) == 0)
            {
                int relayCount = 0;
                bool direct = true;
                if (relayManager != 0)
                {
                    StationIndex relay;
                    if (relayManager->GetOriginalRelayRoute(static_cast<StationIndex>(index), station, &relay) == 0)
                        direct = station == relay;
                    unsigned int directStations;
                    if (relayManager->GetOriginalDirectStationList(static_cast<StationIndex>(index), &directStations)
                        == 0)
                    {
                        for (; directStations != 0; directStations <<= 1)
                        {
                            if ((directStations & cBitmapTopBit) != 0)
                                ++relayCount;
                        }
                    }
                    if (direct)
                        --relayCount;
                }
                unsigned int callbackValue = 0;
                if (mesh->m_hostCandidateRankingCallback != 0)
                    callbackValue = mesh->m_hostCandidateRankingCallback(static_cast<StationIndex>(index), true);
                unsigned int edmWeight = 0;
                if (connectionInfo.m_stationLocation.m_field23 == cStationLocationField23Value2)
                    edmWeight = cScoreWeightEdm;
                unsigned int routeWeight = cScoreWeightNoRelay;
                if (direct)
                    routeWeight = 0;
                scores[index] = edmWeight + routeWeight + (mesh->m_unknown54 - relayCount) * cScoreWeightRelayCount
                                + callbackValue * cScoreWeightCallback + index + 1;
            }
        }
    }
    else
    {
        for (int index = 0; index < mesh->m_unknown54; ++index)
        {
            scores[index] = cScoreUnset;
            if (index != station && mesh->IsValidStationIndex(static_cast<StationIndex>(index)))
            {
                int relayCount = 0;
                bool direct = true;
                if (relayManager != 0)
                {
                    StationIndex relay;
                    if (relayManager->GetOriginalRelayRoute(static_cast<StationIndex>(index), station, &relay) == 0)
                        direct = station == relay;
                    unsigned int directStations;
                    if (relayManager->GetOriginalDirectStationList(static_cast<StationIndex>(index), &directStations)
                        == 0)
                    {
                        for (; directStations != 0; directStations <<= 1)
                        {
                            if ((directStations & cBitmapTopBit) != 0)
                                ++relayCount;
                        }
                    }
                    if (direct)
                        --relayCount;
                }
                unsigned int callbackValue = 0;
                if (mesh->m_hostCandidateRankingCallback != 0)
                    callbackValue = mesh->m_hostCandidateRankingCallback(static_cast<StationIndex>(index), false);
                unsigned int edmWeight = 0;
                if (mesh->m_processUpdateMeshJob->CheckEdmByStationIndex(static_cast<StationIndex>(index)))
                    edmWeight = cScoreWeightEdm;
                unsigned int routeWeight = cScoreWeightNoRelay;
                if (direct)
                    routeWeight = 0;
                scores[index] = edmWeight + routeWeight + (mesh->m_unknown54 - relayCount) * cScoreWeightRelayCount
                                + callbackValue * cScoreWeightCallback + index + 1;
            }
        }
    }
    unsigned int previous = 0;
    unsigned int best = cScoreUnset;
    unsigned int bestIndex;
    unsigned short ranked = 0;
    while (ranked < mesh->m_unknown54)
    {
        for (unsigned short index = 0; index < mesh->m_unknown54; ++index)
        {
            if (scores[index] > previous && scores[index] < best)
            {
                best = scores[index];
                bestIndex = index;
            }
        }
        if (best == cScoreUnset)
            break;
        ranking[ranked] = static_cast<unsigned char>(bestIndex);
        ++ranked;
        previous = best;
        best = cScoreUnset;
    }
    for (int fill = ranked; fill < mesh->m_unknown54; ++fill)
        ranking[fill] = cStationIndexInvalid;
    *meshValue = mesh->m_unknown80;
    *relayValue = relayManager != 0 ? relayManager->m_unknown1c : 0;
    return Result(0);
}

Result ProcessHostMigrationJob::CheckHostCandidateRanking(StationIndex first, StationIndex second, bool* firstIsHigher,
                                                          unsigned int* valueE0, unsigned int* valueE4) const
{
    if (m_unknownE0 == 0)
        return Result(cResultInvalidState);
    if (first == cStationIndexInvalid || second == cStationIndexInvalid || firstIsHigher == 0)
        return Result(cResultInvalidParameter);
    unsigned int firstPosition = cStationNumMax;
    unsigned int secondPosition = cStationNumMax;
    for (unsigned short index = 0; index < g_piaMesh->m_unknown54; ++index)
    {
        if (m_candidateRanking[index] == static_cast<unsigned char>(first))
            firstPosition = index;
        if (m_candidateRanking[index] == static_cast<unsigned char>(second))
            secondPosition = index;
    }
    if (firstPosition == cStationNumMax && secondPosition == cStationNumMax)
        return Result(cResultInvalidParameter);
    *firstIsHigher = static_cast<int>(firstPosition - secondPosition) < 0;
    *valueE0 = m_unknownE0;
    *valueE4 = m_unknownE4;
    return Result(0);
}

void ProcessHostMigrationJob::SetRankDecision(StationIndex station, bool value)
{
    if (station >= g_piaMesh->m_unknown54)
        return;
    m_rankDecision[station] = value ? cRankDecisionValue2 : cRankDecisionValue1;
}

void ProcessHostMigrationJob::InitializeMonitoringDataVar()
{
    m_elapsedTime.m_tick = 0;
    m_stationCountAtStart = cStationCountUnset;
    m_stationCountAtEnd = cStationCountUnset;
    m_hostMigrationCount = 0;
    m_unknown6e = 0;
}

void ProcessHostMigrationJob::SetMonitoringData()
{
    common::g_sessionStateMonitoringContent.m_metric185 = static_cast<unsigned short>(m_hostMigrationCount);
    const unsigned int elapsedMsec = static_cast<unsigned int>(m_elapsedTime.m_tick
                                                               / common::TimeSpan::GetTicksPerMSec().m_tick);
    common::g_sessionStateMonitoringContent.m_metric186 = elapsedMsec;
    if (elapsedMsec == 0)
        common::g_sessionStateMonitoringContent.m_metric186 = 0xffffffff;
    common::g_sessionStateMonitoringContent.m_metric187[0] = m_stationCountAtStart;
    common::g_sessionStateMonitoringContent.m_metric187[1] = m_stationCountAtEnd;
    common::g_sessionStateMonitoringContent.m_metric225[2] = m_unknown6e;
}

void ProcessHostMigrationJob::CleanupStatus()
{
    m_started = false;
    m_restartRequested = false;
    OSBlockSet(m_greetingPending, 0, sizeof(m_greetingPending));
    m_finishSuccess = false;
    m_unknownE4 = 0;
    m_newHostIndex = static_cast<StationIndex>(cStationIndexInvalid);
    m_greetingSenderPrincipalId = 0;
    m_oldHostIndex = static_cast<StationIndex>(cStationIndexInvalid);
    m_finishReceived = false;
    m_unknownA5 = false;
    m_greetingSent = false;
    m_greetingSenderIndex = static_cast<StationIndex>(cStationIndexInvalid);
    m_oldHostPrincipalId = 0;
    m_unknownE0 = 0;
    m_unknownB8 = false;
    OSBlockSet(m_rankDecision, 0, sizeof(m_rankDecision));
    m_hostMigrationSequence = 0;
}

void ProcessHostMigrationJob::DisconnectStation(StationIndex station)
{
    if (g_piaMesh->IsValidStationIndex(station))
    {
        g_piaMesh->UnfixDisconnectedId(station);
        transport::Station* target = transport::StationManager::s_pInstance->GetStation(station);
        if (target != 0)
        {
            target->m_field20 = cStationState6;
            target->SetStationIdAndIndex(0, static_cast<StationIndex>(cStationIndexInvalid));
        }
    }
}

StationIndex ProcessHostMigrationJob::GetStationIndexByPrincipalID(unsigned long long principalId)
{
    return g_piaMesh->m_processUpdateMeshJob->GetStationIndexByPrincipalID(principalId);
}

}
}
}
