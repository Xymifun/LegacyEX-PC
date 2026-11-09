#include "nn/pia/inet/NexMatchMeshLayerController.hpp"

#include <coreinit/memory.h>

#include "nn/nex/NgsBridgeInterface.hpp"
#include "nn/nex/NgsFacade.hpp"
#include "nn/nex/qResult.hpp"
#include "nn/pia/NetworkFactory.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/inet/NexFacade.hpp"
#include "nn/pia/inet/NexMatchmakeSession.hpp"
#include "nn/pia/inet/NexNatRelay.hpp"
#include "nn/pia/inet/NexSessionStartupSetting.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/common/Crypto.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/JointSessionJob.hpp"
#include "nn/nex/NotificationEvent.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

namespace
{

enum
{
    cNexResultNotConnected = static_cast<int>(0x800300D2u),
    cResultBufferFull = 0x4c0d,
    cResultOwnerChanged = 0x6c59,
    cNotificationTypeCategoryUnit = 1000,
    cNotificationTypeParticipation = 3,
    cNotificationTypeOwnershipChange = 4,
    cNotificationTypeGatheringUnregistered = 109,
    cNotificationTypeHostChange = 110,
    cNotificationTypeSystemPasswordChange = 120,
    cNotificationTypeSystemPasswordClear = 121,
    cNotificationTypeSwitchGathering = 122,
    cParticipationEventParticipate = 1,
    cParticipationEventDisconnect = 7,
    cParticipationEventEndParticipation = 8,
    cParticipantTypePlayer = 1,
    cDecimalDigitNum = 10,
    cParticipationTextStationIdLength = 67,
    cParticipationTextStationIdOffset = 2,
    cStationIdTextLength = 16,
    cMatchmakeStatusJointSessionProcessing = 3,
    cExpiredParticipantTimeoutMsec = 500,
    cPayloadSizeMin = 576,
    cPayloadSizeMax = 1364,
    cNgsInterfaceIndexInvalid = 0xff,
    cRelayGatheringIdSize = 4,
    cPlayerNameTraceFlag = 0x4000000
};

const unsigned int cByteMaskEven = 0xff00ff00;
const unsigned int cByteMaskOdd = 0x00ff00ff;

inline unsigned int SwapU32(unsigned int value)
{
    unsigned int swapped = ((value & cByteMaskEven) >> 8) | ((value & cByteMaskOdd) << 8);
    return (swapped << 16) | (swapped >> 16);
}

}

NexMatchMeshLayerController::NexNotificationEventHandler::NexNotificationEventHandler()
    : m_unregisterGathering(false),
      m_sessionParticipantInfo(),
      m_ignoredSessionId()
{
    AllClearSessionId();
    AllClearIgnoredSessionId();
}

NexMatchMeshLayerController::NexNotificationEventHandler::~NexNotificationEventHandler()
{
    Cleanup();
}

void NexMatchMeshLayerController::NexNotificationEventHandler::AllClearSessionId()
{
    for (unsigned int index = 0; index < cSessionIdArraySize; ++index)
        m_sessionParticipantInfo[index].Cleanup();
}

void NexMatchMeshLayerController::NexNotificationEventHandler::AllClearIgnoredSessionId()
{
    m_ignoredSessionId.m_num = 0;
    m_ignoredSessionId.m_head = 0;
}

void NexMatchMeshLayerController::NexNotificationEventHandler::Cleanup()
{
    m_unregisterGathering = false;
    AllClearSessionId();
    AllClearIgnoredSessionId();
}

bool NexMatchMeshLayerController::NexNotificationEventHandler::IsUnregisterGathering() const
{
    return m_unregisterGathering;
}

bool NexMatchMeshLayerController::NexNotificationEventHandler::IsIgnoredSessionId(unsigned int sessionId) const
{
    for (unsigned int index = 0; index < m_ignoredSessionId.m_num; ++index)
    {
        const unsigned int ignoredSessionId =
            m_ignoredSessionId.m_sessionId[(m_ignoredSessionId.m_head + index) % cIgnoredSessionIdNum];
        if (ignoredSessionId != 0 && ignoredSessionId == sessionId)
            return true;
    }
    return false;
}

void NexMatchMeshLayerController::NexNotificationEventHandler::AddIgnoredSessionId(unsigned int sessionId)
{
    if (IsIgnoredSessionId(sessionId))
        return;
    m_ignoredSessionId.m_sessionId[(m_ignoredSessionId.m_head + m_ignoredSessionId.m_num) % cIgnoredSessionIdNum] =
        sessionId;
    if (m_ignoredSessionId.m_num == cIgnoredSessionIdNum)
        m_ignoredSessionId.m_head = (m_ignoredSessionId.m_head + 1) % cIgnoredSessionIdNum;
    else
        ++m_ignoredSessionId.m_num;
}

