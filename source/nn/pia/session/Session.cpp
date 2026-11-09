#include "nn/pia/session/Session.hpp"

#include <cstring>

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/String.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/common/MonitoringDataSender.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/common/SessionStateMonitoringContent.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"
#include "nn/pia/StationIdTable.hpp"
#include "nn/pia/transport/IdentificationInfoTable.hpp"
#include "nn/pia/session/BrowseMatchmakeJob.hpp"
#include "nn/pia/session/ClearMatchmakeSystemPasswordJob.hpp"
#include "nn/pia/session/CloseParticipationJob.hpp"
#include "nn/pia/session/CommunityManagementJob.hpp"
#include "nn/pia/session/CommunitySearchCriteria.hpp"
#include "nn/pia/session/CreateSessionJob.hpp"
#include "nn/pia/session/CreateSessionSetting.hpp"
#include "nn/pia/session/DestroySessionJob.hpp"
#include "nn/pia/session/GenerateMatchmakeSystemPasswordJob.hpp"
#include "nn/pia/session/ICommunityInfoList.hpp"
#include "nn/pia/session/IMatchmakeSession.hpp"
#include "nn/pia/session/ISessionInfoList.hpp"
#include "nn/pia/session/JoinSessionJob.hpp"
#include "nn/pia/session/JointSessionJob.hpp"
#include "nn/pia/session/LeaveSessionJob.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/MeshEventListener.hpp"
#include "nn/pia/session/OpenParticipationJob.hpp"
#include "nn/pia/session/ProcessHostMigrationJob.hpp"
#include "nn/pia/session/MeshLayerController.hpp"
#include "nn/pia/session/RandomMatchmakeJob.hpp"
#include "nn/pia/session/SessionNetworkFactory.hpp"
#include "nn/pia/session/SessionProtocol.hpp"
#include "nn/pia/session/SessionSearchCriteria.hpp"
#include "nn/pia/session/SessionStatusCheckJob.hpp"
#include "nn/pia/session/SessionSupport.hpp"
#include "nn/pia/session/StationIdStatusTable.hpp"
#include "nn/pia/session/UpdateSessionSettingJob.hpp"

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
    cStatusMatchedCurrent = 1,
    cStatusMatchedBuffer = 3,
    cMatchmakeStatusUnknown3 = 3,
    cMatchmakeStatusUnknown4 = 4,
    cResultOwnerChanged = 0x6c59,
    cTraceMaskStationTable = 0x4000000,
    cRankingUnknownStation = 0xfd,
    cRankingUnknownSession = 0xfc,
    cResultStationIdConvertDisabled = 0x4c11,
    cSessionStorageAlignment = 8,
    cMinParticipantNum = 2,
    cLeaveSessionDisconnectReason = 2,
    cStationIdListBufferAlignment = sizeof(unsigned long long),
    cStationLeaveStateRequested = 5,
    cStationLeaveStateLeaving = 6,
    cStationIndexMax = transport::cStationNumMax - 1
};

enum
{
    cMonitoringByteUnset = 0xff,
    cMonitoringWordUnset = 0xffffffff
};

unsigned char* s_sessionStorage = 0;

const unsigned char s_joinRandomStateByJoinMeshState[4] = {4, 5, 6, 7};
const unsigned char s_joinSessionStateByJoinMeshState[4] = {4, 5, 6, 7};
}

Session* g_piaSession = 0;
Session::DebugSetting Session::s_debugSetting;

Session::Session(const Setting&)
    : m_selectionPriorityEnabled(false),
      m_meshLayerController(0),
      m_sessionStatusCheckJob(0),
      m_createSessionJob(0),
      m_randomMatchmakeJob(0),
      m_browseMatchmakeJob(0),
      m_joinSessionJob(0),
      m_leaveSessionJob(0),
      m_destroySessionJob(0),
      m_openParticipationJob(0),
      m_closeParticipationJob(0),
      m_generateMatchmakeSystemPasswordJob(0),
      m_clearMatchmakeSystemPasswordJob(0),
      m_matchmakeSystemPassword(0),
      m_jointSessionJob(0),
      m_updateSessionSettingJob(0),
      m_communityManagementJob(0),
      m_sessionEventCallback(0),
      m_sessionEventCallbackParam(0),
      m_joiningApprovalCallback(0),
      m_stationIdStatusTable(0),
      m_protocolId(0),
      m_sessionProtocol(0),
      m_startupState(0),
      m_matchmakeStatus(0),
      m_disconnectStatus(0),
      m_localStationId(0),
      m_hostStationId(0),
      m_jointSessionHostStationId(0),
      m_field90(0),
      m_callContext(),
      m_asyncProcessingName(cAsyncProcessingNameNone),
      m_meshEventListener(0),
      m_waitingStationIndex(0),
      m_fieldb8(0),
      m_sessionDisconnectReason(0),
      m_fieldc0(0),
      m_postponeNotice(false),
      m_currentSessionIndex(0),
      m_sessionInfoList(0),
      m_communityInfoList(0),
      m_validStationIdList(),
      m_validStationIdBuffer(0),
      m_hostMigrationOccurred(false),
      m_playerNum(0),
      m_participantNum(0),
      m_field13c(0),
      m_field140(0),
      m_field144(0),
      m_field148(0)
{
    m_sessionId[0] = 0;
    m_sessionId[1] = 0;
    m_sessionOpenStatus[0] = cSessionOpenStatusClosed;
    m_sessionOpenStatus[1] = cSessionOpenStatusClosed;
    m_fieldc2[0] = 0;
    m_fieldc2[1] = 0;
    std::memset(&m_matchmakeAnalysisToken, 0, sizeof(m_matchmakeAnalysisToken));
}

Session::~Session()
{
    SessionNetworkFactory* factory = static_cast<SessionNetworkFactory*>(
        transport::g_piaTransport->GetNetworkFactory());
    if (m_createSessionJob != 0)
    {
        factory->DeleteMatchCreateSessionJob(m_createSessionJob);
        m_createSessionJob = 0;
    }
    if (m_randomMatchmakeJob != 0)
    {
        factory->DeleteMatchRandomMatchmakeJob(m_randomMatchmakeJob);
        m_randomMatchmakeJob = 0;
    }
    if (m_browseMatchmakeJob != 0)
    {
        factory->DeleteMatchBrowseMatchmakeJob(m_browseMatchmakeJob);
        m_browseMatchmakeJob = 0;
    }
    if (m_joinSessionJob != 0)
    {
        factory->DeleteMatchJoinSessionJob(m_joinSessionJob);
        m_joinSessionJob = 0;
    }
    if (m_leaveSessionJob != 0)
    {
        factory->DeleteMatchLeaveSessionJob(m_leaveSessionJob);
        m_leaveSessionJob = 0;
    }
    if (m_destroySessionJob != 0)
    {
        factory->DeleteMatchDestroySessionJob(m_destroySessionJob);
        m_destroySessionJob = 0;
    }
    if (m_generateMatchmakeSystemPasswordJob != 0)
    {
        factory->DeleteGenerateMatchmakeSystemPasswordJob(m_generateMatchmakeSystemPasswordJob);
        m_generateMatchmakeSystemPasswordJob = 0;
    }
    if (m_clearMatchmakeSystemPasswordJob != 0)
    {
        factory->DeleteClearMatchmakeSystemPasswordJob(m_clearMatchmakeSystemPasswordJob);
        m_clearMatchmakeSystemPasswordJob = 0;
    }
    if (m_matchmakeSession[0] != 0)
    {
        factory->DeleteMatchmakeSession(m_matchmakeSession[0]);
        m_matchmakeSession[0] = 0;
    }
    if (m_matchmakeSession[1] != 0)
    {
        factory->DeleteMatchmakeSession(m_matchmakeSession[1]);
        m_matchmakeSession[1] = 0;
    }
    if (m_sessionInfoList != 0)
    {
        factory->DeleteSessionInfoList(m_sessionInfoList);
        m_sessionInfoList = 0;
    }
    if (m_communityInfoList != 0)
    {
        factory->DeleteCommunityInfoList(m_communityInfoList);
        m_communityInfoList = 0;
    }
    if (m_jointSessionJob != 0)
    {
        factory->DestroyMatchJointSessionJob(m_jointSessionJob);
        m_jointSessionJob = 0;
    }
    if (m_updateSessionSettingJob != 0)
    {
        factory->DeleteMatchUpdateSessionSettingJob(m_updateSessionSettingJob);
        m_updateSessionSettingJob = 0;
    }
    if (m_communityManagementJob != 0)
    {
        factory->DeleteCommunityManagementJob(m_communityManagementJob);
        m_communityManagementJob = 0;
    }
    if (m_meshLayerController != 0)
    {
        factory->DeleteMatchMeshLayerController(m_meshLayerController);
        m_meshLayerController = 0;
    }
    if (m_stationIdStatusTable != 0)
    {
        m_stationIdStatusTable->~StationIdStatusTable();
        pead::freeToSeadHeap(m_stationIdStatusTable);
        m_stationIdStatusTable = 0;
    }
    if (m_sessionStatusCheckJob != 0)
    {
        m_sessionStatusCheckJob->~SessionStatusCheckJob();
        pead::freeToSeadHeap(m_sessionStatusCheckJob);
        m_sessionStatusCheckJob = 0;
    }
    if (m_openParticipationJob != 0)
    {
        m_openParticipationJob->~OpenParticipationJob();
        pead::freeToSeadHeap(m_openParticipationJob);
        m_openParticipationJob = 0;
    }
    if (m_closeParticipationJob != 0)
    {
        m_closeParticipationJob->~CloseParticipationJob();
        pead::freeToSeadHeap(m_closeParticipationJob);
        m_closeParticipationJob = 0;
    }
    if (m_meshEventListener != 0)
    {
        m_meshEventListener->~MeshEventListener();
        pead::freeToSeadHeap(m_meshEventListener);
        m_meshEventListener = 0;
    }
    if (m_matchmakeSystemPassword != 0)
    {
        factory->DeleteMatchmakeSessionSystemPassword(m_matchmakeSystemPassword);
        m_matchmakeSystemPassword = 0;
    }
    if (m_validStationIdBuffer != 0)
    {
        if (m_validStationIdList.IsInitialized())
            m_validStationIdList.Clear();
        common::DeleteArray(m_validStationIdBuffer);
        m_validStationIdBuffer = 0;
    }
    if (m_sessionProtocol != 0)
    {
        m_sessionProtocol->Finalize();
        m_sessionProtocol = 0;
    }
    if (m_protocolId != 0)
    {
        if (transport::g_piaTransport != 0)
            transport::g_piaTransport->m_protocolManager.DestroyProtocol(m_protocolId);
        m_protocolId = 0;
    }
}

