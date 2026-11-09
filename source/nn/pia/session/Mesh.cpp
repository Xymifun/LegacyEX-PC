#include "nn/pia/session/Mesh.hpp"

#include "nn/pia/Transport.hpp"
#include "nn/pia/session/DestroyMeshJob.hpp"
#include "nn/pia/session/JoinMeshJob.hpp"
#include "nn/pia/session/KickoutManageJob.hpp"
#include "nn/pia/session/LeaveMeshJob.hpp"
#include "nn/pia/session/LeaveWithHostMigrationJob.hpp"
#include "nn/pia/session/MeshEventListener.hpp"
#include "nn/pia/session/CreateMeshJob.hpp"
#include "nn/pia/session/MeshProtocol.hpp"
#include "nn/pia/session/ProcessDestroyMeshJob.hpp"
#include "nn/pia/session/ProcessHostMigrationJob.hpp"
#include "nn/pia/session/ProcessJoinRequestJob.hpp"
#include "nn/pia/session/ProcessUpdateMeshJob.hpp"
#include "nn/pia/session/RelayRouteManageJob.hpp"
#include "nn/pia/session/SyncClockProtocol.hpp"
#include "nn/pia/transport/BandwidthCheckProtocol.hpp"
#include "nn/pia/transport/IdentificationInfoTable.hpp"
#include "nn/pia/transport/ProtocolEvent.hpp"
#include "nn/pia/transport/RelayRouteManager.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"
#include "nn/pia/transport/StationProtocol.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/common/SessionStateMonitoringContent.hpp"
#include "nn/pia/transport/ThreadStreamManager.hpp"
#include "nn/pia/transport/TransportAnalyzer.hpp"

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/session/SessionNetworkFactory.hpp"
#include "nn/pia/session/SessionSupport.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/transport/StationProtocolManager.hpp"
#include "platform/wiiu/pead.hpp"
#include <nn/act/client_cpp.h>
#include <nn/fp/fp_cpp.h>
#include <coreinit/memory.h>
#include <new>