void NexMatchMeshLayerController::NexNotificationEventHandler::RemoveIgnoredSessionId(unsigned int sessionId)
{
    if (!IsIgnoredSessionId(sessionId))
        return;
    for (unsigned int index = 0; index < m_ignoredSessionId.m_num; ++index)
    {
        unsigned int& ignoredSessionId =
            m_ignoredSessionId.m_sessionId[(m_ignoredSessionId.m_head + index) % cIgnoredSessionIdNum];
        if (ignoredSessionId == sessionId)
        {
            ignoredSessionId = 0;
            return;
        }
    }
}

Result NexMatchMeshLayerController::NexNotificationEventHandler::SaveSessionId(unsigned int sessionId)
{
    unsigned int emptyIndex = cSessionIdArraySize;
    for (unsigned int index = 0; index < cSessionIdArraySize; ++index)
    {
        if (m_sessionParticipantInfo[index].m_sessionId == sessionId)
            return transport::cResultAlreadyExists;
        if (emptyIndex == cSessionIdArraySize && m_sessionParticipantInfo[index].m_sessionId == 0)
            emptyIndex = index;
    }
    if (emptyIndex == cSessionIdArraySize)
        return cResultBufferFull;
    m_sessionParticipantInfo[emptyIndex].m_sessionId = sessionId;
    m_sessionParticipantInfo[emptyIndex].m_participantList.Clear();
    return 0;
}

bool NexMatchMeshLayerController::NexNotificationEventHandler::CheckSessionId(unsigned int sessionId) const
{
    for (unsigned int index = 0; index < cSessionIdArraySize; ++index)
    {
        if (m_sessionParticipantInfo[index].m_sessionId == sessionId)
            return true;
    }
    return false;
}

void NexMatchMeshLayerController::NexNotificationEventHandler::ClearSessionId(unsigned int sessionId)
{
    for (unsigned int index = 0; index < cSessionIdArraySize; ++index)
    {
        if (m_sessionParticipantInfo[index].m_sessionId == sessionId)
            m_sessionParticipantInfo[index].Cleanup();
    }
    AddIgnoredSessionId(sessionId);
}

unsigned int NexMatchMeshLayerController::NexNotificationEventHandler::GetSessionId(unsigned int index) const
{
    if (index >= cSessionIdArraySize)
        return 0;
    return m_sessionParticipantInfo[index].m_sessionId;
}

SessionParticipantInfo* NexMatchMeshLayerController::NexNotificationEventHandler::GetSessionParticipantInfo(
    unsigned int sessionId)
{
    for (unsigned int index = 0; index < cSessionIdArraySize; ++index)
    {
        if (m_sessionParticipantInfo[index].m_sessionId == sessionId)
            return &m_sessionParticipantInfo[index];
    }
    return 0;
}

const SessionParticipantInfo* NexMatchMeshLayerController::NexNotificationEventHandler::GetSessionParticipantInfo(
    unsigned int sessionId) const
{
    for (unsigned int index = 0; index < cSessionIdArraySize; ++index)
    {
        if (m_sessionParticipantInfo[index].m_sessionId == sessionId)
            return &m_sessionParticipantInfo[index];
    }
    return 0;
}

unsigned short NexMatchMeshLayerController::NexNotificationEventHandler::GetMatchmakeSessionTotalStations(
    unsigned int sessionId) const
{
    unsigned short stationNum = 0;
    if (session::g_piaSession != 0)
    {
        const SessionParticipantInfo* info = GetSessionParticipantInfo(sessionId);
        if (info != 0)
        {
            typedef SessionParticipantInfo::ParticipantList::Node ParticipantNode;
            const common::ListNode* end = info->m_participantList.Sentinel();
            for (const ParticipantNode* participantNode = static_cast<const ParticipantNode*>(end->next);
                 participantNode != end; participantNode = static_cast<const ParticipantNode*>(participantNode->next))
            {
                if (participantNode->value.m_principalId != 0
                    && participantNode->value.m_principalId == participantNode->value.m_stationId
                    && participantNode->value.m_participationCount > 0)
                    ++stationNum;
            }
        }
    }
    return stationNum;
}

unsigned short NexMatchMeshLayerController::NexNotificationEventHandler::GetMatchmakeSessionTotalParticipants(
    unsigned int sessionId) const
{
    unsigned short participantNum = 0;
    if (session::g_piaSession != 0)
    {
        const SessionParticipantInfo* info = GetSessionParticipantInfo(sessionId);
        if (info != 0)
        {
            typedef SessionParticipantInfo::ParticipantList::Node ParticipantNode;
            const common::ListNode* end = info->m_participantList.Sentinel();
            for (const ParticipantNode* participantNode = static_cast<const ParticipantNode*>(end->next);
                 participantNode != end; participantNode = static_cast<const ParticipantNode*>(participantNode->next))
            {
                if (participantNode->value.m_principalId != 0 && participantNode->value.m_participationCount > 0
                    && participantNode->value.m_participantType == cParticipantTypePlayer)
                    ++participantNum;
            }
        }
    }
    return participantNum;
}