Result Session::CreateInstance(const Setting& setting)
{
    if (!IsInitialized())
        return static_cast<Result>(transport::cResultNotInitialized);
    if (!IsDuringSetup())
        return static_cast<Result>(transport::cResultInvalidState);
    if (g_piaSession != 0)
        return static_cast<Result>(transport::cResultAlreadyExists);
    if (setting.m_networkFactory == 0)
        return static_cast<Result>(transport::cResultInvalidArgument);
    if (setting.m_networkTopologyType != 0 && !setting.m_networkFactory->IsClientRelayConnectionPermitted())
        return static_cast<Result>(transport::cResultInvalidArgument);
    if (setting.m_networkFactory->GetBrowseSessionInfoListSizeMax() < setting.m_browseSessionInfoListSize)
        return static_cast<Result>(transport::cResultInvalidArgument);
    if (setting.m_networkFactory->GetBrowseCommunityInfoListSizeMax() < setting.m_browseCommunityInfoListSize)
        return static_cast<Result>(transport::cResultInvalidArgument);
    if (setting.m_bitRateCheckMode == 1 && !setting.m_networkFactory->IsBitRateCheckPermitted())
        return static_cast<Result>(transport::cResultInvalidArgument);

    Mesh::DebugSetting meshDebugSetting;
    meshDebugSetting.m_relayEmulationEnabled = s_debugSetting.m_relayEmulationEnabled;
    meshDebugSetting.m_checkStartupSettingRange = s_debugSetting.m_checkStartupSettingRange;
    Result result = Mesh::SetDebugSettingForInternal(meshDebugSetting);
    if (result != 0)
        return result;

    Mesh::Setting meshSetting;
    meshSetting.m_networkFactory = setting.m_networkFactory;
    meshSetting.m_networkTopologyType = setting.m_networkTopologyType;
    meshSetting.m_bitRateCheckMode = setting.m_bitRateCheckMode;
    result = Mesh::CreateInstance(meshSetting);
    if (result != 0)
        return result;

    if (g_piaSession == 0)
    {
        s_sessionStorage = static_cast<unsigned char*>(
            pead::allocateFromSeadHeap(sizeof(Session), common::HeapManager::GetHeap(), cSessionStorageAlignment));
        if (s_sessionStorage != 0)
        {
            unsigned char* bytes = s_sessionStorage;
            for (unsigned int remaining = sizeof(Session); remaining != 0; --remaining)
            {
                new (bytes) unsigned char();
                ++bytes;
            }
        }
        if (s_sessionStorage == 0)
        {
            s_sessionStorage = 0;
            return static_cast<Result>(transport::cResultOutOfMemory);
        }
        g_piaSession = new (s_sessionStorage) Session(setting);

        g_piaSession->m_currentSessionIndex = 0;
        g_piaSession->m_sessionInfoList = setting.m_networkFactory->CreateSessionInfoList(
            setting.m_browseSessionInfoListSize);
        g_piaSession->m_communityInfoList = setting.m_networkFactory->CreateCommunityInfoList(
            setting.m_browseCommunityInfoListSize);
        g_piaSession->m_matchmakeSession[0] = setting.m_networkFactory->CreateMatchmakeSession();
        const unsigned short stationNum = transport::g_piaTransport->m_stationNum;
        if (setting.m_networkFactory->IsJointSessionPermitted())
        {
            g_piaSession->m_matchmakeSession[1] = setting.m_networkFactory->CreateMatchmakeSession();
            if (g_piaSession->m_jointSessionJob == 0)
                g_piaSession->m_jointSessionJob = static_cast<JointSessionJob*>(
                    setting.m_networkFactory->CreateMatchJointSessionJob());
        }
        else
        {
            g_piaSession->m_matchmakeSession[1] = 0;
            g_piaSession->m_jointSessionJob = 0;
        }

        if (g_piaSession->m_stationIdStatusTable == 0)
            g_piaSession->m_stationIdStatusTable = transport::CreateObject<StationIdStatusTable>();

        g_piaSession->m_protocolId = transport::g_piaTransport->m_protocolManager.CreateProtocol<SessionProtocol>(0);
        g_piaSession->m_sessionProtocol = transport::g_piaTransport->m_protocolManager.GetProtocol<SessionProtocol>(
            g_piaSession->m_protocolId);
        g_piaSession->m_stationIdStatusTable->Initialize(stationNum);
        g_piaSession->m_sessionProtocol->Initialize(stationNum);

        if (g_piaSession->m_sessionStatusCheckJob == 0)
            g_piaSession->m_sessionStatusCheckJob = transport::CreateObject<SessionStatusCheckJob>();
        if (g_piaSession->m_createSessionJob == 0)
            g_piaSession->m_createSessionJob = static_cast<CreateSessionJob*>(
                setting.m_networkFactory->CreateMatchCreateSessionJob());
        if (g_piaSession->m_randomMatchmakeJob == 0)
            g_piaSession->m_randomMatchmakeJob = static_cast<RandomMatchmakeJob*>(
                setting.m_networkFactory->CreateMatchRandomMatchmakeJob());
        if (g_piaSession->m_browseMatchmakeJob == 0)
            g_piaSession->m_browseMatchmakeJob = static_cast<BrowseMatchmakeJob*>(
                setting.m_networkFactory->CreateMatchBrowseMatchmakeJob());
        if (g_piaSession->m_joinSessionJob == 0)
            g_piaSession->m_joinSessionJob = static_cast<JoinSessionJob*>(
                setting.m_networkFactory->CreateMatchJoinSessionJob());
        if (g_piaSession->m_leaveSessionJob == 0)
            g_piaSession->m_leaveSessionJob = static_cast<LeaveSessionJob*>(
                setting.m_networkFactory->CreateMatchLeaveSessionJob());
        if (g_piaSession->m_destroySessionJob == 0)
            g_piaSession->m_destroySessionJob = static_cast<DestroySessionJob*>(
                setting.m_networkFactory->CreateMatchDestroySessionJob());
        if (g_piaSession->m_openParticipationJob == 0)
            g_piaSession->m_openParticipationJob = transport::CreateObject<OpenParticipationJob>();
        if (g_piaSession->m_closeParticipationJob == 0)
            g_piaSession->m_closeParticipationJob = transport::CreateObject<CloseParticipationJob>();
        if (g_piaSession->m_generateMatchmakeSystemPasswordJob == 0)
            g_piaSession->m_generateMatchmakeSystemPasswordJob = static_cast<GenerateMatchmakeSystemPasswordJob*>(
                setting.m_networkFactory->CreateGenerateMatchmakeSystemPasswordJob());
        if (g_piaSession->m_clearMatchmakeSystemPasswordJob == 0)
            g_piaSession->m_clearMatchmakeSystemPasswordJob = static_cast<ClearMatchmakeSystemPasswordJob*>(
                setting.m_networkFactory->CreateClearMatchmakeSystemPasswordJob());
        if (g_piaSession->m_matchmakeSystemPassword == 0)
            g_piaSession->m_matchmakeSystemPassword = setting.m_networkFactory->CreateMatchmakeSessionSystemPassword();
        if (g_piaSession->m_updateSessionSettingJob == 0)
            g_piaSession->m_updateSessionSettingJob = static_cast<UpdateSessionSettingJob*>(
                setting.m_networkFactory->CreateMatchUpdateSessionSettingJob());
        if (g_piaSession->m_communityManagementJob == 0)
            g_piaSession->m_communityManagementJob = static_cast<CommunityManagementJob*>(
                setting.m_networkFactory->CreateCommunityManagementJob());
        if (g_piaSession->m_meshLayerController == 0)
            g_piaSession->m_meshLayerController = setting.m_networkFactory->CreateMatchMeshLayerController();
        g_piaSession->m_selectionPriorityEnabled = setting.m_networkFactory->IsServerRequired();

        if (g_piaSession->m_meshEventListener == 0)
            g_piaSession->m_meshEventListener = transport::CreateObject<MeshEventListener>();
        if (g_piaSession->m_validStationIdBuffer == 0)
        {
            g_piaSession->m_validStationIdBuffer = common::NewArray<unsigned long long>(
                (sizeof(StationIdList::Node) * stationNum + cStationIdListBufferAlignment - 1)
                / cStationIdListBufferAlignment);
            g_piaSession->m_validStationIdList.Initialize(g_piaSession->m_validStationIdBuffer, stationNum);
        }
    }
    g_piaSession->m_startupState = 0;
    return 0;
}