namespace nn
{
namespace pia
{
namespace session
{

Mesh* g_piaMesh = 0;

namespace
{

Mesh::DebugSetting sDebugSetting;
unsigned char* sMeshStorage = 0;


template <typename T>
T* allocateHeapObject()
{
    void* storage = pead::allocateFromSeadHeap(sizeof(T), common::HeapManager::GetHeap());
    if (storage == 0)
        return 0;
    return new (storage) T();
}

const long long cTimeTransportUnavailable = -3;
const long long cTimeSyncClockUnavailable = -2;

enum
{
    cMonitoringReasonKickout = 4,
    cMonitoringReasonKickoutRequested = 5,
    cDisconnectReasonNormal = 1,
    cKickoutReasonRequested = 2,
    cMonitoringUnset8 = 0xff,
    cMonitoringSessionBeginReport = 1,
    cMonitoringBeginDataType = 0,
    cMiiNameBufferSize = 11,
    cBandwidthCheckProtocolPort = 2,
    cNetworkTopologyRelay = 1,
    cRelayRttMax = 500,
    cHostMigrationModeSingle = 1,
    cHostMigrationModeMulti = 2,
    cKeepAliveIntervalMsec = 1000,
    cStationNumMax = 32,
    cMonitoringBeginIntervalCount = 15,
    cMonitoringBeginStatCount = 8,
    cResultInvalidArgument = 0x10c07,
    cResultInvalidState = 0x10c08,
    cResultAlreadyInitialized = 0x10c0f,
    cResultNotInitialized = 0x10c0c,
    cResultOutOfMemory = 0x10c01,
    cStationTimeoutMsecMin = 1000,
    cStationTimeoutMsecMax = 30000,
    cJoinRequestTimeoutMsec = 10000,
    cStationProtocolTimeoutMsec = 10000,
    cUpdateMeshExtraTimeMsec = 5000,
    cIdentificationInfoPlayersCounted = 3,
    cSyncClockProtocolType = 0x1c,
    cDisconnectReasonDestroyed = 0x10c08,
    cStationUnused = 0xfd,
    cMeshAlignment = 8,
    cProtocolEventConnect = 0,
    cProtocolEventDisconnect = 1,
    cMeshEventMeshStateCreated = 0x11,
    cMeshEventMeshStateChanged = 0x12,
    cMeshEventJoinRequestParticipants = 0x18,
    cMeshEventJoinRequestPrincipal = 0x19,
    cJoinDenyReasonSessionFull = 0,
    cJoinDenyReasonNotAccepting = 2,
    cJoinDenyReasonInvalidState = 4,
    cJoinApproved = 0xff,
    cRecentPlayNameLengthMax = 20,
    cRecentPlayNameCopyMax = nn::act::AccountIdSize - 1,
    cRecentPlayRecordCount = 1,
    cLanguageByteBits = 8
};

const unsigned int cMonitoringUnset32 = 0xffffffffu;

}

Mesh::Mesh(const Setting&)
    : m_createMeshJob(0),
      m_joinMeshJob(0),
      m_leaveMeshJob(0),
      m_unknown0c(0),
      m_processUpdateMeshJob(0),
      m_destroyMeshJob(0),
      m_processDestroyMeshJob(0),
      m_processHostMigrationJob(0),
      m_leaveWithHostMigrationJob(0),
      m_unknown24(0),
      m_kickoutManageJob(0),
      m_unknown2c(0),
      m_meshEventCallback(0),
      m_joiningMeshApprovalCallback(0),
      m_hostCandidateRankingCallback(0),
      m_getSessionIdCallback(0),
      m_meshProtocolId(0),
      m_meshProtocol(0),
      m_stationProtocol(0),
      m_bandwidthCheckProtocolId(0),
      m_bandwidthCheckProtocol(0),
      m_unknown54(0),
      m_unknown56(1),
      m_unknown58(static_cast<StationIndex>(0xfd)),
      m_unknown5c(static_cast<StationIndex>(0xfd)),
      m_meshState(0),
      m_disconnectReason(0),
      m_hostMigrationMode(0),
      m_bitRateCheckMode(0),
      m_hostMigrationEnabled(1),
      m_unknown75(0),
      m_recentPlayRecordEnabled(0),
      m_multiplayerPermitted(0),
      m_connectedStationBitmap(0),
      m_validStationBitmap(0),
      m_unknown80(0xffffffffu),
      m_unknown88(0),
      m_syncClockProtocolId(0),
      m_meshStartTime(0),
      m_meshStartTimeValid(0),
      m_startedUp(0),
      m_hostMigrationStarted(0),
      m_recentPlayRecordRequested(1),
      m_monitoringDataSender(0),
      m_asyncProcess(0),
      m_meshEventListener(0),
      m_skipReport(0),
      m_skipCleanupReport(0),
      m_playersCountedAsParticipants(1),
      m_localPlayerCount(1),
      m_unknownc8(0xffffffffu),
      m_unknowncc(0),
      m_denyJoinRequestByInvalidState(0),
      m_denyJoinRequestBySessionIsFull(0)
{
}

Mesh::~Mesh()
{
    transport::Transport* transport = transport::g_piaTransport;
    SessionNetworkFactory* factory = static_cast<SessionNetworkFactory*>(transport->GetNetworkFactory());
    if (m_createMeshJob != 0)
    {
        factory->DeleteCreateMeshJob(m_createMeshJob);
        m_createMeshJob = 0;
    }
    if (m_joinMeshJob != 0)
    {
        factory->DeleteJoinMeshJob(m_joinMeshJob);
        m_joinMeshJob = 0;
    }
    if (m_leaveMeshJob != 0)
    {
        factory->DeleteLeaveMeshJob(m_leaveMeshJob);
        m_leaveMeshJob = 0;
    }
    if (m_unknown0c != 0)
    {
        m_unknown0c->~ProcessJoinRequestJob();
        pead::freeToSeadHeap(m_unknown0c);
        m_unknown0c = 0;
    }
    if (m_processUpdateMeshJob != 0)
    {
        m_processUpdateMeshJob->~ProcessUpdateMeshJob();
        pead::freeToSeadHeap(m_processUpdateMeshJob);
        m_processUpdateMeshJob = 0;
    }
    if (m_destroyMeshJob != 0)
    {
        m_destroyMeshJob->~DestroyMeshJob();
        pead::freeToSeadHeap(m_destroyMeshJob);
        m_destroyMeshJob = 0;
    }
    if (m_processDestroyMeshJob != 0)
    {
        m_processDestroyMeshJob->~ProcessDestroyMeshJob();
        pead::freeToSeadHeap(m_processDestroyMeshJob);
        m_processDestroyMeshJob = 0;
    }
    if (m_processHostMigrationJob != 0)
    {
        factory->DeleteProcessHostMigrationJob(m_processHostMigrationJob);
        m_processHostMigrationJob = 0;
    }
    if (m_leaveWithHostMigrationJob != 0)
    {
        factory->DeleteLeaveWithHostMigrationJob(m_leaveWithHostMigrationJob);
        m_leaveWithHostMigrationJob = 0;
    }
    if (m_unknown24 != 0)
    {
        m_unknown24->~RelayRouteManageJob();
        pead::freeToSeadHeap(m_unknown24);
        m_unknown24 = 0;
    }

    transport::RelayRouteManager* relayRouteManager = transport::g_piaTransport->m_relayRouteManager;
    if (relayRouteManager != 0)
    {
        transport::g_piaTransport->m_relayRouteManager = 0;
        relayRouteManager->Finalize();
        relayRouteManager->~RelayRouteManager();
        pead::freeToSeadHeap(relayRouteManager);
    }
    if (m_kickoutManageJob != 0)
    {
        factory->DeleteKickoutManageJob(m_kickoutManageJob);
        m_kickoutManageJob = 0;
    }
    if (m_unknown2c != 0)
    {
        factory->DeleteMissingStationHandler(m_unknown2c);
        m_unknown2c = 0;
    }
    if (m_syncClockProtocolId != 0)
    {
        transport::g_piaTransport->m_protocolManager.DestroyProtocol(m_syncClockProtocolId);
        m_syncClockProtocolId = 0;
    }
    if (m_bitRateCheckMode != 0 && m_bandwidthCheckProtocolId != 0)
    {
        m_bandwidthCheckProtocol->Finalize();
        transport::g_piaTransport->m_protocolManager.DestroyProtocol(m_bandwidthCheckProtocolId);
        m_bandwidthCheckProtocol = 0;
        m_bandwidthCheckProtocolId = 0;
    }
    if (m_meshProtocol != 0)
        m_meshProtocol->Finalize();
    if (transport::g_piaTransport != 0)
    {
        transport::g_piaTransport->m_protocolManager.DestroyProtocol(m_meshProtocolId);
        transport::g_piaTransport->m_protocolManager.CleanupProtocolsWithStationAddress();
        transport::g_piaTransport->SetTransportStatus(Result(cResultInvalidState));
    }
    m_meshProtocolId = 0;
    m_stationProtocol = 0;
    m_meshProtocol = 0;
    if (m_bandwidthCheckProtocolId == 0)
    {
        m_bandwidthCheckProtocolId = 0;
        m_bandwidthCheckProtocol = 0;
    }
    if (m_monitoringDataSender != 0)
    {
        factory->DeleteMonitoringDataSender(m_monitoringDataSender);
        m_monitoringDataSender = 0;
    }
    m_unknown75 = 0;
}

Result Mesh::Initialize(const Setting& setting, bool relayEmulationEnabled)
{
    transport::Transport* transport = transport::g_piaTransport;
    SessionNetworkFactory* factory = setting.m_networkFactory;
    m_meshProtocolId = transport->m_protocolManager.CreateProtocol<MeshProtocol>(0);
    m_stationProtocol = transport::g_piaStationProtocolManager->GetStationProtocol();
    m_meshProtocol = transport->m_protocolManager.GetProtocol<MeshProtocol>(m_meshProtocolId);
    m_meshProtocol->Initialize();

    m_syncClockProtocolId = transport->m_protocolManager.CreateProtocol<SyncClockProtocol>(0);
    m_bitRateCheckMode = setting.m_bitRateCheckMode;
    common::g_sessionBeginMonitoringContent.m_metric132 = static_cast<unsigned char>(m_bitRateCheckMode);

    if (m_bitRateCheckMode != 0)
    {
        m_bandwidthCheckProtocolId = transport->m_protocolManager.CreateProtocol<transport::BandwidthCheckProtocol>(
            cBandwidthCheckProtocolPort);
        m_bandwidthCheckProtocol = transport->m_protocolManager.GetProtocol<transport::BandwidthCheckProtocol>(
            m_bandwidthCheckProtocolId);
        m_bandwidthCheckProtocol->Initialize();
    }

    if (transport->m_stationNum <= cStationNumMax)
        m_unknown54 = transport->m_stationNum;
    else
        m_unknown54 = cStationNumMax;
    m_unknown75 = factory->IsServerRequired();

    if (m_createMeshJob == 0)
        m_createMeshJob = static_cast<CreateMeshJob*>(factory->CreateCreateMeshJob());
    if (m_joinMeshJob == 0)
        m_joinMeshJob = static_cast<JoinMeshJob*>(factory->CreateJoinMeshJob());
    if (m_leaveMeshJob == 0)
        m_leaveMeshJob = static_cast<LeaveMeshJob*>(factory->CreateLeaveMeshJob());
    if (m_unknown0c == 0)
        m_unknown0c = allocateHeapObject<ProcessJoinRequestJob>();
    if (m_processUpdateMeshJob == 0)
        m_processUpdateMeshJob = allocateHeapObject<ProcessUpdateMeshJob>();
    if (m_destroyMeshJob == 0)
        m_destroyMeshJob = allocateHeapObject<DestroyMeshJob>();
    if (m_processDestroyMeshJob == 0)
        m_processDestroyMeshJob = allocateHeapObject<ProcessDestroyMeshJob>();

    m_networkTopologyType = setting.m_networkTopologyType;
    if (m_networkTopologyType == cNetworkTopologyRelay)
    {
        transport::RelayRouteManager* relayRouteManager = transport->m_relayRouteManager;
        if (relayRouteManager == 0)
        {
            relayRouteManager = allocateHeapObject<transport::RelayRouteManager>();
            if (relayRouteManager != 0)
                relayRouteManager->Initialize(m_unknown54, relayEmulationEnabled);
            transport->m_relayRouteManager = relayRouteManager;
        }
        relayRouteManager->SetRelayNumMax(m_unknown54 * m_unknown54);
        relayRouteManager->SetRttMax(cRelayRttMax);
        m_unknown24 = allocateHeapObject<RelayRouteManageJob>();
    }
    else
    {
        transport->m_relayRouteManager = 0;
        m_unknown24 = 0;
    }
    m_hostMigrationMode = factory->GetHostMigrationMode();

    if (m_hostMigrationMode == cHostMigrationModeSingle || m_hostMigrationMode == cHostMigrationModeMulti)
    {
        m_processHostMigrationJob = static_cast<ProcessHostMigrationJob*>(factory->CreateProcessHostMigrationJob());
        m_leaveWithHostMigrationJob = static_cast<LeaveWithHostMigrationJob*>(
            factory->CreateLeaveWithHostMigrationJob());
    }
    else
    {
        m_processHostMigrationJob = 0;
        m_leaveWithHostMigrationJob = 0;
    }
    if (m_kickoutManageJob == 0)
        m_kickoutManageJob = static_cast<KickoutManageJob*>(factory->CreateKickoutManageJob());
    if (m_unknown2c == 0)
        m_unknown2c = factory->CreateMissingStationHandler();

    transport->SetKeepAliveInterval(cKeepAliveIntervalMsec);
    if (m_monitoringDataSender == 0)
        m_monitoringDataSender = factory->CreateMonitoringDataSender();
    transport->SetTransportStatus(Result(0));
    m_recentPlayRecordEnabled = factory->IsRecentPlayRecordPermitted();
    m_multiplayerPermitted = factory->IsMultiplayerPermitted();
    return 0;
}

Result Mesh::CreateInstance(const Setting& setting)
{
    if (!IsInitialized())
        return static_cast<Result>(cResultNotInitialized);
    if (!IsDuringSetup())
        return static_cast<Result>(cResultInvalidState);
    if (transport::g_piaTransport == 0)
        return static_cast<Result>(cResultInvalidState);
    if (setting.m_networkFactory == 0)
        return static_cast<Result>(cResultInvalidArgument);
    if (setting.m_networkTopologyType != 0 && setting.m_networkTopologyType != cNetworkTopologyRelay)
        return static_cast<Result>(cResultInvalidArgument);

    bool topologyAllowed = true;
    if (setting.m_networkTopologyType != cNetworkTopologyRelay && sDebugSetting.m_relayEmulationEnabled != 0)
        topologyAllowed = false;
    const bool relayPermitted = setting.m_networkFactory->IsClientRelayConnectionPermitted();
    if ((!relayPermitted && setting.m_networkTopologyType != 0) || !topologyAllowed)
        return static_cast<Result>(cResultInvalidArgument);

    if (g_piaMesh == 0)
    {
        sMeshStorage = common::NewArray<unsigned char>(sizeof(Mesh), cMeshAlignment);
        if (sMeshStorage == 0)
            return static_cast<Result>(cResultOutOfMemory);

        g_piaMesh = new (sMeshStorage) Mesh(setting);
        if (g_piaMesh == 0)
        {
            common::DeleteArray<unsigned char>(sMeshStorage);
            sMeshStorage = 0;
            return static_cast<Result>(cResultOutOfMemory);
        }

        const Result result = g_piaMesh->Initialize(setting, sDebugSetting.m_relayEmulationEnabled != 0);
        if (result != 0)
        {
            g_piaMesh->~Mesh();
            g_piaMesh = 0;
            common::DeleteArray<unsigned char>(sMeshStorage);
            sMeshStorage = 0;
            return result;
        }

        common::g_sessionBeginMonitoringContent.m_metric45 = static_cast<unsigned char>(setting.m_networkTopologyType);
        return 0;
    }
    return static_cast<Result>(cResultAlreadyInitialized);
}

void Mesh::DestroyInstance()
{
    if (g_piaMesh != 0)
    {
        g_piaMesh->Cleanup();
        g_piaMesh->~Mesh();
        g_piaMesh = 0;
    }
    if (sMeshStorage != 0)
    {
        common::DeleteArray<unsigned char>(sMeshStorage);
        sMeshStorage = 0;
    }
}

void Mesh::CleanupStationsJobs()
{
    transport::Station** station = transport::StationManager::s_pInstance->m_stations.data;
    transport::Station** const end = station + transport::StationManager::s_pInstance->m_stations.size;
    for (; station != end; ++station)
        (*station)->CleanupJobs();
}

void Mesh::CleanupJobs()
{
    m_createMeshJob->Cleanup();
    m_createMeshJob->Reset(false);
    m_joinMeshJob->Cleanup(Result(0));
    m_joinMeshJob->Reset(false);
    m_leaveMeshJob->Cleanup();
    m_leaveMeshJob->Reset(false);
    m_unknown0c->Cleanup();
    m_unknown0c->Reset(false);
    m_processUpdateMeshJob->Cleanup();
    m_processUpdateMeshJob->Reset(false);
    m_destroyMeshJob->Cleanup();
    m_destroyMeshJob->Reset(false);
    m_processDestroyMeshJob->Cleanup();
    m_processDestroyMeshJob->Reset(false);
    if (m_processHostMigrationJob != 0)
    {
        m_processHostMigrationJob->Cleanup();
        m_processHostMigrationJob->Reset(false);
    }
    if (m_leaveWithHostMigrationJob != 0)
    {
        m_leaveWithHostMigrationJob->Cleanup();
        m_leaveWithHostMigrationJob->Reset(false);
    }
    if (m_unknown24 != 0)
    {
        m_unknown24->Cleanup();
        m_unknown24->Reset(false);
    }
    m_kickoutManageJob->Cleanup();
    m_kickoutManageJob->Reset(false);
    CleanupStationsJobs();
    m_callContext.Reset();
    m_asyncProcess = 0;
}

bool Mesh::IsValidStationIndex(StationIndex index) const
{
    if (static_cast<int>(index) > 31)
        return false;
    unsigned int mask = 1;
    for (unsigned short shift = 0; shift < static_cast<unsigned short>(index); ++shift)
        mask <<= 1;
    return (m_validStationBitmap & mask) != 0;
}

void Mesh::StopUse(StationIndex index)
{
    if (static_cast<int>(index) > 31)
        return;
    unsigned int mask = 1;
    for (unsigned short shift = 0; shift < static_cast<unsigned short>(index); ++shift)
        mask <<= 1;
    m_validStationBitmap &= ~mask;
    if (m_unknown24 != 0)
        m_unknown24->m_stationBitmap &= ~mask;
    m_connectedStationBitmap &= ~mask;
    transport::g_piaTransport->m_stationIdTable->SetJoinEventNotifiedByStationIndex(index, false);
}

void Mesh::UnfixDisconnectedId(StationIndex index)
{
    transport::Station* station = transport::StationManager::s_pInstance->GetStation(index);
    Result result;
    {
        transport::ProtocolEvent event = {cProtocolEventDisconnect, station->GetStationAddress()};
        result = transport::g_piaTransport->m_protocolManager.UpdateProtocolEvent(event);
    }
    if (result == 0)
    {
        m_kickoutManageJob->SetLeaveEventStationIndex(index);
        NoticeMeshEvent(cEventTypeLeave, index);
        StopUse(index);
    }
}

void Mesh::ResetStationIndexUsage()
{
    m_connectedStationBitmap = 0;
    m_validStationBitmap = 0;
}

void Mesh::SetMeshState(State state)
{
    if (m_meshState == static_cast<unsigned int>(state))
        return;
    m_meshState = static_cast<unsigned int>(state);
    if (m_meshEventListener != 0)
    {
        MeshEventListener::MeshEvent event;
        if (state == cStateCreated)
            event.type = cMeshEventMeshStateCreated;
        else
            event.type = cMeshEventMeshStateChanged;
        event.stationIndex = cStationIndexInvalid;
        event.stationId = 0;
        m_meshEventListener->NoticeEvent(event);
    }
}

void Mesh::CleanupStatus()
{
    for (int index = 0; index < cStationNumMax; ++index)
    {
        if (index != m_unknown5c && IsValidStationIndex(static_cast<StationIndex>(index)))
            UnfixDisconnectedId(static_cast<StationIndex>(index));
    }
    ResetStationIndexUsage();
    transport::g_piaTransport->m_protocolManager.CleanupProtocolsWithStationAddress();
    transport::g_stationConnectionInfoTable->ClearTable();
    transport::IdentificationInfoTable::GetInstance()->ClearTable();
    transport::IdentificationInfoTable::GetInstance()->ClearLocalIdentificationInfo();
    m_localPlayerCount = 1;
    m_playersCountedAsParticipants = true;

    transport::StationManager* stationManager = transport::StationManager::s_pInstance;
    while (stationManager->m_stations.size != 0)
    {
        transport::Station* station = stationManager->m_stations.data[0];
        station->Cleanup();
        station->CleanupJobs();
        stationManager->DestroyStation(station);
    }
    transport::g_piaTransport->OutputStreamUpdateEvent();

    m_unknowncc = 0;
    m_unknown88 = 0;
    m_unknownc8 = 0xffffffffu;
    m_unknown80 = 0xffffffffu;
    SetMeshState(cStateNone);
    if (static_cast<int>(m_unknown5c) < cStationNumMax)
        NoticeMeshEvent(cEventTypeLeave, m_unknown5c);

    const transport::ProtocolId syncClockProtocolId = m_syncClockProtocolId;
    m_unknown5c = static_cast<StationIndex>(cStationUnused);
    m_unknown58 = static_cast<StationIndex>(cStationUnused);
    SyncClockProtocol* syncClockProtocol = static_cast<SyncClockProtocol*>(
        transport::g_piaTransport->m_protocolManager.SearchProtocol(syncClockProtocolId, cSyncClockProtocolType));
    if (syncClockProtocol != 0)
        syncClockProtocol->m_syncClock.ClearBaseTime();

    m_hostMigrationEnabled = true;
    if (!m_skipCleanupReport)
    {
        common::g_sessionBeginMonitoringContent.Cleanup();
        common::g_sessionStateMonitoringContent.Cleanup();
        if (m_monitoringDataSender != 0)
            m_monitoringDataSender->Cleanup();
    }
    m_denyJoinRequestByInvalidState = 0;
    m_skipReport = false;
    m_denyJoinRequestBySessionIsFull = 0;
}

void Mesh::Cleanup()
{
    m_playerInfo[0].m_name.Clear();
    m_playerInfo[0].m_language = transport::Station::PlayerInfo::cLanguageUnset;
    m_playerInfo[0].m_field70 = 0;
    m_playerInfo[0].m_field78 = 0;
    if (m_startedUp)
    {
        if (m_bitRateCheckMode != 0)
            m_bandwidthCheckProtocol->Inactivate();
        CleanupJobs();
        CleanupStatus();
        transport::g_piaTransport->m_connectionRouteRelay = true;
        m_startedUp = false;
    }
}

Result Mesh::CheckPlayerInfo(const transport::Station::PlayerInfo& info) const
{
    if (info.m_name.GetStringLength() == 0 && info.m_name.GetEncodingType() == 0
        && info.m_language == transport::Station::PlayerInfo::cLanguageUnset)
        return static_cast<Result>(0x10c07);
    if (info.m_name.GetStringLength() >= 21)
        return static_cast<Result>(0x10c07);
    if (info.m_name.GetStringLength() != 0)
    {
        if (info.m_name.GetEncodingType() != common::String::EncodingType_Utf16)
            return static_cast<Result>(0x10c07);
        if (info.m_language > 11)
            return static_cast<Result>(0x10c07);
    }
    return 0;
}

Result Mesh::CheckPlayerInfoSetting(const transport::Station::PlayerInfo* infos, unsigned char count) const
{
    if (infos == 0)
        return static_cast<Result>(0x10c07);
    if (count == 0)
        return static_cast<Result>(0x10c07);
    if (count < 2)
    {
        for (int i = 0; i < count; ++i)
        {
            if (CheckPlayerInfo(infos[i]) != 0)
                return static_cast<Result>(0x10c07);
        }
        return 0;
    }
    return static_cast<Result>(0x10c07);
}

Result Mesh::Startup(StartupSetting& setting)
{
    if (IsDuringSetup())
        return static_cast<Result>(cResultInvalidState);
    if (m_startedUp)
        return static_cast<Result>(cResultInvalidState);
    transport::Transport* transport = transport::g_piaTransport;
    if (transport == 0)
        return static_cast<Result>(cResultInvalidState);
    if (transport::IdentificationInfoTable::GetInstance() == 0)
        return static_cast<Result>(cResultInvalidState);
    if (sDebugSetting.m_checkStartupSettingRange != 0
        && setting.m_field00 - cStationTimeoutMsecMin > cStationTimeoutMsecMax - cStationTimeoutMsecMin)
        return static_cast<Result>(cResultInvalidArgument);

    if (m_bitRateCheckMode != 0)
    {
        const Result result = m_bandwidthCheckProtocol->Activate(
            setting.m_bandwidthCheckMeasurementTime, setting.m_bandwidthCheckIpPacketSize, setting.m_skipBandwidthCheck,
            setting.m_bandwidthCheckMeasurementCount);
        if (result != 0)
            return result;
    }

    m_recentPlayRecordRequested = setting.m_recentPlayRecordEnabled;
    if (m_recentPlayRecordRequested && m_recentPlayRecordEnabled)
    {
        const Result result = CheckPlayerInfoSetting(setting.m_playerInfo, setting.m_playerInfoSize);
        if (result != 0)
            return result;
    }

    if (!setting.m_hostMigrationEnabled)
    {
        common::g_sessionBeginMonitoringContent.m_metric46 = 0;
        m_hostMigrationEnabled = setting.m_hostMigrationEnabled;
    }
    else
    {
        if (m_processHostMigrationJob != 0 && !m_processHostMigrationJob->IsValidHostMigrationSetting())
            return static_cast<Result>(cResultInvalidState);
        common::g_sessionBeginMonitoringContent.m_metric46 = static_cast<unsigned char>(m_hostMigrationMode);
        m_hostMigrationEnabled = setting.m_hostMigrationEnabled;
    }

    transport::IdentificationInfo info;
    if (setting.m_identificationToken != 0)
        OSBlockMove(&info.m_field00, setting.m_identificationToken, sizeof(transport::Station::IdentificationToken), 0);
    else
        OSBlockSet(&info.m_field00, 0, sizeof(transport::Station::IdentificationToken));

    if (!m_multiplayerPermitted && setting.m_playerInfoSize > 1)
        return static_cast<Result>(cResultInvalidArgument);

    info.m_field60[0].Clear();
    info.m_fieldC8[0].Clear();
    info.m_field108 = transport::Station::PlayerInfo::cLanguageUnset;
    OSBlockSet(&info.m_field20, 0, sizeof(info.m_field20));
    info.m_principalId[0] = 0;

    m_playerInfo[0].m_name.Clear();
    m_playerInfo[0].m_language = transport::Station::PlayerInfo::cLanguageUnset;
    m_playerInfo[0].m_field70 = 0;
    m_playerInfo[0].m_field78 = 0;
    m_playerInfoCount = 0;
    if (setting.m_playerInfo != 0)
    {
        const transport::Station::PlayerInfo* playerInfo = setting.m_playerInfo;
        const unsigned long long* principalIds = setting.m_field14;
        for (unsigned char index = 0; index < setting.m_playerInfoSize; ++index)
        {
            if (CheckPlayerInfo(*playerInfo) == 0)
            {
                m_playerInfo[m_playerInfoCount].m_name.Copy(playerInfo->m_name);
                m_playerInfo[m_playerInfoCount].m_language = playerInfo->m_language;
                m_playerInfo[m_playerInfoCount].m_field70 = playerInfo->m_field70;
                m_playerInfo[m_playerInfoCount].m_field78 = playerInfo->m_field78;
                if (principalIds != 0)
                    info.m_principalId[m_playerInfoCount] = *principalIds;
                ++m_playerInfoCount;
            }
            ++playerInfo;
            ++principalIds;
        }
    }
    info.m_field109 = cIdentificationInfoPlayersCounted;
    transport::IdentificationInfoTable::GetInstance()->SetLocalIdentificationInfo(&info);

    m_localPlayerCount = setting.m_playerInfoSize;
    m_playersCountedAsParticipants = setting.m_playersCountedAsParticipants;
    m_unknown0c->m_timeoutMsec = cJoinRequestTimeoutMsec;
    m_processUpdateMeshJob->m_updateExtraTimeMsec = setting.m_field00 + cUpdateMeshExtraTimeMsec;

    unsigned int stationTimeout = setting.m_field00;
    if (sDebugSetting.m_checkStartupSettingRange != 0)
    {
        if (stationTimeout < cStationTimeoutMsecMin)
            stationTimeout = cStationTimeoutMsecMin;
        else if (stationTimeout > cStationTimeoutMsecMax)
            stationTimeout = cStationTimeoutMsecMax;
    }
    m_meshProtocol->m_unknown24 = stationTimeout;
    m_meshProtocol->m_unknown28 = static_cast<int>(stationTimeout) / 2;
    static_cast<transport::StationProtocol*>(m_stationProtocol)->m_timeoutMsec = cStationProtocolTimeoutMsec;
    transport->SetKeepAliveInterval(setting.m_field04);
    common::g_sessionBeginMonitoringContent.m_metric43 = static_cast<unsigned short>(setting.m_field00);
    common::g_sessionBeginMonitoringContent.m_metric44 = setting.m_field04;
    m_kickoutManageJob->Startup();
    if (transport->m_relayRouteManager != 0)
        transport->m_relayRouteManager->InitRelayRouteManagerData();

    Result result;
    if (!transport->m_connectionRouteRelay)
    {
        result = transport->UpdateSetting(setting.m_cryptoSetting);
    }
    else
    {
        result = transport->Restartup(0);
        if (result != 0)
            return result;
        result = transport->UpdateSetting(setting.m_cryptoSetting);
    }
    if (result != 0)
        return result;

    m_meshStartTimeValid = false;
    m_meshStartTime = 0;
    m_processUpdateMeshJob->InitializeMonitoringDataVar();
    if (m_processHostMigrationJob != 0)
        m_processHostMigrationJob->InitializeMonitoringDataVar();
    m_startedUp = true;
    m_disconnectReason = cDisconnectReasonNormal;
    return 0;
}

Result Mesh::CreateMesh(common::CallContext* callerContext)
{
    if (callerContext != 0 && callerContext->m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(cResultInvalidState);
    if (m_meshState == cStateCreated)
        return static_cast<Result>(cResultInvalidState);
    if (transport::StationManager::s_pInstance->m_localStation != 0)
        return static_cast<Result>(cResultInvalidState);
    if (m_createMeshJob->GetState() != common::Job::cStateNotEntried
        && m_createMeshJob->GetState() != common::Job::cStateCompleted)
        return static_cast<Result>(cResultInvalidState);
    const Result result = m_createMeshJob->Startup(callerContext);
    if (result == 0)
        m_createMeshJob->Ready(true);
    return result;
}

Result Mesh::CreateMeshAsync()
{
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(cResultInvalidState);
    ResetInternalAsyncState();
    const Result result = CreateMesh(&m_callContext);
    if (result == 0)
        m_asyncProcess = cAsyncCreateMesh;
    return result;
}

bool Mesh::IsAsyncProcessCompleted(AsyncProcessingName process)
{
    if (m_asyncProcess == static_cast<unsigned int>(process))
        return m_callContext.m_state - 2u < 3u;
    return false;
}

bool Mesh::IsCreateMeshCompleted()
{
    return IsAsyncProcessCompleted(cAsyncCreateMesh);
}

Result Mesh::GetAsyncProcessResult(AsyncProcessingName process)
{
    if (m_asyncProcess == static_cast<unsigned int>(process) && m_callContext.m_state - 2u < 3u)
        return m_callContext.m_result;
    return static_cast<Result>(cResultInvalidState);
}

Result Mesh::GetCreateMeshResult()
{
    return GetAsyncProcessResult(cAsyncCreateMesh);
}

Result Mesh::CancelAsyncProcess(AsyncProcessingName process)
{
    if (m_asyncProcess != static_cast<unsigned int>(process)
        || m_callContext.m_state != common::CallContext::cStateInProgress)
        return static_cast<Result>(cResultInvalidState);
    m_callContext.Cancel();
    return 0;
}

unsigned int Mesh::GetHostMigrationMode() const
{
    return m_hostMigrationMode;
}

bool Mesh::IsRecentPlayRecordEnabled() const
{
    return m_recentPlayRecordEnabled != 0;
}

KickoutManageJob* Mesh::GetKickoutManageJobPtr()
{
    return m_kickoutManageJob;
}

void Mesh::SetSyncClockPulseInterval(int interval)
{
    if (transport::g_piaTransport == 0)
        return;

    transport::Protocol* protocol = transport::g_piaTransport->m_protocolManager.SearchProtocol(
        m_syncClockProtocolId, SyncClockProtocol::cProtocolType);
    if (protocol != 0)
        static_cast<SyncClockProtocol*>(protocol)->m_pulseInterval = interval;
}

void Mesh::SetSkipCleanupReport(bool skip)
{
    m_skipCleanupReport = skip;
}

Result Mesh::JoinMeshAsync(const transport::StationConnectionInfo& info)
{
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(cResultInvalidState);
    ResetInternalAsyncState();
    const Result result = JoinMeshCore(info, &m_callContext);
    if (result == 0)
        m_asyncProcess = cAsyncJoinMesh;
    return result;
}

void Mesh::ResetInternalAsyncState()
{
    if (m_asyncProcess != 0 && m_callContext.m_state != common::CallContext::cStateInProgress)
    {
        m_callContext.Reset();
        m_asyncProcess = 0;
    }
}

bool Mesh::IsJoinMeshCompleted()
{
    return IsAsyncProcessCompleted(cAsyncJoinMesh);
}

Result Mesh::GetJoinMeshResult()
{
    return GetAsyncProcessResult(cAsyncJoinMesh);
}

Result Mesh::CancelJoinMeshAsync()
{
    return CancelAsyncProcess(cAsyncJoinMesh);
}

Result Mesh::LeaveMeshAsync()
{
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(cResultInvalidState);
    ResetInternalAsyncState();
    const Result result = LeaveMesh(&m_callContext);
    if (result == 0)
        m_asyncProcess = cAsyncLeaveMesh;
    return result;
}

bool Mesh::IsLeaveMeshCompleted()
{
    return IsAsyncProcessCompleted(cAsyncLeaveMesh);
}

Result Mesh::GetLeaveMeshResult()
{
    return GetAsyncProcessResult(cAsyncLeaveMesh);
}

Result Mesh::LeaveMeshWithHostMigrationAsync()
{
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(0x10c07);
    ResetInternalAsyncState();
    const Result result = LeaveMeshWithHostMigration(&m_callContext);
    if (result == 0)
        m_asyncProcess = cAsyncLeaveMeshWithHostMigration;
    return result;
}

bool Mesh::IsLeaveMeshWithHostMigrationCompleted()
{
    return IsAsyncProcessCompleted(cAsyncLeaveMeshWithHostMigration);
}

Result Mesh::GetLeaveMeshWithHostMigrationResult()
{
    return GetAsyncProcessResult(cAsyncLeaveMeshWithHostMigration);
}

Result Mesh::DestroyMeshAsync()
{
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(0x10c07);
    ResetInternalAsyncState();
    const Result result = DestroyMesh(&m_callContext);
    if (result == 0)
        m_asyncProcess = cAsyncDestroyMesh;
    return result;
}

bool Mesh::IsDestroyMeshCompleted()
{
    return IsAsyncProcessCompleted(cAsyncDestroyMesh);
}

Result Mesh::GetDestroyMeshResult()
{
    return GetAsyncProcessResult(cAsyncDestroyMesh);
}

int Mesh::GetJoinMeshState() const
{
    if (m_joinMeshJob == 0)
        return 0;
    return m_joinMeshJob->GetJoinMeshState();
}

StationIndex Mesh::GetHostStationIndex() const
{
    return m_unknown58;
}

StationIndex Mesh::GetLocalStationIndex() const
{
    return m_unknown5c;
}

transport::BandwidthCheckProtocol* Mesh::GetBandwidthCheckProtocol()
{
    return m_bandwidthCheckProtocol;
}

void Mesh::RegisterMeshEventCallback(MeshEventCallback callback)
{
    m_meshEventCallback = callback;
}

void Mesh::UnregisterMeshEventCallback()
{
    m_meshEventCallback = 0;
}

void Mesh::RegisterJoiningMeshApprovalCallback(JoiningMeshApprovalCallback callback)
{
    m_joiningMeshApprovalCallback = callback;
}

void Mesh::UnregisterJoiningMeshApprovalCallback()
{
    m_joiningMeshApprovalCallback = 0;
}

Result Mesh::RegisterCheckHostCandidateRankingCallback(HostCandidateRankingCallback callback)
{
    if (m_hostMigrationMode != 2)
        return static_cast<Result>(cResultInvalidState);
    m_hostCandidateRankingCallback = callback;
    return 0;
}

void Mesh::UnregisterCheckHostCandidateRankingCallback()
{
    m_hostCandidateRankingCallback = 0;
}

void Mesh::RegisterEventListener(MeshEventListener* listener)
{
    m_meshEventListener = listener;
}

void Mesh::UnregisterEventListener()
{
    m_meshEventListener = 0;
}

void Mesh::RegisterGetSessionIdCallback(GetSessionIdCallback callback)
{
    m_getSessionIdCallback = callback;
}

void Mesh::UnregisterGetSessionIdCallback()
{
    m_getSessionIdCallback = 0;
}

MeshProtocol* Mesh::GetMeshProtocol()
{
    return m_meshProtocol;
}

void* Mesh::GetStationProtocol()
{
    return m_stationProtocol;
}

void Mesh::SetHostMigrationStarted(bool started)
{
    m_hostMigrationStarted = started;
}

bool Mesh::IsHostMigrationStarted()
{
    return m_hostMigrationStarted;
}

void Mesh::SetFatalError(DisconnectReason reason)
{
    g_piaMesh->m_disconnectReason = static_cast<unsigned int>(reason);
    if (static_cast<unsigned int>(reason) == 9)
    {
        g_piaMesh->SetMeshState(cStateNone);
        return;
    }
    g_piaMesh->ProcessingAtMeshEnd(reason);
    g_piaMesh->CleanupStatus();
}

unsigned int Mesh::GetNetworkTopologyType()
{
    return m_networkTopologyType;
}

bool Mesh::IsProcessingLeaveMesh() const
{
    bool running = false;
    if (m_leaveMeshJob != 0)
        running = m_leaveMeshJob->IsRunning();
    if (m_leaveWithHostMigrationJob != 0)
        running |= m_leaveWithHostMigrationJob->IsRunning();
    if (m_destroyMeshJob != 0)
        running |= m_destroyMeshJob->IsRunning();
    return running;
}

unsigned int Mesh::GetLocalPlayerCount() const
{
    return m_localPlayerCount;
}

bool Mesh::IsPlayersCountedAsParticipants() const
{
    return m_playersCountedAsParticipants;
}

common::MonitoringDataSender* Mesh::GetMonitoringDataSender()
{
    return m_monitoringDataSender;
}

unsigned int Mesh::GetBitRateCheckMode()
{
    return m_bitRateCheckMode;
}

void Mesh::SetDenyJoinRequestByInvalidState(bool deny)
{
    m_denyJoinRequestByInvalidState = deny;
}

void Mesh::SetDenyJoinRequestBySessionIsFull(bool deny)
{
    m_denyJoinRequestBySessionIsFull = deny;
}

void Mesh::StartUse(StationIndex index)
{
    if (static_cast<int>(index) > 31)
        return;
    unsigned int mask = 1;
    for (unsigned short shift = 0; shift < static_cast<unsigned short>(index); ++shift)
        mask <<= 1;
    m_validStationBitmap |= mask;
    if (m_unknown24 != 0)
        m_unknown24->m_stationBitmap |= mask;
}

void Mesh::FixConnectedId(StationIndex index)
{
    transport::Station* station = transport::StationManager::s_pInstance->GetStation(index);
    Result result;
    {
        transport::ProtocolEvent event = {cProtocolEventConnect, station->GetStationAddress()};
        result = transport::g_piaTransport->m_protocolManager.UpdateProtocolEvent(event);
    }
    if (result == 0)
    {
        transport::g_piaTransport->OutputStreamUpdateEvent();
        StartUse(index);
        NoticeMeshEvent(cEventTypeJoin, index);
    }
}

void Mesh::JoinResponseComplete(StationIndex index)
{
    if (static_cast<int>(index) > 31)
        return;
    unsigned int mask = 1;
    for (unsigned short shift = 0; shift < static_cast<unsigned short>(index); ++shift)
        mask <<= 1;
    m_connectedStationBitmap |= mask;
}

void Mesh::SetMeshStartTime(common::Time time)
{
    m_meshStartTimeValid = 1;
    m_meshStartTime = time.m_tick;
}

void Mesh::ProcessingAtEndHM()
{
}

void Mesh::AddPlayHistory(transport::Station* station, transport::Station::PlayerInfo*, unsigned int)
{
    nn::fp::RecentPlayRecord record;
    OSBlockSet(&record, 0, sizeof(record));
    transport::IdentificationInfo stationInfo;
    transport::StationConnectionInfo connectionInfo;
    transport::IdentificationInfo localInfo;

    if (transport::IdentificationInfoTable::GetInstance()->GetIdentificationInfo(station, &stationInfo) != 0)
        return;
    if (stationInfo.m_field109 != cIdentificationInfoPlayersCounted)
        return;
    if (stationInfo.m_field60[0].GetStringLength() == 0
        || stationInfo.m_field60[0].GetStringLength() > cRecentPlayNameLengthMax)
        return;
    if (transport::g_stationConnectionInfoTable->GetStationConnectionInfo(station, &connectionInfo) != 0)
        return;
    if (transport::IdentificationInfoTable::GetInstance()->GetLocalIdentificationInfo(&localInfo) != 0)
        return;
    if (localInfo.m_field60[0].GetStringLength() == 0
        || localInfo.m_field60[0].GetStringLength() > cRecentPlayNameLengthMax)
        return;

    const wchar_t* stationName = static_cast<const wchar_t*>(stationInfo.m_field60[0].GetBuffer());
    const unsigned int stationNameLength = stationInfo.m_field60[0].GetStringLength();
    const wchar_t* localName = static_cast<const wchar_t*>(localInfo.m_field60[0].GetBuffer());
    const unsigned int localNameLength = localInfo.m_field60[0].GetStringLength();

    unsigned int index;
    for (index = 0; index < cRecentPlayNameCopyMax && index < stationNameLength; ++index)
        record.inGameName[index] = *stationName++;
    record.inGameName[index] = 0;
    record.principalId = static_cast<nn::act::PrincipalId>(connectionInfo.m_stationLocation.m_principalId);

    for (index = 0; index < cRecentPlayNameCopyMax && index < localNameLength; ++index)
        record.myMiiName[index] = *localName++;
    record.myMiiName[index] = 0;
    record.unk_0x04 = static_cast<uint16_t>((stationInfo.m_field108 << cLanguageByteBits) | localInfo.m_field108);

    nn::fp::AddRecentPlayRecord(&record, cRecentPlayRecordCount);
}

void Mesh::NoticeMeshEvent(EventType type, StationIndex index)
{
    if (type == cEventTypeJoin)
    {
        transport::Station* station = transport::StationManager::s_pInstance->GetStation(index);
        if (station != 0 && m_recentPlayRecordRequested != 0 && m_recentPlayRecordEnabled != 0)
            AddPlayHistory(station, m_playerInfo, 1);
    }
    if (m_meshEventCallback != 0)
    {
        unsigned long long stationId;
        transport::g_piaTransport->ConvertToStationId(&stationId, index);
        m_meshEventCallback(type, stationId);
    }
    if (m_meshEventListener != 0)
    {
        MeshEventListener::MeshEvent event;
        event.type = type;
        event.stationIndex = index;
        event.stationId = 0;
        m_meshEventListener->NoticeEvent(event);
    }
}

unsigned short Mesh::GetStationNum() const
{
    const unsigned int bitmap = IsHost() ? m_connectedStationBitmap : m_validStationBitmap;
    const unsigned int local = m_unknown5c & 0xffff;
    unsigned short count = 1;
    for (unsigned int i = 0; i < 32; ++i)
    {
        if (i != local && (bitmap & (1u << i)) != 0)
            ++count;
    }
    return count;
}

bool Mesh::IsHost() const
{
    bool isHost = false;
    if (static_cast<int>(m_unknown5c) <= 31)
        isHost = m_unknown5c == m_unknown58;
    return isHost;
}

bool Mesh::IsMeshStartTimeValid() const
{
    return m_meshStartTimeValid != 0;
}

common::TimeSpan Mesh::CalcDiffWithMeshStartTime(common::Time time) const
{
    common::TimeSpan span;
    span.m_tick = static_cast<long long>(time.m_tick - m_meshStartTime);
    return span;
}

void Mesh::ProcessingAtMeshEnd(DisconnectReason reason)
{
    MonitoringProcess(reason, 1);
}

Result Mesh::DestroyMesh(common::CallContext* callerContext)
{
    if (m_unknown5c == static_cast<StationIndex>(0xfd) || m_unknown5c != m_unknown58)
        return static_cast<Result>(cResultInvalidState);
    if (callerContext != 0 && callerContext->m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(cResultInvalidState);
    m_unknown56 = 0;
    if (GetStationNum() < 2)
    {
        m_disconnectReason = 2;
        ProcessingAtMeshEnd(static_cast<DisconnectReason>(m_disconnectReason));
        Cleanup();
        if (callerContext != 0)
        {
            callerContext->InitiateCall();
            callerContext->SignalSuccess(0);
        }
    }
    else if (m_destroyMeshJob->Startup(callerContext, false) == 0)
        m_destroyMeshJob->AssociateSystemWith(callerContext);
    else
        m_destroyMeshJob->Ready(false);
    return 0;
}

bool Mesh::IsHostMigrationEnabled() const
{
    return m_hostMigrationEnabled;
}

Result Mesh::LeaveMeshWithHostMigration(common::CallContext* callerContext)
{
    if (!IsHostMigrationEnabled())
        return static_cast<Result>(cResultInvalidState);
    if (m_unknown5c == static_cast<StationIndex>(0xfd) || m_unknown5c != m_unknown58)
        return static_cast<Result>(cResultInvalidState);
    if (callerContext != 0 && callerContext->m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(cResultInvalidState);
    m_unknown56 = 0;
    if (GetStationNum() < 2)
    {
        m_disconnectReason = 2;
        ProcessingAtMeshEnd(static_cast<DisconnectReason>(m_disconnectReason));
        Cleanup();
        if (callerContext != 0)
        {
            callerContext->InitiateCall();
            callerContext->SignalSuccess(0);
        }
    }
    else if (m_leaveWithHostMigrationJob->Startup(callerContext) != 0)
        m_leaveWithHostMigrationJob->Ready(false);
    return 0;
}

Result Mesh::LeaveMesh(common::CallContext* callerContext)
{
    if (callerContext != 0 && callerContext->m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(cResultInvalidState);
    if (m_meshState == cStateNone)
    {
        if (callerContext != 0)
        {
            callerContext->InitiateCall();
            callerContext->SignalSuccess(0);
        }
        return 0;
    }
    if (m_unknown5c == static_cast<StationIndex>(0xfd) || m_unknown5c == m_unknown58)
        return static_cast<Result>(cResultInvalidState);
    if (m_leaveMeshJob->Startup(callerContext))
        m_leaveMeshJob->Ready(false);
    else if ((!m_kickoutManageJob->IsRunning() || !m_kickoutManageJob->AssociateKickoutWith(callerContext))
             && m_leaveMeshJob->RegisterExtraCallback(callerContext) == 0)
        return static_cast<Result>(cResultInvalidState);
    return 0;
}

StationIndex Mesh::GetUsableStationIndex()
{
    for (unsigned int index = 0; index < 32; ++index)
    {
        if ((m_validStationBitmap & (1u << index)) == 0)
            return static_cast<StationIndex>(index);
    }
    return static_cast<StationIndex>(0xfd);
}

unsigned char Mesh::CheckApprovalJoin(transport::Station* station)
{
    unsigned long long principalId = 0;
    if (m_unknown56 == 0 || IsProcessingLeaveMesh())
        return cJoinDenyReasonNotAccepting;
    if (GetStationNum() >= m_unknown54 || GetUsableStationIndex() == cStationIndexInvalid)
        return cJoinDenyReasonSessionFull;

    transport::StationConnectionInfo connectionInfo;
    if (transport::g_stationConnectionInfoTable->GetStationConnectionInfo(station, &connectionInfo) != 0)
        return cJoinDenyReasonNotAccepting;

    if (connectionInfo.m_stationLocation.m_principalId != 0)
        principalId = connectionInfo.m_stationLocation.m_principalId;
    else if (connectionInfo.m_localStationLocation.m_principalId != 0)
        principalId = connectionInfo.m_localStationLocation.m_principalId;
    if (principalId == 0 || transport::g_piaTransport->m_stationIdTable->FindByUniqueIdCore(principalId) == 0)
    {
        if (transport::g_piaTransport->m_stationIdTable->GetEntryNum() >= transport::g_piaTransport->m_stationNum)
            return cJoinDenyReasonSessionFull;
        transport::StationIdTable* stationIdTable = transport::g_piaTransport->m_stationIdTable;
        if (stationIdTable->GetEntryNum() >= stationIdTable->m_availableEntryNum)
            return cJoinDenyReasonSessionFull;
    }

    if (m_meshEventListener != 0)
    {
        unsigned char participantCount = 1;
        if (station->GetParticipantCount(&participantCount) != 0)
            participantCount = 1;
        MeshEventListener::MeshEvent participantsEvent;
        participantsEvent.type = cMeshEventJoinRequestParticipants;
        participantsEvent.stationIndex = station->GetStationIndex();
        participantsEvent.stationId = participantCount;
        m_meshEventListener->NoticeEvent(participantsEvent);
        if (m_denyJoinRequestBySessionIsFull != 0)
        {
            m_denyJoinRequestBySessionIsFull = 0;
            return cJoinDenyReasonSessionFull;
        }

        MeshEventListener::MeshEvent principalEvent;
        principalEvent.type = cMeshEventJoinRequestPrincipal;
        principalEvent.stationIndex = station->GetStationIndex();
        principalEvent.stationId = principalId;
        m_meshEventListener->NoticeEvent(principalEvent);
        if (m_denyJoinRequestByInvalidState != 0)
        {
            m_denyJoinRequestByInvalidState = 0;
            return cJoinDenyReasonInvalidState;
        }
    }
    return cJoinApproved;
}

Result Mesh::JoinMeshCore(const transport::StationConnectionInfo& info, common::CallContext* callerContext)
{
    if (m_meshState == cStateCreated)
        return static_cast<Result>(cResultInvalidState);
    if (transport::StationManager::s_pInstance->m_localStation != 0)
        return static_cast<Result>(cResultInvalidState);
    if (callerContext != 0 && callerContext->m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(cResultInvalidState);
    m_unknown88 = 0;
    m_unknown58 = static_cast<StationIndex>(0xfe);
    m_unknown80 = 0;
    m_unknowncc = 0;
    m_unknownc8 = 0;
    if (m_joinMeshJob->GetState() != common::Job::cStateNotEntried
        && m_joinMeshJob->GetState() != common::Job::cStateCompleted)
        m_joinMeshJob->Reset(true);
    if (m_joinMeshJob->Startup(info, callerContext))
        m_joinMeshJob->Ready(true);
    return 0;
}

ProcessHostMigrationJob* Mesh::GetProcessHostMigrationJobPtr()
{
    return m_processHostMigrationJob;
}

ProcessDestroyMeshJob* Mesh::GetProcessDestroyMeshJobPtr()
{
    return m_processDestroyMeshJob;
}

ProcessUpdateMeshJob* Mesh::GetProcessUpdateMeshJobPtr()
{
    return m_processUpdateMeshJob;
}

Result Mesh::setDebugSettingImpl(const DebugSetting& setting)
{
    if (g_piaMesh != 0)
        return static_cast<Result>(cResultInvalidState);
    sDebugSetting = setting;
    return 0;
}

Result Mesh::SetDebugSettingForInternal(const DebugSetting& setting)
{
    return setDebugSettingImpl(setting);
}

void Mesh::TraceStationConnectionInfoList(unsigned long long)
{
}

void Mesh::NotifyLeaveStationAddress(const common::StationAddress& address)
{
    transport::Station* station = transport::StationManager::s_pInstance->GetStation(address);
    address.Trace(0);
    if (station != 0)
        station->m_field20 = 6;
}

void Mesh::SetSkipReport(bool skip)
{
    m_skipReport = skip;
}

void Mesh::SendReport(unsigned char reason)
{
    if (m_monitoringDataSender != 0)
        m_monitoringDataSender->RequestSendReport(static_cast<common::MonitoringDataSender::DataType>(reason));
}

unsigned int Mesh::GetMeshState() const
{
    return m_meshState;
}

Result Mesh::CheckConnectionError() const
{
    if (m_meshState == cStateCreated)
        return 0;
    return static_cast<Result>(0x6c50);
}

unsigned int Mesh::GetDisconnectReason() const
{
    if (m_meshState == cStateCreated)
        return 0;
    if (m_disconnectReason == cDisconnectReasonNormal)
        return m_disconnectReason;
    if (m_kickoutManageJob->m_kickoutReason == 0)
        return m_disconnectReason;
    if (m_kickoutManageJob->m_kickoutReason != cKickoutReasonRequested)
        return cMonitoringReasonKickoutRequested;
    return cMonitoringReasonKickout;
}

bool Mesh::IsBandwidthCheckProtocolSetupParamValid(int measurementTime, unsigned int ipPacketSize,
                                                   int measurementCount) const
{
    if (m_bitRateCheckMode == 0)
        return true;
    return m_bandwidthCheckProtocol->IsSetupParamValid(measurementTime, ipPacketSize, measurementCount);
}

Result Mesh::SetupProtocols()
{
    return m_meshProtocol->StartupNative();
}

void Mesh::SetupSendReport()
{
    if (m_monitoringDataSender != 0)
        m_monitoringDataSender->Startup();
}

unsigned char Mesh::IsProcessedAtMeshBegin() const
{
    if (m_monitoringDataSender != 0)
        return m_monitoringDataSender->IsSendSessionBeginReportRequested();
    return 0;
}

bool Mesh::IsProcessingHostMigration() const
{
    if (m_processHostMigrationJob == 0)
        return false;
    return m_processHostMigrationJob->m_started;
}

void Mesh::ProcessingAtMeshBegin()
{
    if (m_stationProtocol != 0)
    {
        common::g_sessionBeginMonitoringContent.m_metric126 =
            static_cast<transport::StationProtocol*>(m_stationProtocol)->m_reconnectionCount;
    }
    if (m_skipReport)
        return;
    SendReport(cMonitoringBeginDataType);
}

void Mesh::CleanupMonitoringBeginData()
{
    common::SessionBeginMonitoringContent& content = common::g_sessionBeginMonitoringContent;
    content.m_metric47 = cMonitoringUnset8;
    content.m_joinResult = cMonitoringUnset32;
    content.m_joinProcessingTime = cMonitoringUnset32;
    content.m_metric50 = cMonitoringUnset8;
    for (int index = 0; index < cMonitoringBeginIntervalCount; ++index)
        content.m_metric51[index] = cMonitoringUnset32;
    content.m_metric52 = cMonitoringUnset8;
    for (int index = 0; index < cMonitoringBeginStatCount; ++index)
        content.m_metric53[index] = cMonitoringUnset32;
    content.m_joinMeshState = cMonitoringUnset8;
    content.m_joinTargetIdHash = cMonitoringUnset32;
    content.m_metric126 = cMonitoringUnset32;
}

long long Mesh::GetTime() const
{
    if (transport::g_piaTransport == 0)
        return cTimeTransportUnavailable;

    transport::Protocol* protocol = transport::g_piaTransport->m_protocolManager.SearchProtocol(
        m_syncClockProtocolId, SyncClockProtocol::cProtocolType);
    if (protocol == 0)
        return cTimeSyncClockUnavailable;
    return static_cast<SyncClockProtocol*>(protocol)->m_syncClock.GetTime();
}

void Mesh::MonitoringProcess(DisconnectReason reason, unsigned char dataType)
{
    if (!IsMeshStartTimeValid())
        return;

    const common::Time now = common::g_piaJobScheduler->m_dispatchTime;
    const common::TimeSpan elapsed = CalcDiffWithMeshStartTime(now);
    unsigned int monitoredReason = reason;
    if (reason != cDisconnectReasonNormal)
    {
        if (m_kickoutManageJob->m_kickoutReason != 0)
        {
            monitoredReason = cMonitoringReasonKickoutRequested;
            if (m_kickoutManageJob->m_kickoutReason == cKickoutReasonRequested)
                monitoredReason = cMonitoringReasonKickout;
        }
    }

    unsigned char stationNum = static_cast<unsigned char>(GetStationNum());
    unsigned int version = m_unknown80;
    if (stationNum == 0)
        stationNum = cMonitoringUnset8;
    if (version == 0)
        version = cMonitoringUnset32;

    transport::RelayRouteManager* relayRouteManager = transport::g_piaTransport->m_relayRouteManager;
    unsigned int relayVersion;
    if (relayRouteManager == 0 || (relayVersion = relayRouteManager->m_unknown1c) == 0)
        relayVersion = cMonitoringUnset32;

    common::SessionStateMonitoringContent& content = common::g_sessionStateMonitoringContent;
    content.m_metric179 = static_cast<unsigned int>(elapsed.m_tick / common::TimeSpan::GetTicksPerMSec().m_tick);
    content.m_metric180 = static_cast<unsigned char>(monitoredReason);
    content.m_metric181 = stationNum;
    content.m_metric182 = version;
    content.m_metric183 = relayVersion;
    m_processUpdateMeshJob->SetMonitoringData();
    if (m_processHostMigrationJob != 0)
        m_processHostMigrationJob->SetMonitoringData();
    if (m_stationProtocol != 0)
        content.m_metric222 = static_cast<transport::StationProtocol*>(m_stationProtocol)->m_reconnectionCount;
    common::g_piaJobScheduler->SetMonitoringData();
    transport::g_piaTransport->SetMonitoringNetworkRtt(false);
    transport::g_piaTransportAnalyzer->SetMonitoringData();
    transport::g_piaThreadStreamManager->SetMonitoringData();
    if (!m_skipReport && m_monitoringDataSender != 0)
    {
        m_monitoringDataSender->RequestSendReport(static_cast<common::MonitoringDataSender::DataType>(dataType));
    }
    if (dataType == cMonitoringSessionBeginReport)
        m_meshStartTimeValid = false;
}

Result Mesh::SetupLocalPlayerInfo()
{
    transport::IdentificationInfo info;
    Result result = transport::IdentificationInfoTable::GetInstance()->GetLocalIdentificationInfo(&info);
    if (result != 0)
        return result;

    for (unsigned char index = 0; index < m_playerInfoCount; ++index)
    {
        info.m_field60[index].Copy(m_playerInfo[index].m_name);
        info.m_field108 = m_playerInfo[index].m_language;
    }

    int16_t miiName[cMiiNameBufferSize];
    OSBlockSet(miiName, 0, sizeof(miiName));
    nn::act::Initialize();
    nn::act::GetMiiName(miiName);
    nn::act::Finalize();
    for (unsigned char index = 0; index < m_playerInfoCount; ++index)
        info.m_fieldC8[index].Copy(common::String(reinterpret_cast<const wchar_t*>(miiName)));

    transport::IdentificationInfoTable::GetInstance()->SetLocalIdentificationInfo(&info);
    return 0;
}

}
}
}