bool NexMatchMeshLayerController::NexNotificationEventHandler::IsParticipated(unsigned int sessionId,
                                                                              unsigned long long principalId) const
{
    bool participated = false;
    const SessionParticipantInfo* info = GetSessionParticipantInfo(sessionId);
    if (info != 0)
    {
        typedef SessionParticipantInfo::ParticipantList::Node ParticipantNode;
        const common::ListNode* end = info->m_participantList.Sentinel();
        for (const ParticipantNode* participantNode = static_cast<const ParticipantNode*>(end->next);
             participantNode != end; participantNode = static_cast<const ParticipantNode*>(participantNode->next))
        {
            if (participantNode->value.m_principalId == principalId && participantNode->value.m_participationCount > 0)
            {
                participated = true;
                break;
            }
        }
    }
    return participated;
}

void NexMatchMeshLayerController::NexNotificationEventHandler::AddParticipantPrincipalId(unsigned int sessionId,
                                                                                         unsigned long long principalId,
                                                                                         unsigned long long stationId,
                                                                                         unsigned char participantType)
{
    SessionParticipantInfo* info = GetSessionParticipantInfo(sessionId);
    if (info == 0)
    {
        if (SaveSessionId(sessionId) != 0)
            return;
        info = GetSessionParticipantInfo(sessionId);
        if (info == 0)
            return;
    }
    SessionParticipantInfo::ParticipantList& list = info->m_participantList;
    common::ListNode* end = list.Sentinel();
    common::ListNode* node = end->next;
    common::ListNode* next = node->next;
    while (node != end)
    {
        SessionParticipantInfo::ParticipantList::Node* participantNode =
            static_cast<SessionParticipantInfo::ParticipantList::Node*>(node);
        if (static_cast<long long>(common::g_piaJobScheduler->m_dispatchTime.m_tick
                                   - participantNode->value.m_updateTime.m_tick)
                > common::TimeSpan::GetTicksPerMSec().m_tick * cExpiredParticipantTimeoutMsec
            && participantNode->value.m_participationCount < 0
            && (participantNode->value.m_participationEvent == cParticipationEventDisconnect
                || participantNode->value.m_participationEvent == cParticipationEventEndParticipation))
            list.Free(participantNode);
        node = next;
        next = node->next;
    }
    for (node = end->next; node != end; node = node->next)
    {
        SessionParticipantInfo::Participant* participant = SessionParticipantInfo::ParticipantList::ValueOf(node);
        if (participant->m_principalId == principalId)
        {
            participant->m_stationId = stationId;
            participant->m_participantType = participantType;
            if (participant->m_participationEvent == cParticipationEventDisconnect)
            {
                participant->m_participationCount = 1;
                participant->m_participationEvent = cParticipationEventParticipate;
                participant->m_updateTime = common::g_piaJobScheduler->m_dispatchTime;
                return;
            }
            participant->m_participationEvent = cParticipationEventParticipate;
            ++participant->m_participationCount;
            participant->m_updateTime = common::g_piaJobScheduler->m_dispatchTime;
            if (participant->m_participationCount == 0)
                list.Destroy(participant);
            return;
        }
    }
    SessionParticipantInfo::Participant* participant = list.Alloc();
    if (participant != 0)
    {
        participant->m_principalId = principalId;
        participant->m_stationId = stationId;
        participant->m_participationEvent = cParticipationEventParticipate;
        participant->m_participantType = participantType;
        participant->m_participationCount = 1;
        participant->m_leaving = false;
        participant->m_updateTime = common::g_piaJobScheduler->m_dispatchTime;
    }
}