void Session::DestroyInstance()
{
    if (g_piaSession != 0)
    {
        g_piaSession->~Session();
        g_piaSession = 0;
    }
    if (s_sessionStorage != 0)
    {
        pead::DeleteArray(s_sessionStorage);
        s_sessionStorage = 0;
    }
    if (g_piaMesh != 0)
        Mesh::DestroyInstance();
}

void Session::ResetSessionSystemPassword()
{
    if (m_matchmakeSystemPassword != 0)
        m_matchmakeSystemPassword->Clear();
}

void Session::CleanupStatus(bool keepStationIds)
{
    m_postponeNotice = false;
    if (m_localStationId != 0)
    {
        transport::StationIdTable::EntryList& entries = transport::g_piaTransport->m_stationIdTable->m_entries;
        unsigned long long stationIds[transport::cStationNumMax];
        unsigned short stationIdNum = 0;
        bool localStationFound = false;
        for (common::ListNode* node = entries.Sentinel()->next; node != entries.Sentinel(); node = node->next)
        {
            const unsigned long long stationId = transport::StationIdTable::EntryList::ValueOf(node)->stationId;
            if (stationId == m_localStationId)
                localStationFound = true;
            else if (stationId != 0)
                stationIds[stationIdNum++] = stationId;
        }
        if (localStationFound)
            stationIds[stationIdNum++] = m_localStationId;
        for (unsigned short i = 0; i < stationIdNum; ++i)
            RemoveStationId(stationIds[i], static_cast<StationIndex>(transport::cStationIndexInvalid), 0, 0);
        transport::g_piaTransport->m_stationIdTable->Trace(cTraceMaskStationTable);
        m_stationIdStatusTable->Trace(cTraceMaskStationTable);
    }
    transport::g_piaTransport->m_stationIdTable->Clear();
    m_stationIdStatusTable->ClearAllElements();
    m_disconnectStatus = cConnectionStatusNone;
    m_sessionDisconnectReason = 0;
    m_matchmakeStatus = cMatchmakeStatusNone;
    if (!keepStationIds)
    {
        m_localStationId = 0;
        m_hostStationId = 0;
        m_jointSessionHostStationId = 0;
    }
    m_sessionId[0] = 0;
    m_sessionId[1] = 0;
    m_sessionOpenStatus[0] = cSessionOpenStatusClosed;
    m_sessionOpenStatus[1] = cSessionOpenStatusClosed;
    m_fieldc2[0] = 0;
    m_fieldc2[1] = 0;
    m_playerNum = 0;
    m_participantNum = 0;
    m_field13c = 0;
    m_field140 = 0;
    m_field144 = 0;
    m_field148 = 0;
    std::memset(&m_matchmakeAnalysisToken, 0, sizeof(m_matchmakeAnalysisToken));
    ResetSessionSystemPassword();
}

void Session::Cleanup()
{
    if (g_piaMesh != 0)
        g_piaMesh->Cleanup();
    if (transport::g_piaTransport != 0)
        transport::g_piaTransport->Cleanup();
    for (int i = 0; i < 2; ++i)
    {
        if (m_matchmakeSession[i] != 0)
            m_matchmakeSession[i]->Cleanup();
    }
    if (m_createSessionJob != 0)
    {
        m_createSessionJob->Reset(false);
        m_createSessionJob->Cleanup();
    }
    if (m_randomMatchmakeJob != 0)
    {
        m_randomMatchmakeJob->Reset(false);
        m_randomMatchmakeJob->Cleanup();
    }
    if (m_browseMatchmakeJob != 0)
    {
        m_browseMatchmakeJob->Reset(false);
        m_browseMatchmakeJob->Cleanup();
    }
    if (m_joinSessionJob != 0)
    {
        m_joinSessionJob->Reset(false);
        m_joinSessionJob->Cleanup();
    }
    if (m_leaveSessionJob != 0)
    {
        m_leaveSessionJob->Reset(false);
        m_leaveSessionJob->Cleanup();
    }
    if (m_destroySessionJob != 0)
    {
        m_destroySessionJob->Reset(false);
        m_destroySessionJob->Cleanup();
    }
    if (m_openParticipationJob != 0)
    {
        m_openParticipationJob->Reset(false);
        m_openParticipationJob->Cleanup();
    }
    if (m_closeParticipationJob != 0)
    {
        m_closeParticipationJob->Reset(false);
        m_closeParticipationJob->Cleanup();
    }
    if (m_generateMatchmakeSystemPasswordJob != 0)
    {
        m_generateMatchmakeSystemPasswordJob->Reset(false);
        m_generateMatchmakeSystemPasswordJob->Cleanup();
    }
    if (m_clearMatchmakeSystemPasswordJob != 0)
    {
        m_clearMatchmakeSystemPasswordJob->Reset(false);
        m_clearMatchmakeSystemPasswordJob->Cleanup();
    }
    if (m_updateSessionSettingJob != 0)
    {
        m_updateSessionSettingJob->Reset(false);
        m_updateSessionSettingJob->Cleanup();
    }
    if (m_jointSessionJob != 0)
    {
        m_jointSessionJob->Reset(false);
        m_jointSessionJob->Cleanup();
    }
    m_meshLayerController->Cleanup();
    CleanupStatus(true);
    transport::g_piaTransport->SetTransportStatus(0);
    if (m_sessionStatusCheckJob != 0)
    {
        m_sessionStatusCheckJob->Reset(false);
        m_sessionStatusCheckJob->Cleanup();
    }
    m_jointSessionHostStationId = 0;
    m_localStationId = 0;
    m_hostStationId = 0;
    m_startupState = 0;
    m_callContext.Reset();
    m_asyncProcessingName = cAsyncProcessingNameNone;
    ClearWaitingStationIndex();
    m_fieldb8 = 0;

    common::SessionBeginMonitoringContent& monitoring = common::g_sessionBeginMonitoringContent;
    monitoring.m_joinSessionState = cMonitoringByteUnset;
    monitoring.m_metric77 = cMonitoringWordUnset;
    monitoring.m_metric158 = cMonitoringByteUnset;
    monitoring.m_metric76 = cMonitoringWordUnset;
    monitoring.m_metric79 = cMonitoringByteUnset;
    monitoring.m_metric75 = cMonitoringWordUnset;
    monitoring.m_metric74 = cMonitoringByteUnset;
    monitoring.m_retryReasonACount = cMonitoringByteUnset;
    monitoring.m_ownerChangeRetryCount = cMonitoringByteUnset;
    monitoring.m_metric130 = cMonitoringByteUnset;
    monitoring.m_metric78 = cMonitoringByteUnset;
    monitoring.m_metric128 = cMonitoringByteUnset;
    monitoring.m_joinMeshState = cMonitoringByteUnset;
    monitoring.m_joinRetryCount = cMonitoringByteUnset;
    monitoring.m_sessionInfoRetryCount = cMonitoringByteUnset;
    monitoring.m_metric127 = cMonitoringByteUnset;
    monitoring.m_retryReasonBCount = cMonitoringByteUnset;
    monitoring.m_connectionInfoRetryCount = cMonitoringByteUnset;
    monitoring.Cleanup();
    common::g_sessionStateMonitoringContent.Cleanup();
    if (g_piaMesh->GetMonitoringDataSender() != 0)
        g_piaMesh->GetMonitoringDataSender()->Cleanup();
    g_piaMesh->SetSkipCleanupReport(false);
}

void Session::ClearWaitingStationIndex()
{
    m_waitingStationIndex = 0;
}

bool Session::LeaveNoticeToWaitingStationIndex(StationIndex stationIndex)
{
    if (stationIndex > cStationIndexMax)
        return false;
    unsigned int mask = 1;
    for (int shift = stationIndex; shift > 0; --shift)
        mask <<= 1;
    m_waitingStationIndex &= ~mask;
    return true;
}

bool Session::IsAsyncProcessCompleted(AsyncProcessingName name) const
{
    if (m_asyncProcessingName == name)
        return m_callContext.m_state == common::CallContext::cStateSuccess
               || m_callContext.m_state == common::CallContext::cStateFailure
               || m_callContext.m_state == common::CallContext::cStateCancelled;
    return false;
}

Result Session::GetAsyncProcessResult(AsyncProcessingName name) const
{
    if (m_asyncProcessingName == name && m_callContext.m_state >= common::CallContext::cStateSuccess
        && m_callContext.m_state <= common::CallContext::cStateCancelled)
        return m_callContext.m_result;
    return static_cast<Result>(transport::cResultInvalidState);
}

Result Session::JoinRandomSessionAsync(const JoinRandomSessionSetting* setting)
{
    if (m_randomMatchmakeJob == 0)
        return static_cast<Result>(transport::cResultInvalidState);
    if (setting == 0)
        return static_cast<Result>(transport::cResultInvalidArgument);
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(transport::cResultInvalidState);
    if (m_startupState != 1)
        return static_cast<Result>(transport::cResultInvalidState);
    if (m_matchmakeStatus != cMatchmakeStatusNone)
        return static_cast<Result>(transport::cResultInvalidState);

    ResetInternalAsyncState();
    Result result = m_randomMatchmakeJob->Startup(&m_callContext, setting);
    if (result == 0)
    {
        m_asyncProcessingName = cAsyncProcessingNameJoinRandomSession;
        m_field13c = 0;
        m_field140 = 0;
        m_randomMatchmakeJob->Ready(false);
    }
    return result;
}

bool Session::IsJoinRandomSessionCompleted() const
{
    return IsAsyncProcessCompleted(cAsyncProcessingNameJoinRandomSession);
}

Result Session::GetJoinRandomSessionResult() const
{
    return GetAsyncProcessResult(cAsyncProcessingNameJoinRandomSession);
}

Result Session::BrowseSessionAsync(const SessionSearchCriteria* criteria)
{
    if (m_sessionInfoList != 0)
        m_sessionInfoList->Clear();
    if (criteria == 0)
        return static_cast<Result>(transport::cResultInvalidArgument);
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(transport::cResultInvalidState);
    if (m_startupState != 1)
        return static_cast<Result>(transport::cResultInvalidState);
    if (m_sessionInfoList->GetMaxSize() == 0)
        return static_cast<Result>(transport::cResultInvalidState);
    if (criteria->GetKeyType() != SessionSearchCriteria::cKeyTypeParticipant
        && criteria->GetKeyType() != SessionSearchCriteria::cKeyTypeSessionId
        && m_sessionInfoList->GetMaxSize() < criteria->GetResultRangeSize())
        return static_cast<Result>(transport::cResultInvalidArgument);

    ResetInternalAsyncState();
    Result result = m_browseMatchmakeJob->Startup(&m_callContext, m_matchmakeSession[m_currentSessionIndex], criteria);
    if (result == 0)
    {
        m_browseMatchmakeJob->Ready(false);
        m_asyncProcessingName = cAsyncProcessingNameBrowseSession;
    }
    return result;
}

bool Session::IsBrowseSessionCompleted() const
{
    return IsAsyncProcessCompleted(cAsyncProcessingNameBrowseSession);
}

Result Session::GetBrowseSessionResult() const
{
    return GetAsyncProcessResult(cAsyncProcessingNameBrowseSession);
}

Result Session::BrowseCommunityAsync(const CommunitySearchCriteria* criteria)
{
    if (m_matchmakeSession[m_currentSessionIndex] != 0
        && m_matchmakeSession[m_currentSessionIndex]->GetBrowsedCommunityResult() != 0)
        m_matchmakeSession[m_currentSessionIndex]->GetBrowsedCommunityResult()->Clear();
    if (criteria == 0)
        return static_cast<Result>(transport::cResultInvalidArgument);
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(transport::cResultInvalidState);
    if (m_startupState != 1)
        return static_cast<Result>(transport::cResultInvalidState);
    if (m_communityInfoList == 0)
        return static_cast<Result>(transport::cResultInvalidState);
    if (m_communityInfoList->GetMaxSize() == 0)
        return static_cast<Result>(transport::cResultInvalidState);
    if (m_communityInfoList->GetMaxSize() < criteria->GetResultRangeSize())
        return static_cast<Result>(transport::cResultInvalidArgument);

    ResetInternalAsyncState();
    if (m_communityManagementJob == 0)
        return static_cast<Result>(transport::cResultInvalidState);
    Result result = m_communityManagementJob->BrowseCommunityStartup(&m_callContext, criteria);
    if (result == 0)
    {
        m_asyncProcessingName = cAsyncProcessingNameBrowseCommunity;
        m_communityManagementJob->Ready(false);
    }
    return result;
}

ISessionInfoList* Session::GetBrowsedSessionInfoList()
{
    return m_matchmakeSession[m_currentSessionIndex]->GetBrowseMatchmakeSessionResult();
}

ICommunityInfoList* Session::GetBrowsedCommunityInfoList()
{
    return m_matchmakeSession[m_currentSessionIndex]->GetBrowsedCommunityResult();
}

Result Session::CreateSessionAsync(const CreateSessionSetting* setting)
{
    if (setting == 0)
        return static_cast<Result>(transport::cResultInvalidArgument);
    if (setting->GetMaxParticipantNum() < cMinParticipantNum)
        return static_cast<Result>(transport::cResultInvalidArgument);
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(transport::cResultInvalidState);
    if (m_startupState != 1)
        return static_cast<Result>(transport::cResultInvalidState);
    if (m_matchmakeStatus != cMatchmakeStatusNone)
        return static_cast<Result>(transport::cResultInvalidState);

    ResetInternalAsyncState();
    Result result = m_createSessionJob->Startup(&m_callContext, setting);
    if (result == 0)
    {
        m_createSessionJob->Ready(false);
        m_asyncProcessingName = cAsyncProcessingNameCreateSession;
    }
    return result;
}

bool Session::IsCreateSessionCompleted() const
{
    return IsAsyncProcessCompleted(cAsyncProcessingNameCreateSession);
}

Result Session::GetCreateSessionResult() const
{
    return GetAsyncProcessResult(cAsyncProcessingNameCreateSession);
}

bool Session::IsJoinSessionCompleted() const
{
    return IsAsyncProcessCompleted(cAsyncProcessingNameJoinSession);
}

Result Session::GetJoinSessionResult() const
{
    return GetAsyncProcessResult(cAsyncProcessingNameJoinSession);
}

Result Session::LeaveSessionAsync()
{
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(transport::cResultInvalidState);
    ResetInternalAsyncState();
    if (m_startupState != 1)
        return static_cast<Result>(transport::cResultInvalidState);

    Result result;
    if (!g_piaMesh->IsHost() || g_piaMesh->IsHostMigrationEnabled())
    {
        result = m_leaveSessionJob->Startup(&m_callContext);
        if (result == 0)
        {
            m_leaveSessionJob->Ready(false);
            m_asyncProcessingName = cAsyncProcessingNameLeaveSession;
            m_meshLayerController->SetLeaveFlag();
        }
    }
    else
    {
        result = m_destroySessionJob->Startup(&m_callContext);
        if (result == 0)
        {
            m_destroySessionJob->Ready(false);
            m_asyncProcessingName = cAsyncProcessingNameLeaveSession;
            m_meshLayerController->SetLeaveFlag();
        }
    }
    m_sessionDisconnectReason = cLeaveSessionDisconnectReason;
    return result;
}

bool Session::IsLeaveSessionCompleted() const
{
    return IsAsyncProcessCompleted(cAsyncProcessingNameLeaveSession);
}

Result Session::GetLeaveSessionResult() const
{
    return GetAsyncProcessResult(cAsyncProcessingNameLeaveSession);
}

bool Session::IsUpdateSessionSettingCompleted() const
{
    return IsAsyncProcessCompleted(cAsyncProcessingNameUpdateSessionSetting);
}

Result Session::GetUpdateSessionSettingResult() const
{
    return GetAsyncProcessResult(cAsyncProcessingNameUpdateSessionSetting);
}

Result Session::SetSessionSystemPassword(const common::String& password)
{
    if (m_matchmakeSystemPassword == 0)
        return static_cast<Result>(transport::cResultInvalidState);
    if (password.GetBuffer() == 0)
        return static_cast<Result>(transport::cResultInvalidArgument);
    return m_matchmakeSystemPassword->Copy(password);
}

Result Session::UpdateSelectionPriority(unsigned char selectionPriority) const
{
    if (!m_selectionPriorityEnabled)
        return static_cast<Result>(transport::cResultInvalidState);
    if (!IsHost())
        return static_cast<Result>(transport::cResultInvalidState);
    IMatchmakeSession* matchmakeSession = m_matchmakeSession[m_currentSessionIndex];
    if (matchmakeSession == 0)
        return static_cast<Result>(transport::cResultInvalidState);
    return matchmakeSession->UpdateSelectionPriority(m_sessionId[m_currentSessionIndex], selectionPriority);
}