void NexMatchMeshLayerController::NexNotificationEventHandler::RemoveParticipantPrincipalId(
    unsigned int sessionId, unsigned long long principalId, short participationEvent)
{
    SessionParticipantInfo* info = GetSessionParticipantInfo(sessionId);
    if (info == 0)
    {
        if (IsIgnoredSessionId(sessionId))
            return;
        if (SaveSessionId(sessionId) != 0)
            return;
        info = GetSessionParticipantInfo(sessionId);
        if (info == 0)
            return;
    }
    SessionParticipantInfo::ParticipantList& list = info->m_participantList;
    common::ListNode* end = list.Sentinel();
    common::ListNode* node = end->next;
    common::ListNode* next = node->next;
    while (node != end)
    {
        SessionParticipantInfo::ParticipantList::Node* participantNode =
            static_cast<SessionParticipantInfo::ParticipantList::Node*>(node);
        if (static_cast<long long>(common::g_piaJobScheduler->m_dispatchTime.m_tick
                                   - participantNode->value.m_updateTime.m_tick)
                > common::TimeSpan::GetTicksPerMSec().m_tick * cExpiredParticipantTimeoutMsec
            && participantNode->value.m_participationCount < 0
            && (participantNode->value.m_participationEvent == cParticipationEventDisconnect
                || participantNode->value.m_participationEvent == cParticipationEventEndParticipation))
            list.Free(participantNode);
        node = next;
        next = node->next;
    }
    for (node = end->next; node != end; node = node->next)
    {
        SessionParticipantInfo::Participant* participant = SessionParticipantInfo::ParticipantList::ValueOf(node);
        if (participant->m_principalId == principalId)
        {
            if (participationEvent == cParticipationEventEndParticipation
                && participant->m_participationEvent == cParticipationEventDisconnect)
            {
                participant->m_participationEvent = participationEvent;
                participant->m_updateTime = common::g_piaJobScheduler->m_dispatchTime;
                return;
            }
            participant->m_participationEvent = participationEvent;
            --participant->m_participationCount;
            participant->m_updateTime = common::g_piaJobScheduler->m_dispatchTime;
            if (participant->m_participationCount == 0)
                list.Free(static_cast<SessionParticipantInfo::ParticipantList::Node*>(node));
            return;
        }
    }
    SessionParticipantInfo::Participant* participant = list.Alloc();
    if (participant != 0)
    {
        participant->m_principalId = principalId;
        participant->m_stationId = 0;
        participant->m_participationEvent = participationEvent;
        participant->m_participationCount = -1;
        participant->m_leaving = false;
        participant->m_updateTime = common::g_piaJobScheduler->m_dispatchTime;
    }
}