bool Session::IsUpdatableSelectionPriority(unsigned char selectionPriority) const
{
    if (m_selectionPriorityEnabled && IsHost())
    {
        IMatchmakeSession* matchmakeSession = m_matchmakeSession[m_currentSessionIndex];
        if (matchmakeSession != 0)
            return matchmakeSession->IsUpdatableSelectionPriority(selectionPriority);
    }
    return false;
}

unsigned long long Session::ConvertCommunityIdToCommunityCode(unsigned int communityId)
{
    return m_matchmakeSession[m_currentSessionIndex]->ConvertGatheringIdToPersistentGatheringCode(communityId);
}

unsigned int Session::ConvertCommunityCodeToCommunityId(unsigned long long communityCode)
{
    return m_matchmakeSession[m_currentSessionIndex]->ConvertPersistentGatheringCodeToGatheringId(communityCode);
}

bool Session::IsOwner() const
{
    IMatchmakeSession* matchmakeSession = m_matchmakeSession[m_currentSessionIndex];
    if (matchmakeSession != 0)
        return matchmakeSession->IsMatchmakeOwner();
    return false;
}

unsigned short Session::GetMatchmakeSessionParticipantNum() const
{
    if (m_meshLayerController != 0)
        return m_meshLayerController->GetMatchmakeSessionTotalParticipants();
    return 0;
}

Result Session::CheckConnectionError() const
{
    switch (GetStatus())
    {
    case cStatusNone:
    case cStatusMatched:
    case cStatusUnknown2:
    case cStatusBufferMatched:
        return 0;
    case cStatusDisconnectedSession:
        return static_cast<Result>(cResultStatusDisconnectedSession);
    case cStatusUnknown6:
        return static_cast<Result>(cResultStatusUnknown6);
    case cStatusUnknown7:
        return static_cast<Result>(cResultStatusUnknown7);
    case cStatusUnknown8:
        return static_cast<Result>(cResultStatusUnknown8);
    case cStatusDisconnectedNetwork:
    default:
        return static_cast<Result>(cResultStatusDisconnectedNetwork);
    }
}

const ISessionInfo* Session::GetSessionInfo() const
{
    if (IsAsyncProcessCompleted(cAsyncProcessingNameCreateSession)
        || IsAsyncProcessCompleted(cAsyncProcessingNameJoinRandomSession)
        || IsAsyncProcessCompleted(cAsyncProcessingNameJoinSession)
        || IsAsyncProcessCompleted(cAsyncProcessingNameUnknown18))
    {
        IMatchmakeSession* matchmakeSession = m_matchmakeSession[m_currentSessionIndex];
        if (matchmakeSession != 0)
            return matchmakeSession->GetSessionInfo();
    }
    return 0;
}

unsigned int Session::GetJointSessionId() const
{
    if (m_jointSessionJob != 0 && m_matchmakeStatus == cMatchmakeStatusUnknown4)
        return m_sessionId[m_currentSessionIndex == 0 ? 1 : 0];
    return 0;
}

unsigned int Session::GetJointSessionOpenStatus() const
{
    unsigned int openStatus = cSessionOpenStatusClosed;
    if (IsJointSessionHost())
        openStatus = m_sessionOpenStatus[m_currentSessionIndex == 0 ? 1 : 0];
    return openStatus;
}

const ISessionInfo* Session::GetJointSessionInfo() const
{
    if (IsAsyncProcessCompleted(cAsyncProcessingNameUnknown19))
    {
        IMatchmakeSession* matchmakeSession = m_matchmakeSession[m_currentSessionIndex == 0 ? 1 : 0];
        if (matchmakeSession != 0)
            return matchmakeSession->GetSessionInfo();
    }
    return 0;
}

Result Session::RegisterSessionEventCallback(SessionEventCallback callback, void* param)
{
    if (m_sessionEventCallback != 0)
        return static_cast<Result>(transport::cResultInvalidState);
    m_sessionEventCallback = callback;
    m_sessionEventCallbackParam = param;
    return 0;
}

void Session::UnregisterSessionEventCallback()
{
    m_sessionEventCallback = 0;
    m_sessionEventCallbackParam = 0;
}

void Session::NoticeEventCallback(EventType eventType, StationIndex stationIndex)
{
    Session* session = g_piaSession;
    if (session != 0)
    {
        unsigned long long stationId = 0;
        if (transport::g_piaTransport->ConvertToStationId(&stationId, stationIndex) == 0)
            session->CallSessionEventCallback(eventType, stationId);
    }
}

unsigned int Session::GetSessionDisconnectReason() const
{
    return m_sessionDisconnectReason;
}

Session::ProcessType Session::GetJointSessionProcessType() const
{
    JointSessionJob* jointSessionJob = m_jointSessionJob;
    if (jointSessionJob == 0 || GetStatus() != cStatusUnknown2)
        return cProcessTypeNone;
    return jointSessionJob->m_processType;
}

void Session::SetHostMigrationOccurred(bool occurred)
{
    m_hostMigrationOccurred = occurred;
}

void Session::NotifyLeave(unsigned int sessionId, unsigned long long stationId)
{
    if (m_disconnectStatus - cConnectionStatusDisconnectedSession < 5)
        return;
    if (m_jointSessionJob != 0 && m_jointSessionJob->IsRunning())
        m_jointSessionJob->NoticeLeaveEventImpl(sessionId, stationId);
    if (m_sessionId[m_currentSessionIndex == 0 ? 1 : 0] != sessionId && m_sessionId[m_currentSessionIndex] != sessionId)
        return;
    if (m_matchmakeStatus == cMatchmakeStatusNone && stationId != m_localStationId)
        return;

    bool found = false;
    transport::StationConnectionInfo info;
    transport::StationManager* stationManager = transport::StationManager::s_pInstance;
    transport::Station** station = stationManager->m_stations.data;
    transport::Station** const end = station + stationManager->m_stations.size;
    if (transport::g_stationConnectionInfoTable->GetStationConnectionInfo(stationId, &info) == 0)
    {
        for (; station != end; ++station)
        {
            if ((*station)->m_stationId == stationId
                || info.m_stationLocation.m_stationAddress == (*station)->m_stationAddress
                || info.m_localStationLocation.m_stationAddress == (*station)->m_stationAddress)
            {
                found = true;
                if ((*station)->m_field20 == cStationLeaveStateRequested)
                    (*station)->m_field68 = 1;
                else
                    (*station)->m_field20 = cStationLeaveStateLeaving;
            }
        }
    }
    else
    {
        for (; station != end; ++station)
        {
            if ((*station)->m_stationId == stationId)
            {
                found = true;
                if ((*station)->m_field20 == cStationLeaveStateRequested)
                    (*station)->m_field68 = 1;
                else
                    (*station)->m_field20 = cStationLeaveStateLeaving;
            }
        }
    }
    if (!found)
        stationManager->Trace(cTraceMaskStationTable);
}

void Session::NotifyUnregister(unsigned int sessionId)
{
    if (m_joinSessionJob != 0 && m_joinSessionJob->IsRunning())
    {
        m_joinSessionJob->NoticeUnregisterEvent(sessionId);
        return;
    }
    if (m_randomMatchmakeJob != 0 && m_randomMatchmakeJob->IsRunning())
    {
        m_randomMatchmakeJob->NoticeUnregisterEvent(sessionId);
        return;
    }
    if ((m_leaveSessionJob == 0 || !m_leaveSessionJob->IsRunning())
        && m_disconnectStatus - cConnectionStatusDisconnectedSession >= 5 && m_jointSessionJob != 0
        && m_jointSessionJob->IsRunning())
        m_jointSessionJob->NoticeUnregisterEventImpl(sessionId);
}

bool Session::IsPlayersCountedAsParticipants() const
{
    if (m_meshLayerController == 0)
        return false;
    return m_meshLayerController->IsPlayersCountedAsParticipants();
}

unsigned char Session::GetLocalPlayerCount() const
{
    if (m_meshLayerController == 0)
        return 1;
    return m_meshLayerController->GetPlayerInfoSize();
}

unsigned int Session::IsPreparedForCleanup() const
{
    if (g_piaMesh != 0 && g_piaMesh->GetMonitoringDataSender() != 0)
        return g_piaMesh->GetMonitoringDataSender()->IsPreparedForCleanup();
    return true;
}

bool Session::AddNewStationId(unsigned long long stationId, StationIndex stationIndex, unsigned long long uniqueId,
                              unsigned int sessionId)
{
    Result result = transport::g_piaTransport->m_stationIdTable->Add(stationId, stationIndex, uniqueId);
    if (result == 0)
    {
        const bool postponeNotice = IsPostponeNotice();
        result = m_stationIdStatusTable->AddElement(stationId, sessionId, StationIdStatusTable::cPrepareStatusNone,
                                                    !postponeNotice);
        if (!postponeNotice)
        {
            validateStationId(stationId);
            JoinNotice(stationId);
        }
    }
    else
    {
        transport::g_piaTransport->m_stationIdTable->Trace(cTraceMaskStationTable);
    }
    return result == 0;
}

Result Session::Startup(const StartupSetting& setting)
{
    if (IsDuringSetup() != 0)
        return static_cast<Result>(0x10c08);
    if (m_startupState != 0)
        return static_cast<Result>(0x10c08);
    if (s_debugSetting.m_checkStartupSettingRange != 0 && setting.m_field00 - 1000 >= 29001)
        return static_cast<Result>(0x10c07);

    Result result = m_meshLayerController->Startup(setting);
    if (result != 0)
        return result;

    m_meshLayerController->IsHostMigrationEnabled();
    m_playerNum = 0;
    m_startupState = 1;
    m_participantNum = 0;
    m_sessionDisconnectReason = 0;

    result = m_sessionStatusCheckJob->Startup();
    if (result != 0)
        return result;

    m_sessionStatusCheckJob->Ready(false);
    m_disconnectStatus = 0;

    m_validStationIdList.Clear();

    g_piaMesh->SetSkipCleanupReport(true);
    return transport::g_piaTransport->Startup(m_meshLayerController->m_payloadSize, 0);
}

bool Session::IsValidStationId(unsigned long long stationId) const
{
    return m_stationIdStatusTable->CheckConnectStatus(stationId);
}

bool Session::IsHost() const
{
    if (m_localStationId == 0)
        return false;
    if (m_localStationId != m_hostStationId)
        return false;

    const IMatchmakeSession* matchmakeSession = m_matchmakeSession[m_currentSessionIndex];
    if (matchmakeSession != 0 && matchmakeSession->IsMatchmakeOwner())
        return true;
    return false;
}

bool Session::IsJointSessionHost() const
{
    if (m_jointSessionJob == 0)
        return false;
    if (m_localStationId == 0)
        return false;
    if (m_localStationId != m_hostStationId)
        return false;

    const IMatchmakeSession* currentSession = m_matchmakeSession[m_currentSessionIndex];
    if (currentSession == 0 || !currentSession->IsMatchmakeOwner())
        return false;
    if (m_localStationId != m_jointSessionHostStationId)
        return false;

    const IMatchmakeSession* bufferSession = m_matchmakeSession[m_currentSessionIndex == 0 ? 1 : 0];
    if (bufferSession != 0 && bufferSession->IsMatchmakeOwner())
        return true;
    return false;
}

unsigned int Session::GetStatus() const
{
    switch (m_disconnectStatus)
    {
    case cConnectionStatusDisconnectedSession:
        return cStatusDisconnectedSession;
    case cConnectionStatusDisconnectedNetwork:
        return cStatusDisconnectedNetwork;
    case cConnectionStatusUnknown4:
        return cStatusUnknown6;
    case cConnectionStatusUnknown5:
        return cStatusUnknown7;
    case cConnectionStatusUnknown6:
        return cStatusUnknown8;
    default:
        break;
    }
    switch (m_matchmakeStatus)
    {
    case cMatchmakeStatusMatched:
        return cStatusMatched;
    case cMatchmakeStatusUnknown3:
        return cStatusUnknown2;
    case cMatchmakeStatusUnknown4:
        return cStatusBufferMatched;
    default:
        break;
    }
    return cStatusNone;
}

int Session::GetJoinMeshState() const
{
    return g_piaMesh->GetJoinMeshState();
}

unsigned int Session::GetJoinRandomSessionStateForDebug() const
{
    unsigned int state = m_randomMatchmakeJob->GetJoinSessionStateForDebug();
    if (state == 4)
    {
        const unsigned int joinMeshState = GetJoinMeshState();
        if (joinMeshState < 4)
            state = s_joinRandomStateByJoinMeshState[joinMeshState];
        else
            state = 9;
    }
    return state;
}

unsigned int Session::GetJoinSessionStateForDebug() const
{
    unsigned int state = m_joinSessionJob->GetJoinSessionStateForDebug();
    if (state == 4)
    {
        const unsigned int joinMeshState = GetJoinMeshState();
        if (joinMeshState > 3)
            state = 9;
        else
            state = s_joinSessionStateByJoinMeshState[joinMeshState];
    }
    return state;
}

bool Session::IsProcessingJoin() const
{
    bool processing = false;
    if (m_randomMatchmakeJob != 0)
        processing = m_randomMatchmakeJob->IsRunning();
    if (m_joinSessionJob != 0)
        processing |= m_joinSessionJob->IsRunning();
    return processing;
}

unsigned int Session::GetSessionOpenStatus() const
{
    if (!IsHost())
        return cSessionOpenStatusClosed;
    return m_sessionOpenStatus[m_currentSessionIndex];
}

void Session::SetCurrentSessionOpenStatus(SessionOpenStatus status)
{
    m_sessionOpenStatus[m_currentSessionIndex] = static_cast<unsigned int>(status);
}

void Session::SetBufferSessionOpenStatus(SessionOpenStatus status)
{
    const unsigned int bufferIndex = m_currentSessionIndex == 0 ? 1 : 0;
    m_sessionOpenStatus[bufferIndex] = static_cast<unsigned int>(status);
}

bool Session::UpdateStationIdToBecomeHost()
{
    if (m_sessionId[0] == 0)
        return false;
    g_piaMesh->GetLocalStationIndex();
    const unsigned long long principalId = transport::g_stationConnectionInfoTable->GetPrincipalIdByStation(
        transport::StationManager::s_pInstance->m_localStation);
    if (principalId == 0)
        return false;
    m_localStationId = principalId;
    m_hostStationId = principalId;
    if (m_matchmakeStatus == cMatchmakeStatusMatched)
        return true;
    return false;
}

void Session::CheckStatusAtChangeMeshState()
{
    const common::Job::ExecuteResult result = m_sessionStatusCheckJob->CheckSessionStatus4JointSession();
    if (result.m_kind != common::Job::cExecuteKindNextStep)
        m_sessionStatusCheckJob->Reset(false);
}

void Session::ChangeStatusToDisconnectedSession()
{
    m_disconnectStatus = cConnectionStatusDisconnectedSession;
    m_hostStationId = 0;
    m_jointSessionHostStationId = 0;
    CheckStatusAtChangeMeshState();
    if (m_sessionDisconnectReason == 0)
        m_sessionDisconnectReason = 6;
}

unsigned short Session::GetParticipantNum() const
{
    return m_participantNum;
}

bool Session::IsHostMigrationEnabled() const
{
    if (m_meshLayerController != 0 && m_meshLayerController->IsStartedUp())
        return m_meshLayerController->IsHostMigrationEnabled();
    return false;
}

unsigned int Session::GetMatchmakeSessionStationNum() const
{
    if (m_meshLayerController != 0)
        return m_meshLayerController->GetMatchmakeSessionTotalStations();
    return 0;
}

Session::DisconnectionEventListener Session::GetDisconnectionEventListener()
{
    return DisconnectionEventListenerForSession;
}

MeshEventListener* Session::GetMeshEventListenerPtr()
{
    return m_meshEventListener;
}

int DisconnectionEventListenerForSession(void* event)
{
    return g_piaSession->m_stationIdStatusTable->DisconnectionEventListenerForJointSession(event);
}

unsigned int Session::GetSessionIdCallback()
{
    Session* session = g_piaSession;
    unsigned int sessionId = 0;
    if (session != 0)
        sessionId = session->m_sessionId[session->m_currentSessionIndex];
    return sessionId;
}

unsigned char Session::CheckHostCandidateRankingCallback(StationIndex index, bool checkHost)
{
    if (!g_piaSession->IsHostMigrationEnabled())
        return 0;
    JointSessionJob* jointSessionJob = g_piaSession->m_jointSessionJob;
    if (jointSessionJob == 0)
        return 0;
    if (g_piaSession->m_matchmakeStatus != cMatchmakeStatusUnknown4
        && (g_piaSession->m_matchmakeStatus != cMatchmakeStatusUnknown3 || jointSessionJob->m_fielda3 == 0))
        return 0;
    transport::g_piaTransport->m_stationIdTable->Trace(cTraceMaskStationTable);
    g_piaSession->m_stationIdStatusTable->Trace(cTraceMaskStationTable);
    transport::StationIdTable::Entry entry;
    if (transport::g_piaTransport->m_stationIdTable->FindByStationIndex(&entry, index) != 0)
        return cRankingUnknownStation;
    unsigned int entrySessionId = 0;
    if (!g_piaSession->m_stationIdStatusTable->GetSessionId(entry.stationId, &entrySessionId))
        return cRankingUnknownSession;
    if (entrySessionId == g_piaSession->m_field110)
        return 0;
    const unsigned char rank = g_piaSession->m_stationIdStatusTable->GetSessionIdRank(entrySessionId);
    bool isSessionHost = false;
    if (checkHost)
        g_piaSession->m_stationIdStatusTable->GetSessionHost(entry.stationId, &isSessionHost);
    if (entry.stationId != g_piaSession->m_hostStationId && !isSessionHost)
        return static_cast<unsigned char>(rank * 4 + 1);
    return static_cast<unsigned char>((rank & 0x3f) << 2);
}