void NexMatchMeshLayerController::NexNotificationEventHandler::ProcessNotificationEvent(
    const nex::NotificationEvent& event)
{
    const unsigned int eventCategory = event.GetType() / cNotificationTypeCategoryUnit;
    if (eventCategory != cNotificationTypeParticipation && eventCategory != cNotificationTypeOwnershipChange
        && eventCategory != cNotificationTypeGatheringUnregistered && eventCategory != cNotificationTypeHostChange
        && eventCategory != cNotificationTypeSystemPasswordChange
        && eventCategory != cNotificationTypeSystemPasswordClear && eventCategory != cNotificationTypeSwitchGathering)
        return;

    session::Session* piaSession = session::g_piaSession;
    const unsigned short eventType = static_cast<unsigned short>(eventCategory);
    const unsigned int eventSubType = event.GetType() % cNotificationTypeCategoryUnit;
    const unsigned int sessionId = event.GetParam1();
    const unsigned long long principalId = event.GetParam2();
    NexMatchmakeSession* currentSession = static_cast<NexMatchmakeSession*>(
        piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex]);
    NexMatchmakeSession* bufferSession = static_cast<NexMatchmakeSession*>(
        piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex == 0]);
    const unsigned long long localPrincipalId = NexFacade::GetInstance()->GetLocalPrincipalId();

    if (!IsIgnoredSessionId(sessionId))
    {
        SaveSessionId(sessionId);
    }
    else
    {
        if (principalId != localPrincipalId)
            return;
        if (eventType == cNotificationTypeParticipation)
        {
            if (eventSubType != cParticipationEventParticipate)
                return;
        }
        else if (eventType != cNotificationTypeSwitchGathering)
        {
            return;
        }
        RemoveIgnoredSessionId(sessionId);
    }

    switch (eventType)
    {
    case cNotificationTypeParticipation:
        if (eventSubType == cParticipationEventParticipate)
        {
            unsigned char participantType = cParticipantTypePlayer;
            const char* text = reinterpret_cast<const char*>(static_cast<const wchar_t*>(event.m_StringParam));
            int length = event.m_StringParam.GetLength();
            if (length != 0)
            {
                if (static_cast<unsigned int>(text[0] - '0') < cDecimalDigitNum)
                {
                    const unsigned char type = static_cast<unsigned char>(text[0] - '0');
                    if (type >= cParticipantTypePlayer && type <= cParticipantTypePlayer)
                        participantType = type;
                }
                if (length >= cParticipationTextStationIdLength)
                {
                    unsigned long long stationId;
                    length -= cParticipationTextStationIdOffset;
                    text += cParticipationTextStationIdOffset;
                    if (length >= cStationIdTextLength)
                    {
                        char stationIdText[cStationIdTextLength + 1];
                        OSBlockMove(stationIdText, text, cStationIdTextLength, FALSE);
                        stationIdText[cStationIdTextLength] = 0;
                        unsigned long long parsedStationId;
                        if (pead::StringUtil::tryParseU64(&parsedStationId, pead::SafeStringBase<char>(stationIdText),
                                                          pead::StringUtil::cHexadecimal))
                            stationId = parsedStationId;
                    }
                    if (principalId != stationId)
                        participantType = cParticipantTypePlayer;
                    AddParticipantPrincipalId(sessionId, principalId, stationId, participantType);
                    return;
                }
            }
            AddParticipantPrincipalId(sessionId, principalId, principalId, participantType);
            return;
        }
        RemoveParticipantPrincipalId(sessionId, principalId, static_cast<short>(eventSubType));
        piaSession->NotifyLeave(sessionId, principalId);
        if (principalId == localPrincipalId)
        {
            ClearSessionId(sessionId);
            AddIgnoredSessionId(sessionId);
        }
        break;

    case cNotificationTypeOwnershipChange:
        if (piaSession->m_sessionId[piaSession->m_currentSessionIndex] == sessionId)
        {
            currentSession->SetMatchmakeOwner(principalId == localPrincipalId);
            currentSession->SetMatchmakeOwnerUniqueId(principalId);
        }
        else if (piaSession->m_sessionId[piaSession->m_currentSessionIndex == 0] == sessionId)
        {
            if (bufferSession != 0)
            {
                bufferSession->SetMatchmakeOwner(principalId == localPrincipalId);
                bufferSession->SetMatchmakeOwnerUniqueId(principalId);
            }
        }
        else if (currentSession->IsProcessingJoinMatchmakeSession())
        {
            currentSession->SetTemporaryOwnerChangeEvent(sessionId, principalId);
        }
        else if (bufferSession->IsProcessingJoinMatchmakeSession())
        {
            bufferSession->SetTemporaryOwnerChangeEvent(sessionId, principalId);
        }
        piaSession->NotifyOwnershipChange(sessionId, principalId);
        break;

    case cNotificationTypeGatheringUnregistered:
        if (piaSession->m_sessionId[piaSession->m_currentSessionIndex] == sessionId)
            m_unregisterGathering = true;
        if (piaSession->m_sessionId[piaSession->m_currentSessionIndex == 0] == sessionId && bufferSession != 0)
            m_unregisterGathering = true;
        ClearSessionId(sessionId);
        AddIgnoredSessionId(sessionId);
        piaSession->NotifyUnregister(sessionId);
        break;

    case cNotificationTypeSwitchGathering:
        if (principalId == localPrincipalId)
            SaveSessionId(sessionId);
        if (principalId == localPrincipalId && piaSession->m_sessionId[piaSession->m_currentSessionIndex] != sessionId
            && bufferSession != 0 && piaSession->m_matchmakeStatus == cMatchmakeStatusJointSessionProcessing)
        {
            const unsigned int bufferSessionId = piaSession->m_sessionId[piaSession->m_currentSessionIndex == 0];
            if (bufferSessionId == 0 || bufferSessionId == sessionId)
            {
                session::g_piaSession->m_meshLayerController->SetBufferSessionId(sessionId);
            }
            else if (piaSession->m_meshLayerController->GetJointSessionJobPtr() != 0
                     && piaSession->m_meshLayerController->GetJointSessionJobPtr()->IsRunning())
            {
                session::JointSessionJob* jointSessionJob = static_cast<session::JointSessionJob*>(
                    piaSession->m_meshLayerController->GetJointSessionJobPtr());
                jointSessionJob->m_failurePending = true;
                jointSessionJob->m_pendingFailureResult = cResultOwnerChanged;
            }
        }
        break;

    case cNotificationTypeSystemPasswordChange:
        if (sessionId == piaSession->m_sessionId[piaSession->m_currentSessionIndex]
            || sessionId == piaSession->m_sessionId[piaSession->m_currentSessionIndex == 0])
        {
            common::String password(static_cast<const wchar_t*>(event.m_StringParam));
            piaSession->SetSessionSystemPassword(password);
            session::Session::NoticeEventCallback(session::Session::cEventTypeSessionSystemPasswordSet,
                                                  cStationIndexInvalid);
        }
        break;

    case cNotificationTypeSystemPasswordClear:
        if (sessionId == piaSession->m_sessionId[piaSession->m_currentSessionIndex]
            || sessionId == piaSession->m_sessionId[piaSession->m_currentSessionIndex == 0])
        {
            piaSession->ResetSessionSystemPassword();
            session::Session::NoticeEventCallback(session::Session::cEventTypeSessionSystemPasswordClear,
                                                  cStationIndexInvalid);
        }
        break;

    default:
        break;
    }
}

NexMatchMeshLayerController::NexMatchMeshLayerController() : m_nexFacadeNgsInterface(0)
{
    m_notificationEventHandler = common::NewObj<NexNotificationEventHandler>();
}

NexMatchMeshLayerController::~NexMatchMeshLayerController()
{
    if (m_notificationEventHandler != 0)
    {
        common::DeleteObj(m_notificationEventHandler);
        m_notificationEventHandler = 0;
    }
}

Result NexMatchMeshLayerController::Startup(const session::Session::StartupSetting& baseSetting)
{
    const NexSessionStartupSetting& setting = static_cast<const NexSessionStartupSetting&>(baseSetting);
    if (setting.m_payloadSize > cPayloadSizeMax)
        return ResultInvalidArgument();
    if (setting.m_payloadSize < cPayloadSizeMin)
        return ResultInvalidArgument();

    const bool playerInfoValid = session::g_piaMesh->CheckPlayerInfo(setting.m_playerInfo[0]) == 0;
    m_payloadSize = setting.m_payloadSize;
    common::g_sessionBeginMonitoringContent.m_metric25 = static_cast<unsigned short>(setting.m_payloadSize);
    m_registeredPlayerInfo = false;
    m_playerInfoSize = 0;
    ResetPlayerInfo();
    m_ngsInterface = 0;
    m_fielde0 = 0;
    if (playerInfoValid)
    {
        transport::Station::PlayerInfo& playerInfo = m_playerInfo[m_playerInfoSize];
        const transport::Station::PlayerInfo& sourceInfo = setting.m_playerInfo[m_playerInfoSize];
        m_registeredPlayerInfo = true;
        playerInfo.m_name.Copy(sourceInfo.m_name);
        playerInfo.m_language = sourceInfo.m_language;
        playerInfo.m_field70 = sourceInfo.m_field70;
        playerInfo.m_field78 = sourceInfo.m_field78;
        m_playerInfo[m_playerInfoSize].m_name.Trace(cPlayerNameTraceFlag);
        ++m_playerInfoSize;
    }
    if (m_playerInfoSize == 0)
    {
        if (setting.m_recentPlayRecordEnabled && session::g_piaMesh->IsRecentPlayRecordEnabled())
            return ResultInvalidArgument();
        m_playerInfoSize = 1;
    }
    m_playersCountedAsParticipants = setting.m_playersCountedAsParticipants;
    m_skipBandwidthCheck = setting.m_field99;
    m_hostMigrationEnabled = setting.m_hostMigrationEnabled;
    m_bandwidthCheckMeasurementTime = setting.m_bandwidthCheckMeasurementTime;
    m_bandwidthCheckMeasurementCount = setting.m_bandwidthCheckMeasurementCount;
    m_cryptoSetting.m_mode = common::Crypto::cModeAesGcm;
    m_bandwidthCheckIpPacketSize = setting.m_bandwidthCheckIpPacketSize;
    m_recentPlayRecordEnabled = setting.m_recentPlayRecordEnabled;

    if (NexFacade::GetInstance() == 0)
        return ResultInvalidState();
    m_nexFacadeNgsInterface = NexFacade::GetInstance()->m_ngsBridge;
    if (m_nexFacadeNgsInterface == 0)
        return ResultInvalidState();

    m_ngsInterfaceIndexBoundNexFacade = cNgsInterfaceIndexInvalid;
    nex::NgsFacade* ngsFacade = static_cast<nex::NgsFacade*>(setting.m_field94);
    if (m_playersCountedAsParticipants && ngsFacade != 0)
    {
        if (m_playerInfoSize != 1 && ngsFacade->GetCredentials() == 0)
            return ResultInvalidArgument();
        if (m_nexFacadeNgsInterface != ngsFacade->GetNgsBridge())
            return ResultInvalidArgument();
        m_ngsInterface = ngsFacade->GetNgsBridge();
    }
    else
    {
        m_ngsInterface = m_nexFacadeNgsInterface;
    }
    m_fielde0 = m_ngsInterface->GetPrincipalID();
    m_ngsInterfaceIndexBoundNexFacade = 0;

    session::Session* piaSession = session::g_piaSession;
    const Result result = static_cast<NexMatchmakeSession*>(
                              piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex])
                              ->Initialize();
    if (result != 0)
        return ResultInvalidState();
    NexMatchmakeSession* bufferMatchmakeSession = static_cast<NexMatchmakeSession*>(
        piaSession->m_matchmakeSession[piaSession->m_currentSessionIndex == 0]);
    if (bufferMatchmakeSession != 0)
    {
        const Result bufferResult = bufferMatchmakeSession->Initialize();
        if (bufferResult != 0)
            return ResultInvalidState();
    }

    m_nexFacadeNgsInterface->RegisterNotificationEventHandler(m_notificationEventHandler);
    Result::ClearNexNetworkErrorCode();
    common::g_sessionBeginMonitoringContent.m_metric159 = playerInfoValid;
    common::g_sessionBeginMonitoringContent.m_metric160 = m_playersCountedAsParticipants;
    return MeshLayerController::Startup(baseSetting);
}