unsigned short Session::GetValidStationIdNum() const
{
    return static_cast<unsigned short>(m_validStationIdList.GetNodeNum());
}

unsigned short Session::GetStationNum() const
{
    return GetValidStationIdNum();
}

unsigned long long Session::GetJointSessionHostStationId() const
{
    if (m_jointSessionJob == 0)
        return 0;
    return m_jointSessionHostStationId;
}

void Session::NotifyOwnershipChange(unsigned int sessionId, unsigned long long stationId)
{
    if (m_joinSessionJob != 0 && m_joinSessionJob->IsRunning())
    {
        m_joinSessionJob->NoticeOwnerChangeEvent(sessionId, stationId);
        return;
    }
    if (m_randomMatchmakeJob != 0 && m_randomMatchmakeJob->IsRunning())
    {
        m_randomMatchmakeJob->NoticeOwnerChangeEvent(sessionId, stationId);
        return;
    }
    if (m_leaveSessionJob != 0 && m_leaveSessionJob->IsRunning())
        return;
    if (m_disconnectStatus - cConnectionStatusDisconnectedSession < 5)
        return;

    const unsigned char bufferIndex = m_currentSessionIndex == 0 ? 1 : 0;
    if (sessionId != m_sessionId[m_currentSessionIndex] && sessionId != m_sessionId[bufferIndex])
        return;

    transport::StationIdTable* stationIdTable = transport::g_piaTransport->m_stationIdTable;
    transport::StationIdTable::Entry entry;
    if (stationIdTable->FindByUniqueId(&entry, stationId) != 0)
    {
        transport::IdentificationInfo info;
        if (transport::IdentificationInfoTable::GetInstance()->GetIdentificationInfo(stationId, &info) == 0)
            return;
        if (!g_piaMesh->GetProcessHostMigrationJobPtr()->m_started)
        {
            if (m_sessionId[m_currentSessionIndex] == sessionId)
            {
                if (m_jointSessionJob == 0 || m_jointSessionJob->m_processType == cProcessTypeNone)
                {
                    ChangeStatusToDisconnectedSession();
                    return;
                }
                m_jointSessionJob->m_failurePending = 1;
                m_jointSessionJob->m_pendingFailureResult = cResultOwnerChanged;
            }
        }
        else if (g_piaMesh->GetProcessHostMigrationJobPtr()->m_unknownA5)
        {
            g_piaMesh->GetProcessHostMigrationJobPtr()->CheckWhetherNewHostIsUnknown(stationId);
        }
    }

    if (m_sessionId[m_currentSessionIndex] != sessionId)
    {
        if (m_matchmakeStatus != cMatchmakeStatusUnknown4)
        {
            if (m_matchmakeStatus != cMatchmakeStatusUnknown3)
                return;
            if (m_jointSessionHostStationId == stationId || m_jointSessionHostStationId != m_localStationId)
            {
                m_jointSessionJob->NoticeOwnerChangeEventImpl(sessionId, stationId);
                return;
            }
            if (m_jointSessionJob == 0 || m_jointSessionJob->m_processType == cProcessTypeNone)
            {
                ChangeStatusToDisconnectedSession();
                return;
            }
            if (m_jointSessionJob->m_processType != cProcessTypeDestroyJointSession
                && m_jointSessionJob->m_processType != cProcessTypeLeaveJointSession)
            {
                m_jointSessionJob->m_failurePending = 1;
                m_jointSessionJob->m_pendingFailureResult = cResultOwnerChanged;
                m_jointSessionJob->NoticeOwnerChangeEventImpl(sessionId, stationId);
                return;
            }
            if (stationIdTable->FindByUniqueId(&entry, stationId) == 0)
            {
                unsigned int entrySessionId = 0;
                m_stationIdStatusTable->GetSessionId(entry.stationId, &entrySessionId);
                if (entrySessionId == m_sessionId[m_currentSessionIndex])
                {
                    m_jointSessionJob->m_failurePending = 1;
                    m_jointSessionJob->m_pendingFailureResult = cResultOwnerChanged;
                    m_jointSessionJob->NoticeOwnerChangeEventImpl(sessionId, stationId);
                    return;
                }
            }
            m_jointSessionJob->NoticeOwnerChangeEventImpl(sessionId, stationId);
            return;
        }
        bool isJointSessionHost;
        if (m_jointSessionHostStationId == stationId)
        {
            if (stationIdTable->FindByUniqueId(&entry, stationId) != 0)
                return;
            unsigned int entrySessionId = 0;
            if (!m_stationIdStatusTable->GetSessionId(entry.stationId, &entrySessionId))
                entrySessionId = 0;
            m_field110 = entrySessionId;
            isJointSessionHost = IsJointSessionHost();
        }
        else
        {
            if (m_jointSessionHostStationId == m_localStationId)
            {
                if (m_jointSessionJob == 0 || m_jointSessionJob->m_processType == cProcessTypeNone)
                {
                    ChangeStatusToDisconnectedSession();
                    return;
                }
                m_jointSessionJob->m_failurePending = 1;
                m_jointSessionJob->m_pendingFailureResult = cResultOwnerChanged;
            }
            isJointSessionHost = IsJointSessionHost();
        }
        if (!isJointSessionHost)
            return;
        CallSessionEventCallback(cEventTypeUnknown04, GetJointSessionHostStationId());
    }
    else if (m_matchmakeStatus == cMatchmakeStatusUnknown4)
    {
        if (m_field110 != sessionId)
        {
            if (m_hostStationId != stationId)
            {
                if (stationIdTable->FindByStationId(&entry, m_hostStationId) != 0)
                {
                    ProcessHostMigrationJob* migrationJob = g_piaMesh->GetProcessHostMigrationJobPtr();
                    if (migrationJob->m_started)
                    {
                        migrationJob = g_piaMesh->GetProcessHostMigrationJobPtr();
                        if (!migrationJob->m_started)
                            return;
                        migrationJob = g_piaMesh->GetProcessHostMigrationJobPtr();
                        if (!migrationJob->m_unknownB8)
                            return;
                    }
                    m_hostStationId = stationId;
                    if (m_localStationId == stationId)
                    {
                        m_matchmakeSession[m_currentSessionIndex]->UpdateMatchmakeSessionHost(sessionId);
                        stationId = m_hostStationId;
                    }
                    CallSessionEventCallback(cEventTypeUnknown02, stationId);
                    return;
                }
                if (m_localStationId != m_hostStationId)
                    return;
            }
            ChangeStatusToDisconnectedSession();
            return;
        }
        const bool isJointSessionHost = IsJointSessionHost();
        if (isJointSessionHost)
            CallSessionEventCallback(cEventTypeUnknown04, GetJointSessionHostStationId());
        if (!IsHost())
        {
            if (!isJointSessionHost)
                return;
        }
        else
        {
            CallSessionEventCallback(cEventTypeUnknown02, m_hostStationId);
        }
    }
    else
    {
        if (m_matchmakeStatus != cMatchmakeStatusMatched)
        {
            if (m_matchmakeStatus != cMatchmakeStatusUnknown3)
                return;
            if (m_jointSessionJob == 0 || !m_jointSessionJob->IsRunning())
                return;
            if (m_hostStationId != stationId && m_localStationId == m_hostStationId)
            {
                m_jointSessionJob->m_failurePending = 1;
                m_jointSessionJob->m_pendingFailureResult = cResultOwnerChanged;
                m_jointSessionJob->NoticeOwnerChangeEventImpl(sessionId, stationId);
                return;
            }
            if (m_jointSessionJob->m_processType == cProcessTypeDestroyJointSession)
            {
                unsigned int entrySessionId;
                bool needsNotice;
                if (!m_stationIdStatusTable->GetSessionId(m_jointSessionHostStationId, &entrySessionId))
                {
                    entrySessionId = 0;
                    needsNotice = sessionId != 0;
                }
                else
                {
                    needsNotice = entrySessionId != sessionId;
                }
                if (needsNotice)
                {
                    if (stationIdTable->FindByStationId(&entry, m_hostStationId) != 0
                        || !m_stationIdStatusTable->CheckConnectStatus(m_hostStationId))
                    {
                        if (m_localStationId == stationId)
                            m_matchmakeSession[m_currentSessionIndex]->UpdateMatchmakeSessionHost(sessionId);
                        m_jointSessionJob->NoticeHostChangeEventImpl(stationId);
                    }
                }
            }
            m_jointSessionJob->NoticeOwnerChangeEventImpl(sessionId, stationId);
            return;
        }
        if (!IsHost())
        {
            if (m_hostStationId != m_localStationId)
                return;
            if (m_hostStationId == stationId)
                return;
            ChangeStatusToDisconnectedSession();
            return;
        }
        CallSessionEventCallback(cEventTypeUnknown02, m_hostStationId);
    }
    if (m_postponeNotice)
    {
        m_postponeNotice = false;
        m_stationIdStatusTable->ClearDisconnectedStationId();
        m_stationIdStatusTable->CallJoinEventNotYetNotified();
    }
}

void Session::CancelJoinProcessByFatalError()
{
    if (m_randomMatchmakeJob != 0 && m_randomMatchmakeJob->IsRunning())
        m_randomMatchmakeJob->m_sessionInvalidated = true;
    if (m_joinSessionJob != 0 && m_joinSessionJob->IsRunning())
        m_joinSessionJob->m_sessionInvalidated = true;
}

bool Session::IsPostponeNotice() const
{
    return m_postponeNotice;
}

void Session::invalidateStationId(unsigned long long stationId)
{
    common::ListNode* sentinel = m_validStationIdList.Sentinel();
    for (common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        if (*StationIdList::ValueOf(node) == stationId)
        {
            m_validStationIdList.Free(static_cast<StationIdList::Node*>(node));
            break;
        }
    }
    UpdateSessionStationInfo();
}

bool Session::RemoveStationId(unsigned long long stationId, StationIndex stationIndex, unsigned long long uniqueId,
                              unsigned int)
{
    if (uniqueId != 0)
    {
        transport::StationIdTable::Entry entry;
        if (transport::g_piaTransport->m_stationIdTable->FindByUniqueId(&entry, uniqueId) != 0)
            return false;
        stationId = entry.stationId;
        if (stationId == 0)
            return false;
    }
    else if (stationId == 0)
    {
        if (stationIndex == cStationIndexInvalid)
            return false;
        transport::StationIdTable::Entry entry;
        if (transport::g_piaTransport->m_stationIdTable->FindByStationIndex(&entry, stationIndex) != 0)
            return false;
        stationId = entry.stationId;
        if (stationId == 0)
            return false;
    }

    if (IsPostponeNotice())
    {
        m_stationIdStatusTable->SetConnect(stationId, false);
        return true;
    }

    if (m_stationIdStatusTable->CheckEventNotifiedStatus(stationId))
    {
        if (m_sessionEventCallback != 0)
            m_sessionEventCallback(cEventTypeLeave, stationId, m_sessionEventCallbackParam);
        invalidateStationId(stationId);
    }
    transport::g_piaTransport->m_stationIdTable->RemoveByStationId(stationId);
    Result result = m_stationIdStatusTable->RemoveElement(stationId);
    return result == 0;
}

void Session::JoinNotice(unsigned long long stationId)
{
    if (m_sessionEventCallback != 0)
        m_sessionEventCallback(cEventTypeJoin, stationId, m_sessionEventCallbackParam);
    transport::g_piaTransport->m_stationIdTable->SetJoinEventNotifiedByStationId(stationId, true);
}

void Session::validateStationId(unsigned long long stationId)
{
    unsigned long long* validStationId = m_validStationIdList.Alloc();
    if (validStationId != 0)
        *validStationId = stationId;
    UpdateSessionStationInfo();
}

void Session::UpdateSessionStationInfo()
{
    const unsigned int status = GetStatus();
    if ((status != cStatusMatchedCurrent && status != cStatusMatchedBuffer) || m_leaveSessionJob->IsRunning())
        return;
    common::ListNode* sentinel = m_validStationIdList.Sentinel();
    unsigned short playerNum = 0;
    for (common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        transport::Station* station = transport::StationManager::s_pInstance->GetStation(*StationIdList::ValueOf(node));
        unsigned char count;
        if (station != 0 && station->GetPlayerCount(&count) == 0)
            playerNum += count;
    }
    m_playerNum = playerNum;
    unsigned short participantNum = 0;
    for (common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        transport::Station* station = transport::StationManager::s_pInstance->GetStation(*StationIdList::ValueOf(node));
        unsigned char count;
        if (station != 0 && station->GetParticipantCount(&count) == 0)
            participantNum += count;
    }
    m_participantNum = participantNum;
    if (GetStatus() == cStatusMatchedCurrent)
        m_matchmakeSession[m_currentSessionIndex]->UpdateStationInfoList();
    else if (GetStatus() == cStatusMatchedBuffer)
        m_matchmakeSession[m_currentSessionIndex == 0 ? 1 : 0]->UpdateStationInfoList();
}

void Session::ChangeStationIdConvertEnableFlag(bool enable)
{
    if (!enable)
        transport::g_piaTransport->SetTransportStatus(cResultStationIdConvertDisabled);
    else
        transport::g_piaTransport->SetTransportStatus(0);
}

unsigned int Session::GetJointSessionStateForDebug() const
{
    if (m_jointSessionJob != 0)
        return m_jointSessionJob->GetJointSessionStateForDebug();
    return 0;
}

bool Session::UpdateStationIdToBecomeClient()
{
    g_piaMesh->GetLocalStationIndex();
    const unsigned long long localPrincipalId = transport::g_stationConnectionInfoTable->GetPrincipalIdByStation(
        transport::StationManager::s_pInstance->m_localStation);
    if (localPrincipalId == 0)
        return false;

    m_localStationId = localPrincipalId;
    const unsigned long long hostPrincipalId = transport::g_stationConnectionInfoTable->GetPrincipalIdByStation(
        transport::StationManager::s_pInstance->GetStation(g_piaMesh->GetHostStationIndex()));
    if (hostPrincipalId == 0)
        return false;

    m_field110 = 0;
    if (m_matchmakeStatus == 4)
        return false;
    m_hostStationId = hostPrincipalId;
    return true;
}

void Session::ChangeStatusToDisconnectedNetwork()
{
    m_disconnectStatus = cConnectionStatusDisconnectedNetwork;
    m_hostStationId = 0;
    m_jointSessionHostStationId = 0;
    CheckStatusAtChangeMeshState();
    if (m_sessionDisconnectReason == 0)
        m_sessionDisconnectReason = 8;
}

void Session::CallSessionEventCallback(EventType eventType, unsigned long long stationId)
{
    if ((eventType != cEventTypeUnknown03 && eventType != cEventTypeUnknown02)
        || (!IsProcessingJoin() && !m_leaveSessionJob->IsRunning() && m_disconnectStatus - 2 > 4))
    {
        if (m_sessionEventCallback != 0)
            m_sessionEventCallback(eventType, stationId, m_sessionEventCallbackParam);
    }
}

transport::Station::IdentificationToken* Session::GetMatchmakeAnalysisToken()
{
    return &m_matchmakeAnalysisToken;
}

bool Session::SessionJoiningApprovalCallback(const transport::Station::IdentificationToken* identificationToken)
{
    if (g_piaSession->m_joiningApprovalCallback == 0)
        return true;
    return g_piaSession->m_joiningApprovalCallback(identificationToken);
}

void Session::SetSyncClockPulseInterval(int interval)
{
    if (g_piaMesh != 0)
        g_piaMesh->SetSyncClockPulseInterval(interval);
}

void Session::ResetInternalAsyncState()
{
    if (m_asyncProcessingName != 0 && m_callContext.m_state != common::CallContext::cStateInProgress)
    {
        m_callContext.Reset();
        m_asyncProcessingName = cAsyncProcessingNameNone;
    }
}

Result Session::JoinSessionAsync(const JoinSessionSetting* setting)
{
    if (setting == 0)
        return static_cast<Result>(0x10c07);
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(0x10c08);
    if (m_startupState != 1)
        return static_cast<Result>(0x10c08);
    if (m_matchmakeStatus != cMatchmakeStatusNone)
        return static_cast<Result>(0x10c08);

    ResetInternalAsyncState();
    Result result = m_joinSessionJob->Startup(&m_callContext, setting);
    if (result == 0)
    {
        m_joinSessionJob->Ready(false);
        m_asyncProcessingName = cAsyncProcessingNameJoinSession;
    }
    return result;
}

Result Session::UpdateSessionSettingAsync(const UpdateSessionSetting* setting)
{
    IMatchmakeSession* matchmakeSession = m_matchmakeSession[m_currentSessionIndex];
    unsigned int sessionId = m_sessionId[m_currentSessionIndex];
    Result result = UpdateSessionSettingAsyncCore(sessionId, matchmakeSession, setting, false);
    if (result == 0)
        m_asyncProcessingName = cAsyncProcessingNameUpdateSessionSetting;
    return result;
}

Result Session::UpdateSessionSettingAsyncCore(unsigned int sessionId, IMatchmakeSession* matchmakeSession,
                                              const UpdateSessionSetting* setting, bool isJointSession)
{
    if (m_callContext.m_state == common::CallContext::cStateInProgress)
        return static_cast<Result>(0x10c08);
    if (m_startupState != 1)
        return static_cast<Result>(0x10c08);
    if (m_updateSessionSettingJob->IsRunning())
        return static_cast<Result>(0x10c08);

    ResetInternalAsyncState();
    if (m_updateSessionSettingJob == 0)
        return static_cast<Result>(0x10c08);

    Result result = m_updateSessionSettingJob->Startup(&m_callContext, sessionId, matchmakeSession, setting,
                                                       isJointSession);
    if (result == 0)
        m_updateSessionSettingJob->Ready(false);
    return result;
}

}
}
}