void NexMatchMeshLayerController::MeshCleanup()
{
    MeshLayerController::MeshCleanup();
    NexFacade::GetInstance()->StopNatSession();
    NexFacade::GetInstance()->Cleanup();
}

Result NexMatchMeshLayerController::MeshStartup(session::IMatchmakeSession* matchmakeSession, bool skipBandwidthCheck)
{
    const unsigned char* sessionKey = static_cast<NexMatchmakeSession*>(matchmakeSession)->m_matchmakeSessionKey;
    if (m_cryptoSetting.m_mode != common::Crypto::cModeNone)
        OSBlockMove(m_cryptoSetting.m_key, sessionKey, sizeof(m_cryptoSetting.m_key), FALSE);
    const Result result = MeshLayerController::MeshStartup(matchmakeSession, skipBandwidthCheck);
    if (result == 0)
    {
        unsigned int relayGatheringId = 0;
        unsigned char relaySignatureKey[NexNatRelay::cSignatureKeySize];
        unsigned char* gatheringIdBytes = reinterpret_cast<unsigned char*>(&relayGatheringId);
        for (unsigned int index = 0; index < cRelayGatheringIdSize; ++index)
            gatheringIdBytes[index] = *sessionKey++;
        relayGatheringId = SwapU32(relayGatheringId);
        for (unsigned int index = 0; index < NexNatRelay::cSignatureKeySize; ++index)
            relaySignatureKey[index] = *sessionKey++;
        NexFacade::GetInstance()->m_natRelay->SetRelayGatheringId(relayGatheringId);
        NexFacade::GetInstance()->m_natRelay->SetRelayServerSignatureKey(relaySignatureKey);
    }
    return result;
}

Result NexMatchMeshLayerController::StartNatSession()
{
    Result result = NexFacade::GetInstance()->Startup();
    if (result == 0)
    {
        result = NexFacade::GetInstance()->StartNatSessionAsync();
        if (result == 0)
            return result;
        NexFacade::GetInstance()->StopNatSession();
    }
    NexFacade::GetInstance()->Cleanup();
    return result;
}

common::Job::ExecuteKind NexMatchMeshLayerController::WaitStartNatSession(Result* result)
{
    if (NexFacade::GetInstance()->IsStartNatSessionCompleted())
    {
        const Result startResult = NexFacade::GetInstance()->GetStartNatSessionResult();
        if (startResult == 0)
        {
            session::Session* piaSession = session::g_piaSession;
            NexFacade::GetInstance()->UpdateGatheringID(piaSession->m_sessionId[piaSession->m_currentSessionIndex]);
            *result = startResult;
            return common::Job::cExecuteKindContinue;
        }
        *result = startResult;
        NexFacade::GetInstance()->StopNatSession();
        NexFacade::GetInstance()->Cleanup();
        return common::Job::cExecuteKindFinished;
    }
    return common::Job::cExecuteKindNextStep;
}

Result NexMatchMeshLayerController::CancelStartNatSession()
{
    return NexFacade::GetInstance()->CancelStartNatSessionAsync();
}

void NexMatchMeshLayerController::SetLeaveFlag()
{
    NexFacade::GetInstance()->SetIgnoreNatProbeRequest(true);
}

bool NexMatchMeshLayerController::IsAllStationJoined(unsigned int sessionId)
{
    const SessionParticipantInfo* info = GetSessionParticipantInfo(sessionId);
    transport::StationIdTable* stationIdTable = transport::g_piaTransport->m_stationIdTable;
    const common::ListNode* end = info->m_participantList.Sentinel();
    for (const common::ListNode* node = end->next; node != end; node = node->next)
    {
        const SessionParticipantInfo::Participant* participant = SessionParticipantInfo::ParticipantList::ValueOf(node);
        if (participant->m_principalId == participant->m_stationId
            && stationIdTable->FindByUniqueIdCore(participant->m_principalId) == 0)
            return false;
    }
    return true;
}

void NexMatchMeshLayerController::Cleanup()
{
    NexMatchMeshLayerController::MeshCleanup();
    m_notificationEventHandler->Cleanup();
    MeshLayerController::Cleanup();
    static_cast<NexMatchmakeSession*>(
        session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex])
        ->Finalize();
    NexMatchmakeSession* otherMatchmakeSession = static_cast<NexMatchmakeSession*>(
        session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex == 0]);
    if (otherMatchmakeSession != 0)
        otherMatchmakeSession->Finalize();
    if (m_nexFacadeNgsInterface != 0)
    {
        m_nexFacadeNgsInterface->UnregisterNotificationEventHandler(m_notificationEventHandler);
        m_nexFacadeNgsInterface = 0;
    }
    ResetPlayerInfo();
    m_ngsInterface = 0;
    m_fielde0 = 0;
}

Result NexMatchMeshLayerController::CheckConnectedStatusCore(nex::NgsBridgeInterface* ngsInterface)
{
    if (ngsInterface == 0)
        return transport::cResultInvalidArgument;
    if (!ngsInterface->IsConnected() && ngsInterface->IsConnected() == nex::qResult(cNexResultNotConnected))
        return session::Session::cResultStatusDisconnectedNetwork;
    return 0;
}

Result NexMatchMeshLayerController::CheckConnectedStatus()
{
    if (m_ngsInterface != 0)
    {
        const Result result = CheckConnectedStatusCore(m_ngsInterface);
        if (result != 0)
            return result;
    }
    if (m_notificationEventHandler->IsUnregisterGathering())
        return session::Session::cResultStatusDisconnectedSession;
    return 0;
}

bool NexMatchMeshLayerController::CheckSessionId(unsigned int sessionId)
{
    if (m_notificationEventHandler == 0)
        return false;
    return m_notificationEventHandler->CheckSessionId(sessionId);
}

void NexMatchMeshLayerController::ClearSessionId(unsigned int sessionId)
{
    if (m_notificationEventHandler != 0)
        m_notificationEventHandler->ClearSessionId(sessionId);
}

unsigned int NexMatchMeshLayerController::GetSessionId(unsigned int index) const
{
    if (m_notificationEventHandler == 0)
        return 0;
    return m_notificationEventHandler->GetSessionId(index);
}

unsigned int NexMatchMeshLayerController::GetSessionIdArraySize() const
{
    if (m_notificationEventHandler == 0)
        return 0;
    return cSessionIdArraySize;
}

const SessionParticipantInfo* NexMatchMeshLayerController::GetSessionParticipantInfo(unsigned int sessionId) const
{
    if (m_notificationEventHandler == 0)
        return 0;
    return m_notificationEventHandler->GetSessionParticipantInfo(sessionId);
}

unsigned int NexMatchMeshLayerController::GetMatchmakeSessionTotalStations() const
{
    if (m_notificationEventHandler != 0 && session::g_piaSession != 0)
    {
        session::Session* piaSession = session::g_piaSession;
        if (piaSession->GetJointSessionId() != 0)
            return m_notificationEventHandler->GetMatchmakeSessionTotalStations(piaSession->GetJointSessionId());
        return m_notificationEventHandler->GetMatchmakeSessionTotalStations(
            piaSession->m_sessionId[piaSession->m_currentSessionIndex]);
    }
    return 0;
}

unsigned short NexMatchMeshLayerController::GetMatchmakeSessionTotalParticipants() const
{
    if (m_notificationEventHandler != 0 && session::g_piaSession != 0)
    {
        session::Session* piaSession = session::g_piaSession;
        if (piaSession->GetJointSessionId() != 0)
            return m_notificationEventHandler->GetMatchmakeSessionTotalParticipants(piaSession->GetJointSessionId());
        const unsigned int sessionId = piaSession->m_sessionId[piaSession->m_currentSessionIndex];
        return m_notificationEventHandler->GetMatchmakeSessionTotalParticipants(sessionId);
    }
    return 0;
}

bool NexMatchMeshLayerController::IsParticipated(unsigned int sessionId, unsigned long long stationId) const
{
    if (m_notificationEventHandler == 0)
        return false;
    return m_notificationEventHandler->IsParticipated(sessionId, stationId);
}

unsigned int NexMatchMeshLayerController::GetMatchmakeSessionTotalStations(unsigned int sessionId) const
{
    if (m_notificationEventHandler != 0 && session::g_piaSession != 0)
        return m_notificationEventHandler->GetMatchmakeSessionTotalStations(sessionId);
    return 0;
}

void NexMatchMeshLayerController::CleanupEventHandler()
{
    if (m_notificationEventHandler != 0)
        m_notificationEventHandler->Cleanup();
}

bool NexMatchMeshLayerController::IsJoinedUnknownSession(unsigned int sessionId, unsigned int bufferSessionId)
{
    for (unsigned int index = 0; index < GetSessionIdArraySize(); ++index)
    {
        const unsigned int joinedSessionId = GetSessionId(index);
        if (joinedSessionId != 0 && joinedSessionId != sessionId && joinedSessionId != bufferSessionId)
            return true;
    }
    return false;
}

nex::NgsBridgeInterface* NexMatchMeshLayerController::GetNgsInterface(unsigned char index)
{
    if (index != 0)
        return 0;
    return m_ngsInterface;
}

unsigned char NexMatchMeshLayerController::GetNgsInterfaceIndexBoundNexFacade() const
{
    return m_ngsInterfaceIndexBoundNexFacade;
}

void NexMatchMeshLayerController::Trace(unsigned long long) const
{
}

}
}
}
