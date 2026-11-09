#include "nn/pia/inet/NexMatchmakeSession.hpp"
#include "nn/nex/Variant.hpp"
#include "nn/pia/inet/NexCreateSessionSetting.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/common/SessionStateMonitoringContent.hpp"
#include "nn/pia/inet/NexFacade.hpp"
#include "nn/pia/inet/NexJoinSessionSetting.hpp"
#include "nn/pia/session/JoinRandomSessionSetting.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/inet/NexMatchMeshLayerController.hpp"
#include "nn/pia/inet/SessionParticipantInfo.hpp"
#include "nn/pia/session/ICommunityInfoList.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/session/SessionInfoList.hpp"
#include "nn/pia/inet/NexSessionInfoPrivate.hpp"
#include "nn/pia/inet/NexCommunityInfoList.hpp"
#include "nn/pia/inet/NexCreateCommunitySetting.hpp"
#include "nn/pia/inet/NexUpdateCommunitySetting.hpp"
#include "nn/pia/inet/NexUpdateSessionSetting.hpp"
#include "nn/pia/inet/NexSessionSearchCriteriaParticipant.hpp"
#include "nn/pia/inet/NexSessionSearchCriteriaSessionId.hpp"
#include "nn/nex/PlayingSession.hpp"
#include "nn/nex/ErrorCodeConverter.hpp"
#include "nn/nex/NgsBridgeInterface.hpp"
#include "nn/nex/Reconstruction.hpp"
#include <coreinit/memory.h>
#include <cstdio>
#include <new>

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
    cResultInvalidArgument = 0x10c07,
    cResultRequestFailed = 0xac5a,
    cResultCancelled = 0x6c05,
    cResultSessionVoid = 0x6c4b,
    cResultSessionFull = 0x6c4c,
    cResultDeniedByParticipants = 0x6c4d,
    cResultParticipantInBlackList = 0x6c4e,
    cResultUserPasswordMismatch = 0x6c51,
    cResultSystemPasswordMismatch = 0x6c52,
    cResultSessionClosed = 0x6c53,
    cResultOwnerOffline = 0x6c54,
    cResultNotFriend = 0x6c55,
    cResultSessionInconsistent = 0x6c5b,
    cResultCommunityCreationLimit = 0x6c5f,
    cResultCommunityParticipationLimit = 0x6c60,
    cResultCommunityFull = 0x6c61,
    cResultCommunityVoid = 0x6c62,
    cResultCommunityOutOfParticipationPeriod = 0x6c63,
    cResultCommunityPasswordMismatch = 0x6c66,
    cResultAlreadyParticipated = 0x2c64,
    cResultServerMaintenance = 0xa44f,
    cResultSelectionPriorityUpdateTooFrequent = 0x4c11,
    cResultNexError = 0xe437
};

enum
{
    cNexResultInvalidArgument = static_cast<int>(0x8001000Au),
    cNexResultSessionVoid = static_cast<int>(0x80030073u),
    cNexResultSessionFull = static_cast<int>(0x800300C8u),
    cNexResultInvalidGatheringPassword = static_cast<int>(0x800300C9u),
    cNexResultWithoutParticipationPeriod = static_cast<int>(0x800300CAu),
    cNexResultPersistentGatheringCreationMax = static_cast<int>(0x800300CBu),
    cNexResultPersistentGatheringParticipationMax = static_cast<int>(0x800300CCu),
    cNexResultDeniedByParticipants = static_cast<int>(0x800300CDu),
    cNexResultParticipantInBlackList = static_cast<int>(0x800300CEu),
    cNexResultGameServerMaintenance = static_cast<int>(0x800300CFu),
    cNexResultOutOfRatingRange = static_cast<int>(0x800300D1u),
    cNexResultNotParticipatedGathering = static_cast<int>(0x800300D4u),
    cNexResultUserPasswordUnmatch = static_cast<int>(0x800300D5u),
    cNexResultSystemPasswordUnmatch = static_cast<int>(0x800300D6u),
    cNexResultUserIsOffline = static_cast<int>(0x800300D7u),
    cNexResultAlreadyParticipatedGathering = static_cast<int>(0x800300D8u),
    cNexResultPermissionDenied = static_cast<int>(0x800300D9u),
    cNexResultNotFriend = static_cast<int>(0x800300DAu),
    cNexResultSessionClosed = static_cast<int>(0x800300DBu)
};

enum
{
    cRequestTimeoutMsec = 10000
};

enum
{
    cJoinMessageBufferSize = 72,
    cJoinMessageLengthMax = cJoinMessageBufferSize - 1
};

enum
{
    cGatheringFlagsHostMigration = 0x210,
    cGatheringFlagsMatchmake = 0x800
};

enum
{
    cMonitoringMetricUnset = 0xff
};

enum
{
    cTraceSessionInfo = 0x04000000
};

enum
{
    cSessionAttributeNum = 6,
    cApplicationDataSizeMax = 512
};

enum
{
    cMatchmakePolicyArgumentAutoClose = 1
};

enum
{
    cModificationFlagOpenParticipation = 1 << 1
};

enum
{
    cParticipationPolicyNormal = 95,
    cParticipationPolicyDefault = 96,
    cParticipationPolicyRanked = 98
};

enum
{
    cSelectionPriorityMax = 100,
    cSelectionPriorityUpdateIntervalSec = 30
};
}

NexMatchmakeSession::NexMatchmakeSession()
    : m_gatheringId(0),
      m_protocolCallContext(0),
      m_protocolCallContextB(0),
      m_callContext(0),
      m_matchmakeHost(false),
      m_randomMatchmakeScore(0),
      m_randomMatchmakeScoreAvailable(false),
      m_processingJoinMatchmakeSession(false),
      m_temporaryOwnerChangeGatheringId(0),
      m_temporaryOwnerChangeOwnerId(0)
{
    m_ngsBridge = 0;
    m_matchmakeExtensionClientMemory = common::NewArray<unsigned char>(sizeof(nn::nex::MatchmakeExtensionClient), 8);
    m_ownMatchmakeExtensionClient = 0;
    m_matchmakeExtensionClient = 0;
    m_matchmakeOwnerUniqueId = 0;
    m_matchmakeOwner = false;
    m_requestSession = common::NewObj<nn::nex::MatchmakeSession>();
    m_searchCriteriaList = common::NewObj<SearchCriteriaList>();
    m_resultRangeList = common::NewObj<ResultRangeList>();
    m_gatheringList = common::NewObj<nn::nex::qList<nn::nex::AnyObjectHolder<nn::nex::Gathering, nn::nex::String> > >();
    m_stationUrlList = common::NewObj<nn::nex::qList<nn::nex::StationURL> >();
    m_string78 = common::NewObj<nn::nex::String>();
    m_string78->Reserve(16);
    m_foundSession = common::NewObj<nn::nex::MatchmakeSession>();
    m_matchmakeParam = common::NewObj<nn::nex::MatchmakeParam>();
    m_autoMatchmakeParam = common::NewObj<nn::nex::AutoMatchmakeParam>();
    m_createMatchmakeSessionParam = common::NewObj<nn::nex::CreateMatchmakeSessionParam>();
    m_joinMatchmakeSessionParam = common::NewObj<nn::nex::JoinMatchmakeSessionParam>();
    m_updateMatchmakeSessionParam = common::NewObj<nn::nex::UpdateMatchmakeSessionParam>();
    m_persistentGathering = common::NewObj<nn::nex::PersistentGathering>();
    m_persistentGatheringList = common::NewObj<nn::nex::qList<nn::nex::PersistentGathering> >();
    m_browsedCommunityInfoList = session::g_piaSession->m_communityInfoList;
    m_idList68 = common::NewObj<IdList>();
    m_resultRange = common::NewObj<nn::nex::ResultRange>();
    m_browseMatchmakeSessionResult = session::g_piaSession->m_sessionInfoList;
    m_idList7c = common::NewObj<IdList>();
    m_inetAddress = common::NewObj<nn::nex::InetAddress>();
    m_playingSessionList = common::NewObj<nn::nex::qList<nn::nex::PlayingSession> >();
    OSBlockSet(m_matchmakeSessionKey, 0, sizeof(m_matchmakeSessionKey));
}

NexMatchmakeSession::~NexMatchmakeSession()
{
    m_matchmakeExtensionClient = m_ownMatchmakeExtensionClient;
    if (m_matchmakeExtensionClient != 0)
    {
        m_matchmakeExtensionClient->Unbind();
        m_matchmakeExtensionClient->~MatchmakeExtensionClient();
        m_ownMatchmakeExtensionClient = 0;
        m_matchmakeExtensionClient = 0;
    }
    if (m_matchmakeExtensionClientMemory != 0)
    {
        common::DeleteArray(m_matchmakeExtensionClientMemory);
        m_matchmakeExtensionClientMemory = 0;
    }
    if (m_requestSession != 0)
    {
        common::DeleteObj(m_requestSession);
        m_requestSession = 0;
    }
    if (m_searchCriteriaList != 0)
    {
        common::DeleteObj(m_searchCriteriaList);
        m_searchCriteriaList = 0;
    }
    if (m_resultRangeList != 0)
    {
        common::DeleteObj(m_resultRangeList);
        m_resultRangeList = 0;
    }
    if (m_gatheringList != 0)
    {
        common::DeleteObj(m_gatheringList);
        m_gatheringList = 0;
    }
    if (m_stationUrlList != 0)
    {
        common::DeleteObj(m_stationUrlList);
        m_stationUrlList = 0;
    }
    if (m_updateMatchmakeSessionParam != 0)
    {
        common::DeleteObj(m_updateMatchmakeSessionParam);
        m_updateMatchmakeSessionParam = 0;
    }
    if (m_foundSession != 0)
    {
        common::DeleteObj(m_foundSession);
        m_foundSession = 0;
    }
    if (m_matchmakeParam != 0)
    {
        common::DeleteObj(m_matchmakeParam);
        m_matchmakeParam = 0;
    }
    if (m_autoMatchmakeParam != 0)
    {
        common::DeleteObj(m_autoMatchmakeParam);
        m_autoMatchmakeParam = 0;
    }
    if (m_createMatchmakeSessionParam != 0)
    {
        common::DeleteObj(m_createMatchmakeSessionParam);
        m_createMatchmakeSessionParam = 0;
    }
    if (m_joinMatchmakeSessionParam != 0)
    {
        common::DeleteObj(m_joinMatchmakeSessionParam);
        m_joinMatchmakeSessionParam = 0;
    }
    if (m_persistentGathering != 0)
    {
        common::DeleteObj(m_persistentGathering);
        m_persistentGathering = 0;
    }
    if (m_persistentGatheringList != 0)
    {
        common::DeleteObj(m_persistentGatheringList);
        m_persistentGatheringList = 0;
    }
    m_browsedCommunityInfoList = 0;
    if (m_idList68 != 0)
    {
        common::DeleteObj(m_idList68);
        m_idList68 = 0;
    }
    if (m_resultRange != 0)
    {
        common::DeleteObj(m_resultRange);
        m_resultRange = 0;
    }
    if (m_string78 != 0)
    {
        common::DeleteObj(m_string78);
        m_string78 = 0;
    }
    m_browseMatchmakeSessionResult = 0;
    if (m_idList7c != 0)
    {
        common::DeleteObj(m_idList7c);
        m_idList7c = 0;
    }
    if (m_inetAddress != 0)
    {
        common::DeleteObj(m_inetAddress);
        m_inetAddress = 0;
    }
    if (m_playingSessionList != 0)
    {
        common::DeleteObj(m_playingSessionList);
        m_playingSessionList = 0;
    }
}

Result NexMatchmakeSession::BrowseMatchmakeSession(nn::pia::common::CallContext* callContext)
{
    if (callContext == 0)
    {
        return cResultInvalidArgument;
    }
    if (m_searchCriteriaList->size() == 0)
    {
        return transport::cResultInvalidState;
    }
    if (m_resultRangeList->size() == 0)
    {
        return transport::cResultInvalidState;
    }
    if (callContext->m_state == common::CallContext::cStateInProgress)
    {
        return ResultInvalidArgument();
    }
    if (m_callContext != 0)
    {
        return ResultInvalidState();
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallInProgress)
    {
        return transport::cResultInvalidState;
    }
    m_callContext = callContext;
    if (!m_matchmakeExtensionClient->BrowseMatchmakeSession(&m_protocolCallContext, m_searchCriteriaList->front(),
                                                            m_resultRangeList->front(), m_gatheringList))
    {
        m_callContext = 0;
        return ResultInvalidState();
    }
    m_callContext->InitiateCall();
    m_browseMatchmakeSessionResult->Clear();
    return 0;
}

void NexMatchmakeSession::Cleanup()
{
    m_gatheringHolder.Release();
    if (m_searchCriteriaList != 0)
    {
        m_searchCriteriaList->clear();
    }
    if (m_resultRangeList != 0)
    {
        m_resultRangeList->clear();
    }
    if (m_browseMatchmakeSessionResult != 0)
    {
        m_browseMatchmakeSessionResult->Clear();
    }
    if (m_gatheringList != 0)
    {
        m_gatheringList->clear();
    }
    if (m_stationUrlList != 0)
    {
        m_stationUrlList->clear();
    }
    m_gatheringId = 0;
    if (m_foundSession != 0)
    {
        m_foundSession->Reset();
    }
    if (m_matchmakeParam != 0)
    {
        m_matchmakeParam->m_Parameters.clear();
    }
    if (m_autoMatchmakeParam != 0)
    {
        m_autoMatchmakeParam->Reset();
    }
    if (m_createMatchmakeSessionParam != 0)
    {
        m_createMatchmakeSessionParam->Reset();
    }
    if (m_joinMatchmakeSessionParam != 0)
    {
        m_joinMatchmakeSessionParam->Reset();
    }
    if (m_updateMatchmakeSessionParam != 0)
    {
        m_updateMatchmakeSessionParam->Reset();
    }
    if (m_persistentGathering != 0)
    {
        m_persistentGathering->Reset();
    }
    if (m_persistentGatheringList != 0)
    {
        m_persistentGatheringList->clear();
    }
    if (m_browsedCommunityInfoList != 0)
    {
        m_browsedCommunityInfoList->Clear();
    }
    if (m_idList68 != 0)
    {
        m_idList68->clear();
    }
    if (m_idList7c != 0)
    {
        m_idList7c->clear();
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallInProgress)
    {
        m_protocolCallContext.Cancel(nn::nex::CallContext::CallCanceled);
        m_protocolCallContext.Reset();
    }
    if (m_protocolCallContextB.GetState() == nn::nex::CallContext::CallInProgress)
    {
        m_protocolCallContextB.Cancel(nn::nex::CallContext::CallCanceled);
        m_protocolCallContextB.Reset();
    }
    if (m_playingSessionList != 0)
    {
        m_playingSessionList->clear();
    }
    if (m_callContext != 0)
    {
        if (m_callContext->m_state == common::CallContext::cStateInProgress)
        {
            m_callContext->SignalFailure(cResultCancelled);
        }
        m_callContext = 0;
    }
    OSBlockSet(m_matchmakeSessionKey, 0, sizeof(m_matchmakeSessionKey));
    m_matchmakeOwner = false;
    m_matchmakeHost = false;
    m_matchmakeOwnerUniqueId = 0;
    m_sessionInfo.Clear();
    m_processingJoinMatchmakeSession = false;
    m_temporaryOwnerChangeOwnerId = 0;
    m_randomMatchmakeScoreAvailable = false;
    m_randomMatchmakeScore = 0;
    m_temporaryOwnerChangeGatheringId = 0;
}

Result NexMatchmakeSession::ClearSystemPassword(nn::pia::common::CallContext* callContext, unsigned int gatheringId)
{
    if (callContext == 0)
    {
        return cResultInvalidArgument;
    }
    if (callContext->m_state == common::CallContext::cStateInProgress)
    {
        return cResultInvalidArgument;
    }
    if (m_callContext != 0)
    {
        return transport::cResultInvalidState;
    }
    m_callContext = callContext;
    if (!m_matchmakeExtensionClient->ClearMatchmakeSessionSystemPassword(&m_protocolCallContext, gatheringId))
    {
        m_callContext = 0;
        return cResultRequestFailed;
    }
    m_callContext->InitiateCall();
    return 0;
}

Result NexMatchmakeSession::CloseParticipate(nn::pia::common::CallContext* callContext, unsigned int gatheringId)
{
    if (callContext == 0)
    {
        return ResultInvalidArgument();
    }
    if (callContext->m_state == common::CallContext::cStateInProgress)
    {
        return ResultInvalidArgument();
    }
    if (m_callContext != 0)
    {
        return transport::cResultInvalidState;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallInProgress)
    {
        return ResultInvalidState();
    }
    m_callContext = callContext;
    if (!m_matchmakeExtensionClient->CloseParticipation(&m_protocolCallContext, gatheringId))
    {
        m_callContext = 0;
        return cResultRequestFailed;
    }
    m_callContext->InitiateCall();
    return 0;
}

unsigned long long NexMatchmakeSession::ConvertGatheringIdToPersistentGatheringCode(unsigned int gatheringId)
{
    NexFacade::GetInstance()->SetPersistentGatheringCodeKey();
    unsigned long long code = nn::nex::MatchmakeExtensionClient::GatheringIdToPersistentGatheringCode(gatheringId);
    NexFacade::GetInstance()->ClearPersistentGatheringCodeKey();
    return code;
}

SessionType NexMatchmakeSession::ConvertMatchmakeSystemTypeToSessionType(nn::nex::MatchmakeSystemType systemType)
{
    if (systemType == nn::nex::cMatchmakeSystemType_1)
    {
        return cSessionType_Normal;
    }
    if (systemType == nn::nex::cMatchmakeSystemType_2)
    {
        return cSessionType_Ranked;
    }
    return cSessionType_Normal;
}

unsigned int NexMatchmakeSession::ConvertPersistentGatheringCodeToGatheringId(unsigned long long code)
{
    NexFacade::GetInstance()->SetPersistentGatheringCodeKey();
    unsigned int gatheringId = nn::nex::MatchmakeExtensionClient::PersistentGatheringCodeToGatheringId(code);
    NexFacade::GetInstance()->ClearPersistentGatheringCodeKey();
    return gatheringId;
}

nn::nex::MatchmakeSelectionMethod NexMatchmakeSession::ConvertSelectionMethodToMatchmakeSelectionMethod(
    SelectionMethod method)
{
    switch (method)
    {
    case cSelectionMethod_1:
        return nn::nex::cMatchmakeSelectionMethod_4;
    case cSelectionMethod_2:
        return nn::nex::cMatchmakeSelectionMethod_5;
    default:
        return nn::nex::cMatchmakeSelectionMethod_0;
    }
}

nn::nex::MatchmakeSystemType NexMatchmakeSession::ConvertSessionTypeToMatchmakeSystemType(SessionType sessionType)
{
    if (sessionType == cSessionType_Normal)
    {
        return nn::nex::cMatchmakeSystemType_1;
    }
    if (sessionType == cSessionType_Ranked)
    {
        return nn::nex::cMatchmakeSystemType_2;
    }
    return nn::nex::cMatchmakeSystemType_0;
}

void NexMatchmakeSession::CopyFoundPersistentGatheringToBrowsedCommunityInfoList(
    const nn::nex::PersistentGathering* gathering, NexCommunityInfo* communityInfo)
{
    communityInfo->SetCommunityId(gathering->m_uiValue1);
    communityInfo->SetOfficialCommunity(gathering->GetPersistentGatheringType()
                                        == nn::nex::PersistentGatheringType::cPersistentGatheringType_Official);
    communityInfo->SetRestrictedByPassword(
        gathering->GetPersistentGatheringType()
        == nn::nex::PersistentGatheringType::cPersistentGatheringType_PasswordLocked);
    if (!gathering->m_ParticipationStartDate.IsNever())
    {
        communityInfo->SetParticipationStartTime(common::DateTime(
            gathering->m_ParticipationStartDate.GetYear(), gathering->m_ParticipationStartDate.GetMonth(),
            gathering->m_ParticipationStartDate.GetDay(), gathering->m_ParticipationStartDate.GetHour(),
            gathering->m_ParticipationStartDate.GetMinute(), gathering->m_ParticipationStartDate.GetSecond()));
    }
    else
    {
        communityInfo->SetParticipationStartTime(common::DateTime());
    }
    if (!gathering->m_ParticipationEndDate.IsNever())
    {
        communityInfo->SetParticipationEndTime(common::DateTime(
            gathering->m_ParticipationEndDate.GetYear(), gathering->m_ParticipationEndDate.GetMonth(),
            gathering->m_ParticipationEndDate.GetDay(), gathering->m_ParticipationEndDate.GetHour(),
            gathering->m_ParticipationEndDate.GetMinute(), gathering->m_ParticipationEndDate.GetSecond()));
    }
    else
    {
        communityInfo->SetParticipationEndTime(common::DateTime());
    }
    communityInfo->SetRelatedSessionCount(gathering->m_uiMatchmakeSessionCount);
    for (unsigned int i = 0; i < cSessionAttributeNum; ++i)
    {
        communityInfo->SetAttribute(static_cast<unsigned char>(i), gathering->GetAttribute(i));
        unsigned int attribute;
        communityInfo->GetAttribute(static_cast<unsigned char>(i), &attribute);
    }
    communityInfo->SetOwnerPrincipalId(gathering->m_uiValue2);
    communityInfo->SetCurrentParticipantNum(static_cast<unsigned short>(gathering->m_uiParticipationCount));
    communityInfo->SetMaxParticipantNum(gathering->m_uiShortValue2);
    communityInfo->SetMinParticipantNum(gathering->m_uiShortValue1);
    communityInfo->SetDescription(common::String(static_cast<const wchar_t*>(gathering->m_Description)));
    nn::nex::qVector<unsigned char> applicationBuffer(gathering->m_ApplicationBuffer);
    unsigned char applicationData[cApplicationDataSizeMax];
    unsigned int applicationDataSize = applicationBuffer.size();
    if (applicationDataSize > cApplicationDataSizeMax)
    {
        applicationDataSize = cApplicationDataSizeMax;
    }
    for (unsigned int i = 0; i < applicationDataSize; ++i)
    {
        applicationData[i] = applicationBuffer[i];
    }
    communityInfo->SetApplicationData(applicationData, applicationDataSize);
    communityInfo->GetApplicationDataSize(&applicationDataSize);
}

void NexMatchmakeSession::CopyMatchmakeSessionToSessionInfo(const nn::nex::MatchmakeSession* session,
                                                            NexSessionInfo* sessionInfo)
{
    sessionInfo->SetGameMode(session->m_uiGameMode);
    sessionInfo->SetSessionId(session->m_uiValue1);
    sessionInfo->SetCurrentParticipantNum(static_cast<unsigned short>(session->m_uiParticipationCount));
    sessionInfo->SetMinParticipantNum(session->m_uiShortValue1);
    sessionInfo->SetMaxParticipantNum(session->m_uiShortValue2);
    sessionInfo->SetOpen(session->m_bOpenParticipation);
    for (unsigned int i = 0; i < cSessionAttributeNum; ++i)
    {
        sessionInfo->SetAttribute(static_cast<unsigned char>(i), session->GetAttribute(i));
    }
    nn::nex::qVector<unsigned char> applicationBuffer;
    applicationBuffer.assign(session->m_ApplicationBuffer.begin(), session->m_ApplicationBuffer.end());
    unsigned char applicationData[cApplicationDataSizeMax];
    unsigned int applicationDataSize = applicationBuffer.size();
    if (applicationDataSize > cApplicationDataSizeMax)
    {
        applicationDataSize = cApplicationDataSizeMax;
    }
    for (unsigned int i = 0; i < applicationDataSize; ++i)
    {
        applicationData[i] = applicationBuffer[i];
    }
    sessionInfo->SetApplicationData(applicationData, applicationDataSize);
    sessionInfo->SetUserPasswordEnabled(session->m_bUserPasswordEnabled);
    sessionInfo->SetSystemPasswordEnabled(session->m_bSystemPasswordEnabled);
    bool autoCloseSession = false;
    if (session->m_uiMatchmakeSystemType == nn::nex::cMatchmakeSystemType_1)
    {
        if (session->GetPolicyArgument() == cMatchmakePolicyArgumentAutoClose)
        {
            autoCloseSession = true;
        }
    }
    else if (session->m_uiMatchmakeSystemType == nn::nex::cMatchmakeSystemType_2)
    {
        if (session->GetPolicyArgument() != cMatchmakePolicyArgumentAutoClose)
        {
            autoCloseSession = true;
        }
    }
    sessionInfo->SetSessionType(ConvertMatchmakeSystemTypeToSessionType(
                                    static_cast<nn::nex::MatchmakeSystemType>(session->m_uiMatchmakeSystemType)),
                                autoCloseSession);
    sessionInfo->SetHostPrincipalId(session->m_uiValue3);
    sessionInfo->SetSelectionPriority(session->m_uiProgressScore);
    nn::nex::DateTime startedTime(session->m_StartedTime);
    if (!startedTime.IsNever())
    {
        sessionInfo->SetStartedTime(common::DateTime(startedTime.GetYear(), startedTime.GetMonth(),
                                                     startedTime.GetDay(), startedTime.GetHour(),
                                                     startedTime.GetMinute(), startedTime.GetSecond()));
    }
    sessionInfo->SetCommunityId(session->m_uiReferGatheringID);
}

Result NexMatchmakeSession::CreateCommunity(nn::pia::common::CallContext* callContext)
{
    if (callContext == 0)
    {
        return ResultInvalidArgument();
    }
    if (callContext->m_state == common::CallContext::cStateInProgress)
    {
        return cResultInvalidArgument;
    }
    if (m_callContext != 0)
    {
        return transport::cResultInvalidState;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallInProgress)
    {
        return ResultInvalidState();
    }
    m_callContext = callContext;
    if (!m_matchmakeExtensionClient->CreatePersistentGathering(&m_protocolCallContext, *m_persistentGathering,
                                                               &m_gatheringId))
    {
        m_callContext = 0;
        return cResultRequestFailed;
    }
    m_callContext->InitiateCall();
    return 0;
}

Result NexMatchmakeSession::CreateMatchmakeSession(nn::pia::common::CallContext* callContext)
{
    if (callContext == 0)
    {
        return cResultInvalidArgument;
    }
    if (callContext->m_state == common::CallContext::cStateInProgress)
    {
        return cResultInvalidArgument;
    }
    if (m_callContext != 0)
    {
        return transport::cResultInvalidState;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallInProgress)
    {
        return transport::cResultInvalidState;
    }
    m_callContext = callContext;
    if (session::g_piaSession != 0 && session::g_piaSession->GetLocalPlayerCount() > 1
        && session::g_piaSession->IsPlayersCountedAsParticipants())
    {
        unsigned int localPlayerCount = session::g_piaSession->GetLocalPlayerCount();
        unsigned int addedParticipantNum = 0;
        if (m_ngsBridge != 0)
        {
            unsigned int principalId = m_ngsBridge->GetPrincipalID();
            if (principalId != NexFacade::GetInstance()->GetLocalPrincipalId())
            {
                m_idList7c->push_back(principalId);
                addedParticipantNum = 1;
            }
        }
        m_createMatchmakeSessionParam->m_AdditionalParticipants = *m_idList7c;
        unsigned char participationCount = static_cast<unsigned char>(localPlayerCount - addedParticipantNum);
        if (addedParticipantNum == 0)
        {
            m_createMatchmakeSessionParam->m_uiParticipationCount = participationCount;
        }
        char joinMessage[cJoinMessageBufferSize];
        int length = snprintf(joinMessage, cJoinMessageLengthMax, "%d", participationCount);
        unsigned long long ownerPrincipalId = 0;
        if (m_ngsBridge != 0)
        {
            ownerPrincipalId = m_ngsBridge->GetPrincipalID();
        }
        joinMessage[length] = ' ';
        ++length;
        length += snprintf(joinMessage + length, cJoinMessageLengthMax - length, "%016llx", ownerPrincipalId);
        if (length > cJoinMessageLengthMax)
        {
            length = cJoinMessageLengthMax;
        }
        joinMessage[length] = '\0';
        m_createMatchmakeSessionParam->m_strJoinMessage = nn::nex::String(joinMessage);
    }
    if (!m_matchmakeExtensionClient->CreateMatchmakeSession(&m_protocolCallContext, *m_createMatchmakeSessionParam,
                                                            m_foundSession))
    {
        m_callContext = 0;
        return cResultRequestFailed;
    }
    m_callContext->InitiateCall();
    return 0;
}

Result NexMatchmakeSession::CreateMatchmakeSessionWithTeam(nn::pia::common::CallContext* callContext,
                                                           unsigned long long* teamOwnerIds, unsigned short teamCount,
                                                           unsigned int gatheringId)
{
    if (callContext == 0)
    {
        return cResultInvalidArgument;
    }
    if (callContext->m_state == common::CallContext::cStateInProgress)
    {
        return cResultInvalidArgument;
    }
    if (m_callContext != 0)
    {
        return transport::cResultInvalidState;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallInProgress)
    {
        return transport::cResultInvalidState;
    }
    m_callContext = callContext;
    for (unsigned short i = 0; i < teamCount; ++i)
    {
        m_idList7c->push_back(static_cast<unsigned int>(teamOwnerIds[i]));
    }
    if (session::g_piaSession != 0 && session::g_piaSession->GetLocalPlayerCount() > 1
        && session::g_piaSession->IsPlayersCountedAsParticipants())
    {
        return transport::cResultInvalidState;
    }
    m_createMatchmakeSessionParam->m_AdditionalParticipants = *m_idList7c;
    m_createMatchmakeSessionParam->m_uiGatheringIDForParticipationCheck = gatheringId;
    if (!m_matchmakeExtensionClient->CreateMatchmakeSession(&m_protocolCallContext, *m_createMatchmakeSessionParam,
                                                            m_foundSession))
    {
        m_callContext = 0;
        return cResultRequestFailed;
    }
    m_callContext->InitiateCall();
    return 0;
}

Result NexMatchmakeSession::Finalize()
{
    if (m_ngsBridge != 0)
    {
        m_matchmakeExtensionClient = m_ownMatchmakeExtensionClient;
        if (m_matchmakeExtensionClient != 0)
        {
            m_matchmakeExtensionClient->Unbind();
            m_matchmakeExtensionClient->~MatchmakeExtensionClient();
            m_matchmakeExtensionClient = 0;
            m_ownMatchmakeExtensionClient = 0;
        }
        m_ngsBridge = 0;
    }
    return 0;
}

Result NexMatchmakeSession::FindCommunityByGatheringId(nn::pia::common::CallContext* callContext)
{
    if (callContext == 0)
    {
        return cResultInvalidArgument;
    }
    if (callContext->m_state == common::CallContext::cStateInProgress)
    {
        return cResultInvalidArgument;
    }
    if (m_callContext != 0)
    {
        return transport::cResultInvalidState;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallInProgress)
    {
        return transport::cResultInvalidState;
    }
    m_callContext = callContext;
    if (!m_matchmakeExtensionClient->FindPersistentGatheringByGatheringId(&m_protocolCallContext, *m_idList68,
                                                                          m_persistentGatheringList))
    {
        m_callContext = 0;
        return cResultRequestFailed;
    }
    m_callContext->InitiateCall();
    m_browsedCommunityInfoList->Clear();
    return 0;
}

Result NexMatchmakeSession::FindCommunityByOwner(nn::pia::common::CallContext* callContext, unsigned long long ownerId,
                                                 unsigned int offset, unsigned int size)
{
    if (callContext == 0)
    {
        return cResultInvalidArgument;
    }
    if (callContext->m_state == common::CallContext::cStateInProgress)
    {
        return cResultInvalidArgument;
    }
    if (m_callContext != 0)
    {
        return transport::cResultInvalidState;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallInProgress)
    {
        return transport::cResultInvalidState;
    }
    m_callContext = callContext;
    m_resultRange->m_uiSize = size;
    m_resultRange->m_uiOffset = offset;
    if (!m_matchmakeExtensionClient->FindByOwner(&m_protocolCallContext, ownerId, *m_resultRange, m_gatheringList))
    {
        m_callContext = 0;
        return cResultRequestFailed;
    }
    m_callContext->InitiateCall();
    m_browsedCommunityInfoList->Clear();
    return 0;
}

Result NexMatchmakeSession::FindCommunityByParticipant(nn::pia::common::CallContext* callContext,
                                                       unsigned long long participantId, unsigned int offset,
                                                       unsigned int size)
{
    if (callContext == 0)
    {
        return cResultInvalidArgument;
    }
    if (callContext->m_state == common::CallContext::cStateInProgress)
    {
        return cResultInvalidArgument;
    }
    if (m_callContext != 0)
    {
        return transport::cResultInvalidState;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallInProgress)
    {
        return transport::cResultInvalidState;
    }
    m_callContext = callContext;
    m_resultRange->m_uiSize = size;
    m_resultRange->m_uiOffset = offset;
    if (!m_matchmakeExtensionClient->FindPersistentGatheringByParticipant(&m_protocolCallContext, participantId,
                                                                          *m_resultRange, m_persistentGatheringList))
    {
        m_callContext = 0;
        return cResultRequestFailed;
    }
    m_callContext->InitiateCall();
    m_browsedCommunityInfoList->Clear();
    return 0;
}

Result NexMatchmakeSession::FindOfficialCommunity(nn::pia::common::CallContext* callContext, bool isAvailableOnly,
                                                  unsigned int offset, unsigned int size)
{
    if (callContext == 0)
    {
        return ResultInvalidArgument();
    }
    if (callContext->m_state == common::CallContext::cStateInProgress)
    {
        return ResultInvalidArgument();
    }
    if (m_callContext != 0)
    {
        return transport::cResultInvalidState;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallInProgress)
    {
        return transport::cResultInvalidState;
    }
    m_callContext = callContext;
    m_resultRange->m_uiSize = size;
    m_resultRange->m_uiOffset = offset;
    if (!m_matchmakeExtensionClient->FindOfficialPersistentGathering(&m_protocolCallContext, isAvailableOnly,
                                                                     *m_resultRange, m_persistentGatheringList))
    {
        m_callContext = 0;
        return cResultRequestFailed;
    }
    m_callContext->InitiateCall();
    m_browsedCommunityInfoList->Clear();
    return 0;
}

Result NexMatchmakeSession::FindSessionByOwner(nn::pia::common::CallContext* callContext,
                                               const NexSessionSearchCriteriaOwner& criteria)
{
    unsigned long long ownerId = criteria.GetOwnerPrincipalId();
    nn::nex::ResultRange resultRange(criteria.GetResultRangeOffset(), criteria.GetResultRangeSize());
    if (callContext == 0)
    {
        return cResultInvalidArgument;
    }
    if (callContext->m_state == common::CallContext::cStateInProgress)
    {
        return cResultInvalidArgument;
    }
    if (m_callContext != 0)
    {
        return transport::cResultInvalidState;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallInProgress)
    {
        return transport::cResultInvalidState;
    }
    m_callContext = callContext;
    if (!m_matchmakeExtensionClient->FindByOwner(&m_protocolCallContext, static_cast<unsigned int>(ownerId),
                                                 resultRange, m_gatheringList))
    {
        m_callContext = 0;
        return cResultRequestFailed;
    }
    m_callContext->InitiateCall();
    m_browseMatchmakeSessionResult->Clear();
    return 0;
}

Result NexMatchmakeSession::FindSessionByParticipant(nn::pia::common::CallContext* callContext,
                                                     const NexSessionSearchCriteriaParticipant& criteria)
{
    if (callContext == 0)
    {
        return cResultInvalidArgument;
    }
    if (callContext->m_state == common::CallContext::cStateInProgress)
    {
        return cResultInvalidArgument;
    }
    if (m_callContext != 0)
    {
        return transport::cResultInvalidState;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallInProgress)
    {
        return transport::cResultInvalidState;
    }
    m_callContext = callContext;
    IdList principalIds;
    const unsigned long long* participantIds = criteria.GetParticipantPrincipalId();
    for (unsigned int i = 0; i < criteria.GetParticipantPrincipalIdSize(); ++i)
    {
        principalIds.push_back(static_cast<unsigned int>(participantIds[i]));
    }
    if (!m_matchmakeExtensionClient->GetPlayingSession(&m_protocolCallContext, principalIds, m_playingSessionList))
    {
        m_callContext = 0;
        return cResultRequestFailed;
    }
    m_callContext->InitiateCall();
    m_browseMatchmakeSessionResult->Clear();
    return 0;
}

Result NexMatchmakeSession::FindSessionBySessionId(nn::pia::common::CallContext* callContext,
                                                   const NexSessionSearchCriteriaSessionId& criteria)
{
    if (callContext == 0)
    {
        return ResultInvalidArgument();
    }
    if (callContext->m_state == common::CallContext::cStateInProgress)
    {
        return ResultInvalidArgument();
    }
    if (m_callContext != 0)
    {
        return ResultInvalidState();
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallInProgress)
    {
        return ResultInvalidState();
    }
    m_callContext = callContext;
    if (!m_matchmakeExtensionClient->FindByID(&m_protocolCallContext, criteria.GetSessionId(), &m_gatheringHolder))
    {
        m_callContext = 0;
        return cResultRequestFailed;
    }
    const common::TimeSpan timeout = common::TimeSpan::FromMSec(cRequestTimeoutMsec);
    m_requestDeadline = common::g_piaJobScheduler->m_dispatchTime + timeout;
    m_callContext->InitiateCall();
    m_browseMatchmakeSessionResult->Clear();
    return 0;
}

Result NexMatchmakeSession::GenerateSystemPassword(nn::pia::common::CallContext* callContext, unsigned int gatheringId)
{
    if (callContext == 0)
    {
        return cResultInvalidArgument;
    }
    if (callContext->m_state == common::CallContext::cStateInProgress)
    {
        return cResultInvalidArgument;
    }
    if (m_callContext != 0)
    {
        return transport::cResultInvalidState;
    }
    m_callContext = callContext;
    if (!m_matchmakeExtensionClient->GenerateMatchmakeSessionSystemPassword(&m_protocolCallContext, gatheringId,
                                                                            m_string78))
    {
        m_callContext = 0;
        return cResultRequestFailed;
    }
    m_callContext->InitiateCall();
    return 0;
}

session::ICommunityInfoList* NexMatchmakeSession::GetBrowsedCommunityResult()
{
    return m_browsedCommunityInfoList;
}

session::ISessionInfoList* NexMatchmakeSession::GetBrowseMatchmakeSessionResult()
{
    return m_browseMatchmakeSessionResult;
}

const session::ISessionInfo* NexMatchmakeSession::GetSessionInfo() const
{
    return &m_sessionInfo;
}

session::ISessionInfo* NexMatchmakeSession::GetSessionInfo()
{
    return &m_sessionInfo;
}

unsigned int NexMatchmakeSession::GetRandomMatchmakeScore() const
{
    return m_randomMatchmakeScore;
}

Result NexMatchmakeSession::GetStationConnectionInfo(nn::pia::common::CallContext* callContext, unsigned int stationId)
{
    if (callContext == 0)
    {
        return ResultInvalidArgument();
    }
    if (callContext->m_state == common::CallContext::cStateInProgress)
    {
        return ResultInvalidArgument();
    }
    if (m_callContext != 0)
    {
        return transport::cResultInvalidState;
    }
    m_callContext = callContext;
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallInProgress)
    {
        return transport::cResultInvalidState;
    }
    if (!m_matchmakeExtensionClient->GetSessionURLs(&m_protocolCallContext, stationId, m_stationUrlList))
    {
        m_callContext = 0;
        return cResultRequestFailed;
    }
    m_callContext->InitiateCall();
    return 0;
}

Result NexMatchmakeSession::Initialize()
{
    NexMatchMeshLayerController* meshLayerController = static_cast<NexMatchMeshLayerController*>(
        session::g_piaSession->m_meshLayerController);
    if (meshLayerController == 0)
    {
        return transport::cResultInvalidState;
    }
    if (m_ngsBridge != 0)
    {
        return transport::cResultInvalidState;
    }
    if (meshLayerController->GetNgsInterfaceIndexBoundNexFacade() != 0)
    {
        return transport::cResultInvalidState;
    }
    nn::nex::NgsBridgeInterface* ngsBridge = meshLayerController->GetNgsInterface(0);
    if (ngsBridge != 0)
    {
        if (m_ngsBridge != 0)
        {
            return transport::cResultInvalidState;
        }
        m_ngsBridge = ngsBridge;
        m_ownMatchmakeExtensionClient = ::new (m_matchmakeExtensionClientMemory) nn::nex::MatchmakeExtensionClient();
        if (!m_ownMatchmakeExtensionClient->Bind(ngsBridge->GetCredentials()))
        {
            return transport::cResultInvalidState;
        }
    }
    m_matchmakeExtensionClient = m_ownMatchmakeExtensionClient;
    return 0;
}

unsigned char NexMatchmakeSession::IsMatchmakeHost() const
{
    return m_matchmakeHost;
}

unsigned char NexMatchmakeSession::IsProcessingJoinMatchmakeSession() const
{
    return m_processingJoinMatchmakeSession;
}

unsigned char NexMatchmakeSession::IsRandomMatchmakeScoreAvailable() const
{
    return m_randomMatchmakeScoreAvailable;
}

unsigned int NexMatchmakeSession::IsUpdatableSelectionPriority(unsigned char priority)
{
    if (priority == 0 || priority == cSelectionPriorityMax || m_selectionPriorityUpdateTime.m_tick == 0)
    {
        return 1;
    }
    common::Time now;
    now.SetNow();
    common::TimeSpan elapsed = now - m_selectionPriorityUpdateTime;
    unsigned int elapsedSec = static_cast<unsigned int>(elapsed.m_tick / common::TimeSpan::GetTicksPerSec().m_tick);
    if (elapsedSec > cSelectionPriorityUpdateIntervalSec)
    {
        return 1;
    }
    return 0;
}

Result NexMatchmakeSession::JoinCommunity(nn::pia::common::CallContext* callContext)
{
    if (callContext == 0)
    {
        return cResultInvalidArgument;
    }
    if (callContext->m_state == common::CallContext::cStateInProgress)
    {
        return cResultInvalidArgument;
    }
    if (m_callContext != 0)
    {
        return ResultInvalidState();
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallInProgress)
    {
        return transport::cResultInvalidState;
    }
    m_callContext = callContext;
    const nn::nex::PersistentGathering* community = m_persistentGathering;
    if (!m_matchmakeExtensionClient->JoinPersistentGathering(&m_protocolCallContext, community->m_uiValue1, "",
                                                             community->m_strPassword))
    {
        m_callContext = 0;
        return cResultRequestFailed;
    }
    m_callContext->InitiateCall();
    return 0;
}

Result NexMatchmakeSession::JoinMatchmakeSession(nn::pia::common::CallContext* callContext, unsigned int gatheringId)
{
    if (callContext == 0)
    {
        return cResultInvalidArgument;
    }
    if (callContext->m_state == common::CallContext::cStateInProgress)
    {
        return cResultInvalidArgument;
    }
    if (m_callContext != 0)
    {
        return transport::cResultInvalidState;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallInProgress)
    {
        return transport::cResultInvalidState;
    }
    m_callContext = callContext;
    m_joinMatchmakeSessionParam->m_uiGatheringID = gatheringId;
    if (session::g_piaSession != 0 && session::g_piaSession->GetLocalPlayerCount() > 1
        && session::g_piaSession->IsPlayersCountedAsParticipants())
    {
        unsigned int localPlayerCount = session::g_piaSession->GetLocalPlayerCount();
        unsigned int addedParticipantNum = 0;
        if (m_ngsBridge != 0)
        {
            unsigned int principalId = m_ngsBridge->GetPrincipalID();
            if (principalId != NexFacade::GetInstance()->GetLocalPrincipalId())
            {
                m_idList7c->push_back(principalId);
                addedParticipantNum = 1;
            }
        }
        m_joinMatchmakeSessionParam->m_AdditionalParticipants = *m_idList7c;
        unsigned char participationCount = static_cast<unsigned char>(localPlayerCount - addedParticipantNum);
        if (addedParticipantNum == 0)
        {
            m_joinMatchmakeSessionParam->m_uiParticipationCount = participationCount;
        }
        char joinMessage[cJoinMessageBufferSize];
        int length = snprintf(joinMessage, cJoinMessageLengthMax, "%d", participationCount);
        unsigned long long ownerPrincipalId = 0;
        if (m_ngsBridge != 0)
        {
            ownerPrincipalId = m_ngsBridge->GetPrincipalID();
        }
        joinMessage[length] = ' ';
        ++length;
        length += snprintf(joinMessage + length, cJoinMessageLengthMax - length, "%016llx", ownerPrincipalId);
        if (length > cJoinMessageLengthMax)
        {
            length = cJoinMessageLengthMax;
        }
        joinMessage[length] = '\0';
        m_joinMatchmakeSessionParam->m_strJoinMessage = nn::nex::String(joinMessage);
    }
    if (!m_matchmakeExtensionClient->JoinMatchmakeSession(&m_protocolCallContext, *m_joinMatchmakeSessionParam,
                                                          m_foundSession))
    {
        m_callContext = 0;
        return cResultRequestFailed;
    }
    m_temporaryOwnerChangeGatheringId = 0;
    m_processingJoinMatchmakeSession = true;
    m_temporaryOwnerChangeOwnerId = 0;
    m_callContext->InitiateCall();
    return 0;
}

Result NexMatchmakeSession::JoinMatchmakeSessionWithTeam(nn::pia::common::CallContext* callContext,
                                                         unsigned int gatheringId, unsigned long long* teamOwnerIds,
                                                         unsigned short teamCount, unsigned int teamIndex)
{
    if (callContext == 0)
    {
        return cResultInvalidArgument;
    }
    if (callContext->m_state == common::CallContext::cStateInProgress)
    {
        return cResultInvalidArgument;
    }
    if (m_callContext != 0)
    {
        return transport::cResultInvalidState;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallInProgress)
    {
        return transport::cResultInvalidState;
    }
    m_callContext = callContext;
    m_joinMatchmakeSessionParam->m_uiGatheringID = gatheringId;
    for (unsigned short i = 0; i < teamCount; ++i)
    {
        m_idList7c->push_back(static_cast<unsigned int>(teamOwnerIds[i]));
    }
    if (session::g_piaSession != 0 && session::g_piaSession->GetLocalPlayerCount() > 1
        && session::g_piaSession->IsPlayersCountedAsParticipants())
    {
        return transport::cResultInvalidState;
    }
    m_joinMatchmakeSessionParam->m_AdditionalParticipants = *m_idList7c;
    m_joinMatchmakeSessionParam->m_uiGatheringIDForParticipationCheck = teamIndex;
    if (!m_matchmakeExtensionClient->JoinMatchmakeSession(&m_protocolCallContext, *m_joinMatchmakeSessionParam,
                                                          m_foundSession))
    {
        m_callContext = 0;
        return cResultRequestFailed;
    }
    m_callContext->InitiateCall();
    m_temporaryOwnerChangeGatheringId = 0;
    m_processingJoinMatchmakeSession = true;
    m_temporaryOwnerChangeOwnerId = 0;
    return 0;
}

Result NexMatchmakeSession::LeaveMatchmakeSession(nn::pia::common::CallContext* callContext, unsigned int gatheringId)
{
    Result result = cResultRequestFailed;
    if (callContext == 0)
    {
        result = cResultInvalidArgument;
    }
    else
    {
        if (callContext->m_state == common::CallContext::cStateInProgress)
        {
            return cResultInvalidArgument;
        }
        if (m_callContext != 0)
        {
            return transport::cResultInvalidState;
        }
        if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallInProgress)
        {
            return transport::cResultInvalidState;
        }
        m_callContext = callContext;
        if (m_matchmakeExtensionClient->EndParticipation(&m_protocolCallContext, gatheringId, nn::nex::String(L"")))
        {
            m_requestDeadline = common::g_piaJobScheduler->m_dispatchTime
                                + common::TimeSpan(common::TimeSpan::GetTicksPerMSec().m_tick * cRequestTimeoutMsec);
            m_callContext->InitiateCall();
            m_gatheringId = gatheringId;
            return 0;
        }
        m_callContext = 0;
    }
    NexMatchMeshLayerController* meshLayerController = static_cast<NexMatchMeshLayerController*>(
        session::g_piaSession->m_meshLayerController);
    if (meshLayerController != 0)
    {
        meshLayerController->ClearSessionId(gatheringId);
    }
    m_gatheringId = gatheringId;
    return result;
}

Result NexMatchmakeSession::MigrateMatchmakeSessionOwner(nn::pia::common::CallContext* callContext,
                                                         unsigned int gatheringId, unsigned long long* candidateIds,
                                                         unsigned short candidateCount)
{
    if (callContext == 0)
    {
        return cResultInvalidArgument;
    }
    if (callContext->m_state == common::CallContext::cStateInProgress)
    {
        return cResultInvalidArgument;
    }
    if (m_callContext != 0)
    {
        return transport::cResultInvalidState;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallInProgress)
    {
        return transport::cResultInvalidState;
    }
    m_callContext = callContext;
    for (unsigned short i = 0; i < candidateCount; ++i)
    {
        m_idList7c->push_back(static_cast<unsigned int>(candidateIds[i]));
    }
    if (!m_matchmakeExtensionClient->MigrateGatheringOwnership(&m_protocolCallContext, gatheringId, *m_idList7c))
    {
        m_callContext = 0;
        return cResultRequestFailed;
    }
    const common::TimeSpan timeout = common::TimeSpan::FromMSec(cRequestTimeoutMsec);
    m_requestDeadline = common::g_piaJobScheduler->m_dispatchTime + timeout;
    m_callContext->InitiateCall();
    return 0;
}

Result NexMatchmakeSession::ModifyAttribute(nn::pia::common::CallContext* callContext, unsigned int gatheringId,
                                            unsigned int index, unsigned int value)
{
    if (callContext == 0)
    {
        return ResultInvalidArgument();
    }
    if (callContext->m_state == common::CallContext::cStateInProgress)
    {
        return ResultInvalidArgument();
    }
    if (m_callContext != 0)
    {
        return transport::cResultInvalidState;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallInProgress)
    {
        return transport::cResultInvalidState;
    }
    m_callContext = callContext;
    if (!m_matchmakeExtensionClient->ModifyMatchmakeSessionAttribute(&m_protocolCallContext, gatheringId, index, value))
    {
        m_callContext = 0;
        return cResultRequestFailed;
    }
    m_callContext->InitiateCall();
    return 0;
}

Result NexMatchmakeSession::OpenParticipate(nn::pia::common::CallContext* callContext, unsigned int gatheringId)
{
    if (callContext == 0)
    {
        return ResultInvalidArgument();
    }
    if (callContext->m_state == common::CallContext::cStateInProgress)
    {
        return ResultInvalidArgument();
    }
    if (m_callContext != 0)
    {
        return transport::cResultInvalidState;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallInProgress)
    {
        return ResultInvalidState();
    }
    m_callContext = callContext;
    if (!m_matchmakeExtensionClient->OpenParticipation(&m_protocolCallContext, gatheringId))
    {
        m_callContext = 0;
        return cResultRequestFailed;
    }
    m_callContext->InitiateCall();
    return 0;
}

Result NexMatchmakeSession::RandomMatchmakeSession(nn::pia::common::CallContext* callContext)
{
    if (callContext == 0)
    {
        return cResultInvalidArgument;
    }
    if (callContext->m_state == common::CallContext::cStateInProgress)
    {
        return cResultInvalidArgument;
    }
    if (m_callContext != 0)
    {
        return transport::cResultInvalidState;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallInProgress)
    {
        return transport::cResultInvalidState;
    }
    if (session::g_piaSession != 0 && session::g_piaSession->GetLocalPlayerCount() > 1
        && session::g_piaSession->IsPlayersCountedAsParticipants())
    {
        unsigned int localPlayerCount = session::g_piaSession->GetLocalPlayerCount();
        unsigned int addedParticipantNum = 0;
        if (m_ngsBridge != 0)
        {
            unsigned int principalId = m_ngsBridge->GetPrincipalID();
            if (principalId != NexFacade::GetInstance()->GetLocalPrincipalId())
            {
                m_idList7c->push_back(principalId);
                addedParticipantNum = 1;
            }
        }
        m_autoMatchmakeParam->m_AdditionalParticipants = *m_idList7c;
        unsigned char participationCount = static_cast<unsigned char>(localPlayerCount - addedParticipantNum);
        if (addedParticipantNum == 0)
        {
            m_autoMatchmakeParam->m_uiParticipationCount = participationCount;
        }
        char joinMessage[cJoinMessageBufferSize];
        int length = snprintf(joinMessage, cJoinMessageLengthMax, "%d", participationCount);
        unsigned long long ownerPrincipalId = 0;
        if (m_ngsBridge != 0)
        {
            ownerPrincipalId = m_ngsBridge->GetPrincipalID();
        }
        joinMessage[length] = ' ';
        ++length;
        length += snprintf(joinMessage + length, cJoinMessageLengthMax - length, "%016llx", ownerPrincipalId);
        if (length > cJoinMessageLengthMax)
        {
            length = cJoinMessageLengthMax;
        }
        joinMessage[length] = '\0';
        m_autoMatchmakeParam->m_strJoinMessage = nn::nex::String(joinMessage);
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallInProgress)
    {
        return transport::cResultInvalidState;
    }
    if (!m_matchmakeExtensionClient->AutoMatchmake(&m_protocolCallContext, *m_autoMatchmakeParam, m_foundSession))
    {
        return cResultRequestFailed;
    }
    m_callContext = callContext;
    m_callContext->InitiateCall();
    m_randomMatchmakeScoreAvailable = false;
    m_randomMatchmakeScore = 0;
    m_processingJoinMatchmakeSession = true;
    m_temporaryOwnerChangeOwnerId = 0;
    m_temporaryOwnerChangeGatheringId = 0;
    return 0;
}

Result NexMatchmakeSession::RandomMatchmakeSessionWithTeam(nn::pia::common::CallContext* callContext,
                                                           unsigned long long* teamOwnerIds, unsigned short teamCount,
                                                           unsigned int gatheringId)
{
    if (callContext == 0)
    {
        return cResultInvalidArgument;
    }
    if (callContext->m_state == common::CallContext::cStateInProgress)
    {
        return cResultInvalidArgument;
    }
    if (m_callContext != 0)
    {
        return transport::cResultInvalidState;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallInProgress)
    {
        return transport::cResultInvalidState;
    }
    for (unsigned short i = 0; i < teamCount; ++i)
    {
        m_idList7c->push_back(static_cast<unsigned int>(teamOwnerIds[i]));
    }
    if (session::g_piaSession != 0 && session::g_piaSession->GetLocalPlayerCount() > 1
        && session::g_piaSession->IsPlayersCountedAsParticipants())
    {
        return transport::cResultInvalidState;
    }
    m_autoMatchmakeParam->m_AdditionalParticipants = *m_idList7c;
    m_autoMatchmakeParam->m_uiGatheringIDForParticipationCheck = gatheringId;
    if (!m_matchmakeExtensionClient->AutoMatchmake(&m_protocolCallContext, *m_autoMatchmakeParam, m_foundSession))
    {
        return cResultRequestFailed;
    }
    m_temporaryOwnerChangeGatheringId = 0;
    m_randomMatchmakeScoreAvailable = false;
    m_temporaryOwnerChangeOwnerId = 0;
    m_randomMatchmakeScore = 0;
    m_callContext = callContext;
    m_processingJoinMatchmakeSession = true;
    m_callContext->InitiateCall();
    return 0;
}

Result NexMatchmakeSession::ReflectCommunityIdList(const unsigned int* idList, unsigned int count)
{
    for (unsigned int i = 0; i < count; ++i)
    {
        m_idList68->push_back(idList[i]);
    }
    return 0;
}

Result NexMatchmakeSession::ReflectCreateCommunitySetting(const NexCreateCommunitySetting* setting)
{
    m_persistentGathering->Reset();
    m_persistentGathering->m_uiShortValue2 = setting->GetMaxParticipantNum();
    m_persistentGathering->m_uiShortValue1 = setting->GetMinParticipantNum();
    const common::DateTime* startDate = setting->GetParticipaitonStartDate();
    if (!startDate->IsValid())
    {
        m_persistentGathering->m_ParticipationStartDate = nn::nex::DateTime::s_never;
    }
    else
    {
        nn::nex::DateTime participationStartDate(startDate->GetYear(), startDate->GetMonth(), startDate->GetDay(),
                                                 startDate->GetHour(), startDate->GetMinute(), startDate->GetSecond());
        if (!participationStartDate.IsValid())
        {
            return cResultInvalidArgument;
        }
        m_persistentGathering->m_ParticipationStartDate = participationStartDate;
    }
    const common::DateTime* endDate = setting->GetParticipaitonEndDate();
    if (!endDate->IsValid())
    {
        m_persistentGathering->m_ParticipationEndDate = nn::nex::DateTime::s_never;
    }
    else
    {
        nn::nex::DateTime participationEndDate(endDate->GetYear(), endDate->GetMonth(), endDate->GetDay(),
                                               endDate->GetHour(), endDate->GetMinute(), endDate->GetSecond());
        if (!participationEndDate.IsValid())
        {
            return cResultInvalidArgument;
        }
        m_persistentGathering->m_ParticipationEndDate = participationEndDate;
    }
    nn::nex::qVector<unsigned char> applicationData;
    setting->CopyApplicationDataToVector(&applicationData);
    m_persistentGathering->m_ApplicationBuffer = applicationData;
    for (unsigned int i = 0; i < cSessionAttributeNum; ++i)
    {
        if (setting->IsSetAttribute(i))
        {
            m_persistentGathering->SetAttribute(i, setting->GetAttribute(i));
        }
    }
    if (setting->GetDescription()->GetEncodingType() == common::String::EncodingType_Utf16)
    {
        m_persistentGathering->SetDescription(
            nn::nex::String(static_cast<const wchar_t*>(setting->GetDescription()->GetBuffer())));
    }
    else
    {
        m_persistentGathering->SetDescription(
            nn::nex::String(static_cast<const char*>(setting->GetDescription()->GetBuffer())));
    }
    if (setting->GetCommunityUserPassword()->GetStringLength() == 0)
    {
        m_persistentGathering->SetPersistentGatheringType(
            nn::nex::PersistentGatheringType::cPersistentGatheringType_Open);
    }
    else
    {
        nn::nex::PersistentGathering* community = m_persistentGathering;
        if (setting->GetCommunityUserPassword()->GetEncodingType() == common::String::EncodingType_Utf16)
        {
            community->m_strPassword = nn::nex::String(
                static_cast<const wchar_t*>(setting->GetCommunityUserPassword()->GetBuffer()));
        }
        else
        {
            community->m_strPassword = nn::nex::String(
                static_cast<const char*>(setting->GetCommunityUserPassword()->GetBuffer()));
        }
        community->SetPersistentGatheringType(
            nn::nex::PersistentGatheringType::cPersistentGatheringType_PasswordLocked);
    }
    return 0;
}

Result NexMatchmakeSession::ReflectCreateSetting(const nn::pia::session::CreateSessionSetting* setting,
                                                 bool hostMigrationEnabled)
{
    if (ReflectSettingToMatchmakeSession(setting, hostMigrationEnabled) != 0)
    {
        return static_cast<Result>(transport::cResultInvalidState);
    }
    m_createMatchmakeSessionParam->Reset();
    m_createMatchmakeSessionParam->m_MatchmakeSession = *m_requestSession;
    const NexCreateSessionSetting* createSetting = static_cast<const NexCreateSessionSetting*>(setting);
    if (createSetting->IsSetMatchmakeSessionOption())
    {
        m_createMatchmakeSessionParam->m_uiCreateMatchmakeSessionOption = createSetting->GetMatchmakeSessionOption();
    }
    return static_cast<Result>(0);
}

Result NexMatchmakeSession::ReflectJoinCommunitySetting(const NexJoinCommunitySetting* setting)
{
    m_persistentGathering->Reset();
    m_persistentGathering->m_uiValue1 = setting->GetCommunityId();
    if (setting->GetCommunityUserPassword().GetStringLength() > 0)
    {
        if (setting->GetCommunityUserPassword().GetEncodingType() == common::String::EncodingType_Utf16)
        {
            nn::nex::PersistentGathering* community = m_persistentGathering;
            community->m_strPassword = nn::nex::String(
                static_cast<const wchar_t*>(setting->GetCommunityUserPassword().GetBuffer()));
        }
        else
        {
            nn::nex::PersistentGathering* community = m_persistentGathering;
            community->m_strPassword = nn::nex::String(
                static_cast<const char*>(setting->GetCommunityUserPassword().GetBuffer()));
        }
    }
    return 0;
}

Result NexMatchmakeSession::ReflectJoinRandomSetting(const nn::pia::session::JoinRandomSessionSetting* setting,
                                                     bool isTeam)
{
    if (RegisterSearchCriteria(setting->GetSessionSearchCriteria(0),
                               static_cast<unsigned char>(setting->GetSessionSearchCriteriaSize()))
        != 0)
    {
        return cResultInvalidArgument;
    }
    if (ReflectSettingToMatchmakeSession(setting->GetCreateSessionSetting(), isTeam) != 0)
    {
        return transport::cResultInvalidState;
    }
    const NexCreateSessionSetting* createSetting = static_cast<const NexCreateSessionSetting*>(
        setting->GetCreateSessionSetting());
    m_autoMatchmakeParam->Reset();
    m_autoMatchmakeParam->m_SearchCriteria = *m_searchCriteriaList;
    m_autoMatchmakeParam->m_MatchmakeSession = *m_requestSession;
    if (createSetting->IsSetMatchmakeSessionOption() && createSetting->GetMatchmakeSessionOption() != 0)
    {
        m_autoMatchmakeParam->m_uiAutoMatchmakeOption = createSetting->GetMatchmakeSessionOption();
    }
    return 0;
}

Result NexMatchmakeSession::ReflectJoinSetting(const nn::pia::session::JoinSessionSetting* setting)
{
    if (setting == 0)
    {
        return cResultInvalidArgument;
    }
    const NexJoinSessionSetting* joinSetting = static_cast<const NexJoinSessionSetting*>(setting);
    m_joinMatchmakeSessionParam->Reset();
    nn::nex::JoinMatchmakeSessionParam* param = m_joinMatchmakeSessionParam;
    if (joinSetting->GetSessionUserPassword().GetEncodingType() == common::String::EncodingType_Utf16)
    {
        param->m_strUserPassword = nn::nex::String(
            static_cast<const wchar_t*>(joinSetting->GetSessionUserPassword().GetBuffer()));
    }
    else
    {
        param->m_strUserPassword = nn::nex::String(
            static_cast<const char*>(joinSetting->GetSessionUserPassword().GetBuffer()));
    }
    param = m_joinMatchmakeSessionParam;
    if (joinSetting->GetSessionSystemPassword().GetEncodingType() == common::String::EncodingType_Utf16)
    {
        param->m_strSystemPassword = nn::nex::String(
            static_cast<const wchar_t*>(joinSetting->GetSessionSystemPassword().GetBuffer()));
    }
    else
    {
        param->m_strSystemPassword = nn::nex::String(
            static_cast<const char*>(joinSetting->GetSessionSystemPassword().GetBuffer()));
    }
    if (joinSetting->GetExtraParticipantNum() != 0)
    {
        m_joinMatchmakeSessionParam->m_uiExtraParticipants = joinSetting->GetExtraParticipantNum();
    }
    if (joinSetting->IsSetMatchmakeSessionOption())
    {
        m_joinMatchmakeSessionParam->m_uiJoinMatchmakeSessionOption = joinSetting->GetMatchmakeSessionOption();
    }
    return 0;
}

Result NexMatchmakeSession::ReflectSettingToMatchmakeSession(const nn::pia::session::CreateSessionSetting* setting,
                                                             bool hostMigrationEnabled)
{
    if (setting == 0)
    {
        return static_cast<Result>(cResultInvalidArgument);
    }
    const NexCreateSessionSetting* createSetting = static_cast<const NexCreateSessionSetting*>(setting);
    m_requestSession->Reset();
    if (createSetting->IsSetGameMode())
    {
        m_requestSession->SetGameMode(createSetting->GetGameMode());
    }
    if (createSetting->IsSetMaxParticipantNum())
    {
        m_requestSession->m_uiShortValue2 = createSetting->GetMaxParticipantNum();
    }
    if (createSetting->IsSetMinParticipantNum())
    {
        m_requestSession->m_uiShortValue1 = createSetting->GetMinParticipantNum();
    }
    nn::nex::MatchmakeSystemType systemType = nn::nex::cMatchmakeSystemType_1;
    unsigned int policyArgument = 0;
    if (createSetting->IsSetSessionType())
    {
        systemType = ConvertSessionTypeToMatchmakeSystemType(createSetting->GetSessionType());
        if (systemType == nn::nex::cMatchmakeSystemType_1)
        {
            if (createSetting->IsAutoCloseSession())
            {
                policyArgument = 1;
            }
            else
            {
                policyArgument = 0;
            }
        }
        else if (systemType == nn::nex::cMatchmakeSystemType_2)
        {
            if (createSetting->IsAutoCloseSession())
            {
                policyArgument = 0;
            }
            else
            {
                policyArgument = 1;
            }
        }
    }
    m_requestSession->SetMatchmakeSystemType(systemType, policyArgument);
    if (createSetting->IsSetSessionUserPassword())
    {
        common::String::EncodingType encoding = createSetting->GetSessionUserPassword().GetEncodingType();
        nn::nex::MatchmakeSession* session = m_requestSession;
        if (encoding == common::String::EncodingType_Utf16)
        {
            session->m_strUserPassword = nn::nex::String(
                static_cast<const wchar_t*>(createSetting->GetSessionUserPassword().GetBuffer()));
        }
        else
        {
            session->m_strUserPassword = nn::nex::String(
                static_cast<const char*>(createSetting->GetSessionUserPassword().GetBuffer()));
        }
    }
    if (createSetting->IsSetOpenSession())
    {
        m_requestSession->SetOpenParticipation(createSetting->IsOpenSession());
    }
    if (createSetting->IsSetSelectionPriority())
    {
        m_requestSession->SetProgressScore(createSetting->GetSelectionPriority());
    }
    if (createSetting->IsSetCommunityId())
    {
        m_requestSession->m_uiReferGatheringID = createSetting->GetCommunityId();
    }
    m_matchmakeParam->m_Parameters.clear();
    if (createSetting->IsSetRatingValue())
    {
        m_matchmakeParam->m_Parameters[nn::nex::String(s_ratingValueKey)] = nn::nex::Variant(
            createSetting->GetRatingValue());
    }
    if (createSetting->IsSetDisconnectionRate())
    {
        m_matchmakeParam->m_Parameters[nn::nex::String(s_disconnectionRateKey)] = nn::nex::Variant(
            createSetting->GetDisconnectionRate());
    }
    if (createSetting->IsSetViolationRate())
    {
        m_matchmakeParam->m_Parameters[nn::nex::String(s_violationRateKey)] = nn::nex::Variant(
            createSetting->GetViolationRate());
    }
    if (createSetting->IsSetUseGeoIp())
    {
        m_matchmakeParam->m_Parameters[nn::nex::String(s_useGeoIpKey)] = nn::nex::Variant(createSetting->GetUseGeoIp());
    }
    if (createSetting->IsSetCountryCode())
    {
        m_matchmakeParam->m_Parameters[nn::nex::String(s_countryCodeKey)] = nn::nex::Variant(
            createSetting->GetCountryCode());
    }
    if (createSetting->IsSetOverrideIpAddress())
    {
        NexFacade::ConvertInetAddressToNexInetAddress(createSetting->GetOverrideIpAddress(), m_inetAddress);
        m_matchmakeParam->m_Parameters[nn::nex::String(s_overrideIpAddressKey)] = nn::nex::Variant(
            m_inetAddress->GetAddressStr());
    }
    for (unsigned int i = 0; i < NexCreateSessionSetting::cAdditionalScoreBasedParameterValueNum; ++i)
    {
        const ParamVariant* value = createSetting->GetAdditionalScoreBasedParameterValue(i);
        const common::String* key = createSetting->GetAdditionalScoreBasedParameterValueKey(i);
        if (key == 0 || key->GetStringLength() == 0)
        {
            continue;
        }
        switch (value->m_type)
        {
        case ParamVariant::cType_UInt64:
            m_matchmakeParam->m_Parameters[nn::nex::String(static_cast<const wchar_t*>(key->GetBuffer()))] =
                nn::nex::Variant(value->m_uint64Value);
            break;
        case ParamVariant::cType_Int64:
            m_matchmakeParam->m_Parameters[nn::nex::String(static_cast<const wchar_t*>(key->GetBuffer()))] =
                nn::nex::Variant(value->m_int64Value);
            break;
        case ParamVariant::cType_UInt32:
            m_matchmakeParam->m_Parameters[nn::nex::String(static_cast<const wchar_t*>(key->GetBuffer()))] =
                nn::nex::Variant(value->m_uint32Value);
            break;
        case ParamVariant::cType_Int32:
            m_matchmakeParam->m_Parameters[nn::nex::String(static_cast<const wchar_t*>(key->GetBuffer()))] =
                nn::nex::Variant(value->m_int32Value);
            break;
        case ParamVariant::cType_Bool:
            m_matchmakeParam->m_Parameters[nn::nex::String(static_cast<const wchar_t*>(key->GetBuffer()))] =
                nn::nex::Variant(value->m_boolValue);
            break;
        case ParamVariant::cType_Double:
            m_matchmakeParam->m_Parameters[nn::nex::String(static_cast<const wchar_t*>(key->GetBuffer()))] =
                nn::nex::Variant(value->m_doubleValue);
            break;
        default:
            break;
        }
    }
    if (createSetting->GetAdditionalScoreBasedParameterStringKey(0) != 0
        && createSetting->GetAdditionalScoreBasedParameterString(0) != 0)
    {
        m_matchmakeParam->m_Parameters[nn::nex::String(
            static_cast<const wchar_t*>(createSetting->GetAdditionalScoreBasedParameterStringKey(0)->GetBuffer()))] =
            nn::nex::Variant(
                static_cast<const wchar_t*>(createSetting->GetAdditionalScoreBasedParameterString(0)->GetBuffer()));
    }
    if (createSetting->IsSetScoreBasedParameter())
    {
        m_requestSession->m_MatchmakeParam = *m_matchmakeParam;
    }
    common::g_sessionBeginMonitoringContent.m_metric81 = createSetting->GetGameMode();
    common::g_sessionBeginMonitoringContent.m_metric83 = createSetting->GetMaxParticipantNum();
    common::g_sessionBeginMonitoringContent.m_metric108 = createSetting->GetExtraParticipantNum();
    common::g_sessionStateMonitoringContent.m_metric223[1] = createSetting->IsOpenSession();
    if (createSetting->IsSetApplicationData())
    {
        nn::nex::qVector<unsigned char> applicationData;
        createSetting->CopyApplicationDataToVector(&applicationData);
        m_requestSession->SetApplicationBuffer(applicationData);
    }
    for (unsigned int i = 0; i < NexCreateSessionSetting::cAttributeNum; ++i)
    {
        if (createSetting->IsSetAttribute(i))
        {
            m_requestSession->SetAttribute(i, createSetting->GetAttribute(i));
            common::g_sessionBeginMonitoringContent.m_metric82[i] = createSetting->GetAttribute(i);
        }
    }
    unsigned int flags = 0;
    if (hostMigrationEnabled)
    {
        flags = cGatheringFlagsHostMigration;
    }
    m_requestSession->m_uiValue4 |= flags | cGatheringFlagsMatchmake;
    return static_cast<Result>(0);
}

Result NexMatchmakeSession::ReflectUpdateCommunitySetting(const NexUpdateCommunitySetting* setting)
{
    if (!setting->IsValid())
    {
        return cResultInvalidArgument;
    }
    m_persistentGathering->Reset();
    m_persistentGathering->m_uiValue1 = setting->GetCommunityId();
    m_persistentGathering->m_uiShortValue2 = setting->GetMaxParticipantNum();
    m_persistentGathering->m_uiShortValue1 = setting->GetMinParticipantNum();
    const common::DateTime* startDate = &setting->GetParticipaitonStartDate();
    if (!startDate->IsValid())
    {
        m_persistentGathering->m_ParticipationStartDate = nn::nex::DateTime::s_never;
    }
    else
    {
        nn::nex::DateTime participationStartDate(startDate->GetYear(), startDate->GetMonth(), startDate->GetDay(),
                                                 startDate->GetHour(), startDate->GetMinute(), startDate->GetSecond());
        if (!participationStartDate.IsValid())
        {
            return cResultInvalidArgument;
        }
        m_persistentGathering->m_ParticipationStartDate = participationStartDate;
    }
    const common::DateTime* endDate = &setting->GetParticipaitonEndDate();
    if (!endDate->IsValid())
    {
        m_persistentGathering->m_ParticipationEndDate = nn::nex::DateTime::s_never;
    }
    else
    {
        nn::nex::DateTime participationEndDate(endDate->GetYear(), endDate->GetMonth(), endDate->GetDay(),
                                               endDate->GetHour(), endDate->GetMinute(), endDate->GetSecond());
        if (!participationEndDate.IsValid())
        {
            return cResultInvalidArgument;
        }
        m_persistentGathering->m_ParticipationEndDate = participationEndDate;
    }
    nn::nex::qVector<unsigned char> applicationData;
    setting->CopyApplicationDataToVector(&applicationData);
    m_persistentGathering->m_ApplicationBuffer = applicationData;
    for (unsigned int i = 0; i < cSessionAttributeNum; ++i)
    {
        m_persistentGathering->SetAttribute(i, setting->GetAttribute(i));
    }
    if (setting->GetDescription().GetEncodingType() == common::String::EncodingType_Utf16)
    {
        m_persistentGathering->SetDescription(
            nn::nex::String(static_cast<const wchar_t*>(setting->GetDescription().GetBuffer())));
    }
    else
    {
        m_persistentGathering->SetDescription(
            nn::nex::String(static_cast<const char*>(setting->GetDescription().GetBuffer())));
    }
    if (setting->IsSetCommunityUserPassword())
    {
        m_persistentGathering->SetPersistentGatheringType(
            nn::nex::PersistentGatheringType::cPersistentGatheringType_PasswordLocked);
    }
    if (setting->GetCommunityUserPassword().GetStringLength() != 0)
    {
        nn::nex::PersistentGathering* community = m_persistentGathering;
        if (setting->GetCommunityUserPassword().GetEncodingType() == common::String::EncodingType_Utf16)
        {
            community->m_strPassword = nn::nex::String(
                static_cast<const wchar_t*>(setting->GetCommunityUserPassword().GetBuffer()));
        }
        else
        {
            community->m_strPassword = nn::nex::String(
                static_cast<const char*>(setting->GetCommunityUserPassword().GetBuffer()));
        }
    }
    return 0;
}

Result NexMatchmakeSession::ReflectUpdateSetting(const nn::pia::session::UpdateSessionSetting* setting,
                                                 bool isJointSession)
{
    if (setting == 0)
    {
        return cResultInvalidArgument;
    }
    const NexUpdateSessionSetting* updateSetting = static_cast<const NexUpdateSessionSetting*>(setting);
    m_updateMatchmakeSessionParam->Reset();
    unsigned int modificationFlag = updateSetting->GetNexModificationFlag();
    if (isJointSession)
    {
        modificationFlag |= cModificationFlagOpenParticipation;
    }
    m_updateMatchmakeSessionParam->m_uiValue2 = modificationFlag;
    m_updateMatchmakeSessionParam->m_uiValue5 = updateSetting->GetMaxParticipantNum();
    m_updateMatchmakeSessionParam->m_uiValue4 = updateSetting->GetMinParticipantNum();
    m_updateMatchmakeSessionParam->m_bOpenParticipation = isJointSession;
    nn::nex::MatchmakeSystemType systemType = ConvertSessionTypeToMatchmakeSystemType(updateSetting->GetSessionType());
    unsigned int policyArgument = 0;
    if ((systemType == nn::nex::cMatchmakeSystemType_1 && updateSetting->IsAutoCloseSession())
        || (systemType == nn::nex::cMatchmakeSystemType_2 && !updateSetting->IsAutoCloseSession()))
    {
        policyArgument = 1;
    }
    nn::nex::UpdateMatchmakeSessionParam* param = m_updateMatchmakeSessionParam;
    param->m_uiValue6 = systemType;
    switch (systemType)
    {
    case nn::nex::cMatchmakeSystemType_1:
        param->m_uiValue7 = cParticipationPolicyNormal;
        param->m_uiValue8 = policyArgument;
        break;
    case nn::nex::cMatchmakeSystemType_2:
        param->m_uiValue7 = cParticipationPolicyRanked;
        param->m_uiValue8 = policyArgument;
        break;
    default:
        param->m_uiValue7 = cParticipationPolicyDefault;
        param->m_uiValue8 = 0;
        break;
    }
    param = m_updateMatchmakeSessionParam;
    if (updateSetting->GetSessionUserPassword().GetEncodingType() == common::String::EncodingType_Utf16)
    {
        param->m_strValue1 = nn::nex::String(
            static_cast<const wchar_t*>(updateSetting->GetSessionUserPassword().GetBuffer()));
    }
    else
    {
        param->m_strValue1 = nn::nex::String(
            static_cast<const char*>(updateSetting->GetSessionUserPassword().GetBuffer()));
    }
    param = m_updateMatchmakeSessionParam;
    unsigned char selectionPriority = updateSetting->GetSelectionPriority();
    if (selectionPriority <= nn::nex::UpdateMatchmakeSessionParam::MAX_PROGRESS_SCORE)
    {
        param->m_uiProgressScore = selectionPriority;
    }
    nn::nex::qVector<unsigned char> applicationData;
    updateSetting->CopyApplicationDataToVector(&applicationData);
    m_updateMatchmakeSessionParam->m_ApplicationBuffer = applicationData;
    unsigned int attributes[NexUpdateSessionSetting::cAttributeNum];
    for (unsigned int i = 0; i < NexUpdateSessionSetting::cAttributeNum; ++i)
    {
        attributes[i] = updateSetting->GetAttribute(i);
    }
    m_updateMatchmakeSessionParam->m_Attributes.assign(attributes, attributes + NexUpdateSessionSetting::cAttributeNum);
    if (!updateSetting->GetStartedTime().IsValid())
    {
        m_updateMatchmakeSessionParam->m_StartedTime = nn::nex::DateTime::s_never;
    }
    else
    {
        m_updateMatchmakeSessionParam->m_StartedTime = nn::nex::DateTime(
            updateSetting->GetStartedTime().GetYear(), updateSetting->GetStartedTime().GetMonth(),
            updateSetting->GetStartedTime().GetDay(), updateSetting->GetStartedTime().GetHour(),
            updateSetting->GetStartedTime().GetMinute(), updateSetting->GetStartedTime().GetSecond());
    }
    m_matchmakeParam->m_Parameters.clear();
    if (updateSetting->IsSetScoreBaseParam())
    {
        if (updateSetting->IsSetRatingValue())
        {
            m_matchmakeParam->m_Parameters[nn::nex::String(s_ratingValueKey)] = nn::nex::Variant(
                updateSetting->GetRatingValue());
        }
        if (updateSetting->IsSetDisconnectionRate())
        {
            m_matchmakeParam->m_Parameters[nn::nex::String(s_disconnectionRateKey)] = nn::nex::Variant(
                updateSetting->GetDisconnectionRate());
        }
        if (updateSetting->IsSetViolationRate())
        {
            m_matchmakeParam->m_Parameters[nn::nex::String(s_violationRateKey)] = nn::nex::Variant(
                updateSetting->GetViolationRate());
        }
        if (updateSetting->IsSetCountryCode())
        {
            m_matchmakeParam->m_Parameters[nn::nex::String(s_countryCodeKey)] = nn::nex::Variant(
                updateSetting->GetCountryCode());
        }
        if (updateSetting->GetUpdateGeoIp())
        {
            m_matchmakeParam->m_Parameters[nn::nex::String(s_updateGeoIpKey)] = nn::nex::Variant(
                updateSetting->GetUpdateGeoIp());
        }
        for (unsigned int i = 0; i < NexUpdateSessionSetting::cAttributeNum / 2; ++i)
        {
            const ParamVariant* value = updateSetting->GetAdditionalScoreBasedParameterValue(i);
            const common::String* key = updateSetting->GetAdditionalScoreBasedParameterValueKey(i);
            if (key == 0 || key->GetStringLength() == 0)
            {
                continue;
            }
            switch (value->m_type)
            {
            case ParamVariant::cType_UInt64:
                m_matchmakeParam->m_Parameters[nn::nex::String(static_cast<const wchar_t*>(key->GetBuffer()))] =
                    nn::nex::Variant(value->m_uint64Value);
                break;
            case ParamVariant::cType_Int64:
                m_matchmakeParam->m_Parameters[nn::nex::String(static_cast<const wchar_t*>(key->GetBuffer()))] =
                    nn::nex::Variant(value->m_int64Value);
                break;
            case ParamVariant::cType_UInt32:
                m_matchmakeParam->m_Parameters[nn::nex::String(static_cast<const wchar_t*>(key->GetBuffer()))] =
                    nn::nex::Variant(value->m_uint32Value);
                break;
            case ParamVariant::cType_Int32:
                m_matchmakeParam->m_Parameters[nn::nex::String(static_cast<const wchar_t*>(key->GetBuffer()))] =
                    nn::nex::Variant(value->m_int32Value);
                break;
            case ParamVariant::cType_Bool:
                m_matchmakeParam->m_Parameters[nn::nex::String(static_cast<const wchar_t*>(key->GetBuffer()))] =
                    nn::nex::Variant(value->m_boolValue);
                break;
            case ParamVariant::cType_Double:
                m_matchmakeParam->m_Parameters[nn::nex::String(static_cast<const wchar_t*>(key->GetBuffer()))] =
                    nn::nex::Variant(value->m_doubleValue);
                break;
            default:
                break;
            }
        }
        if (updateSetting->GetAdditionalScoreBasedParameterStringKey(0) != 0
            && updateSetting->GetAdditionalScoreBasedParameterString(0) != 0)
        {
            m_matchmakeParam->m_Parameters[nn::nex::String(static_cast<const wchar_t*>(
                updateSetting->GetAdditionalScoreBasedParameterStringKey(0)->GetBuffer()))] =
                nn::nex::Variant(
                    static_cast<const wchar_t*>(updateSetting->GetAdditionalScoreBasedParameterString(0)->GetBuffer()));
        }
        m_updateMatchmakeSessionParam->m_MatchmakeParam = *m_matchmakeParam;
    }
    return 0;
}

void NexMatchmakeSession::RegisterFindCommunityResult()
{
    for (nn::nex::qList<nn::nex::PersistentGathering>::const_iterator it = m_persistentGatheringList->begin();
         it != m_persistentGatheringList->end(); ++it)
    {
        NexCommunityInfo* communityInfo =
            static_cast<NexCommunityInfoList*>(m_browsedCommunityInfoList)->AddCommunityInfo();
        if (communityInfo == 0)
        {
            return;
        }
        CopyFoundPersistentGatheringToBrowsedCommunityInfoList(&*it, communityInfo);
    }
}

void NexMatchmakeSession::RegisterFindSessionResult(nn::pia::session::SessionSearchCriteria::SearchCriteriaKey keyType)
{
    typedef session::SessionInfoList<NexSessionInfoPrivate> SessionInfoList;
    if (keyType == session::SessionSearchCriteria::cKeyTypeDefault
        || keyType == session::SessionSearchCriteria::cKeyTypeOwner)
    {
        for (nn::nex::qList<nn::nex::AnyObjectHolder<nn::nex::Gathering, nn::nex::String> >::const_iterator it =
                 m_gatheringList->begin();
             it != m_gatheringList->end(); ++it)
        {
            if (it->GetObject()->IsA(nn::nex::String("MatchmakeSession")) == true)
            {
                SessionInfoList* sessionInfoList = static_cast<SessionInfoList*>(m_browseMatchmakeSessionResult);
                const nn::nex::MatchmakeSession* foundSession = static_cast<const nn::nex::MatchmakeSession*>(
                    it->GetObject());
                session::ISessionInfo* sessionInfo = 0;
                if (sessionInfoList->m_maxSize != 0 && sessionInfoList->m_size < sessionInfoList->m_maxSize)
                {
                    sessionInfo = sessionInfoList->m_items[sessionInfoList->m_size];
                    sessionInfo->Clear();
                    ++sessionInfoList->m_size;
                }
                if (sessionInfo == 0)
                {
                    return;
                }
                NexSessionInfo* nexSessionInfo = static_cast<NexSessionInfo*>(sessionInfo);
                CopyMatchmakeSessionToSessionInfo(foundSession, nexSessionInfo);
                if (keyType == session::SessionSearchCriteria::cKeyTypeOwner)
                {
                    nexSessionInfo->SetTargetPrincipalId(foundSession->m_uiValue2);
                }
                else
                {
                    nexSessionInfo->SetTargetPrincipalId(0);
                }
                nexSessionInfo->Trace(cTraceSessionInfo);
            }
        }
    }
    else if (keyType == session::SessionSearchCriteria::cKeyTypeParticipant)
    {
        for (nn::nex::qList<nn::nex::PlayingSession>::const_iterator it = m_playingSessionList->begin();
             it != m_playingSessionList->end(); ++it)
        {
            const nn::nex::MatchmakeSession* playingSession = it->GetMatchmakeSession();
            SessionInfoList* sessionInfoList = static_cast<SessionInfoList*>(m_browseMatchmakeSessionResult);
            session::ISessionInfo* sessionInfo = 0;
            if (sessionInfoList->m_maxSize != 0 && sessionInfoList->m_size < sessionInfoList->m_maxSize)
            {
                sessionInfo = sessionInfoList->m_items[sessionInfoList->m_size];
                sessionInfo->Clear();
                ++sessionInfoList->m_size;
            }
            if (sessionInfo == 0)
            {
                return;
            }
            NexSessionInfo* nexSessionInfo = static_cast<NexSessionInfo*>(sessionInfo);
            CopyMatchmakeSessionToSessionInfo(playingSession, nexSessionInfo);
            nexSessionInfo->SetTargetPrincipalId(it->m_uiPrincipalID);
            nexSessionInfo->Trace(cTraceSessionInfo);
        }
    }
}

Result NexMatchmakeSession::RegisterSearchCriteria(const nn::pia::session::SessionSearchCriteria* criteria,
                                                   unsigned char criteriaNum)
{
    UnregisterSearchCriteria();
    const NexSessionSearchCriteria* nexCriteria = static_cast<const NexSessionSearchCriteria*>(criteria);
    for (unsigned char index = 0; index < criteriaNum; ++index)
    {
        if (!nexCriteria[index].IsValid())
        {
            return cResultInvalidArgument;
        }
        nn::nex::MatchmakeSessionSearchCriteria searchCriteria;
        nn::nex::ResultRange resultRange;
        Result result = nexCriteria[index].ConvertToNexMatchmakeSessionSearchCriteria(
            &searchCriteria, m_inetAddress, m_matchmakeParam, &resultRange, index);
        if (result != 0)
        {
            return result;
        }
        m_searchCriteriaList->push_back(searchCriteria);
        m_resultRangeList->push_back(resultRange);
    }
    return 0;
}

Result NexMatchmakeSession::RequestSessionInfo(nn::pia::common::CallContext* callContext, unsigned int gatheringId)
{
    if (callContext == 0)
    {
        return cResultInvalidArgument;
    }
    if (callContext->m_state == common::CallContext::cStateInProgress)
    {
        return cResultInvalidArgument;
    }
    if (m_callContext != 0)
    {
        return transport::cResultInvalidState;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallInProgress)
    {
        return ResultInvalidState();
    }
    m_callContext = callContext;
    if (!m_matchmakeExtensionClient->FindMatchmakeSessionByGatheringIdDetail(&m_protocolCallContext, gatheringId,
                                                                             m_foundSession))
    {
        m_callContext = 0;
        return Result(cResultRequestFailed);
    }
    common::TimeSpan timeout = common::TimeSpan::FromMSec(cRequestTimeoutMsec);
    m_requestDeadline = common::g_piaJobScheduler->m_dispatchTime + timeout;
    m_callContext->InitiateCall();
    return 0;
}

Result NexMatchmakeSession::RequestSessionInfoWithoutPermission(nn::pia::common::CallContext* callContext,
                                                                unsigned int gatheringId)
{
    if (callContext == 0)
    {
        return cResultInvalidArgument;
    }
    if (callContext->m_state == common::CallContext::cStateInProgress)
    {
        return cResultInvalidArgument;
    }
    if (m_callContext != 0)
    {
        return transport::cResultInvalidState;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallInProgress)
    {
        return ResultInvalidState();
    }
    m_callContext = callContext;
    if (!m_matchmakeExtensionClient->FindByID(&m_protocolCallContext, gatheringId, &m_gatheringHolder))
    {
        m_callContext = 0;
        return Result(cResultRequestFailed);
    }
    common::TimeSpan timeout = common::TimeSpan::FromMSec(cRequestTimeoutMsec);
    m_requestDeadline = common::g_piaJobScheduler->m_dispatchTime + timeout;
    m_callContext->InitiateCall();
    return 0;
}

void NexMatchmakeSession::SetMatchmakeCurrentParticipantNum(unsigned short participantNum)
{
}

void NexMatchmakeSession::SetMatchmakeOwnerUniqueId(unsigned long long ownerId)
{
    CommonMatchmakeSession::SetMatchmakeOwnerUniqueId(ownerId);
}

Result NexMatchmakeSession::SetMatchmakeSessionKey(const unsigned char* key, unsigned int keySize)
{
    OSBlockSet(m_matchmakeSessionKey, 0, sizeof(m_matchmakeSessionKey));
    if (key == 0 || keySize != sizeof(m_matchmakeSessionKey))
    {
        return transport::cResultInvalidArgument;
    }
    OSBlockMove(m_matchmakeSessionKey, key, sizeof(m_matchmakeSessionKey), FALSE);
    return 0;
}

Result NexMatchmakeSession::SetMatchmakeSessionKey(const nn::nex::qVector<unsigned char>& key)
{
    OSBlockSet(m_matchmakeSessionKey, 0, sizeof(m_matchmakeSessionKey));
    unsigned int index = 0;
    nn::nex::qVector<unsigned char>::const_iterator it = key.begin();
    while (it != key.end() && index < sizeof(m_matchmakeSessionKey))
    {
        m_matchmakeSessionKey[index++] = *it++;
    }
    return 0;
}

void NexMatchmakeSession::SetTemporaryOwnerChangeEvent(unsigned int gatheringId, unsigned long long ownerId)
{
    m_temporaryOwnerChangeGatheringId = gatheringId;
    m_temporaryOwnerChangeOwnerId = ownerId;
}

Result NexMatchmakeSession::UnregisterMatchmakeSession(nn::pia::common::CallContext* callContext,
                                                       unsigned int gatheringId)
{
    Result result = cResultRequestFailed;
    if (callContext == 0)
    {
        result = cResultInvalidArgument;
    }
    else
    {
        if (callContext->m_state == common::CallContext::cStateInProgress)
        {
            return cResultInvalidArgument;
        }
        if (m_callContext != 0)
        {
            return transport::cResultInvalidState;
        }
        if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallInProgress)
        {
            return transport::cResultInvalidState;
        }
        m_callContext = callContext;
        if (m_matchmakeExtensionClient->UnregisterGathering(&m_protocolCallContext, gatheringId))
        {
            m_requestDeadline = common::g_piaJobScheduler->m_dispatchTime
                                + common::TimeSpan(common::TimeSpan::GetTicksPerMSec().m_tick * cRequestTimeoutMsec);
            m_callContext->InitiateCall();
            m_gatheringId = gatheringId;
            return 0;
        }
        m_callContext = 0;
    }
    NexMatchMeshLayerController* meshLayerController = static_cast<NexMatchMeshLayerController*>(
        session::g_piaSession->m_meshLayerController);
    if (meshLayerController != 0)
    {
        meshLayerController->ClearSessionId(gatheringId);
    }
    m_gatheringId = gatheringId;
    return result;
}

void NexMatchmakeSession::UnregisterSearchCriteria()
{
    m_searchCriteriaList->clear();
    m_resultRangeList->clear();
}

Result NexMatchmakeSession::UpdateApplicationBuffer(nn::pia::common::CallContext* callContext, unsigned int gatheringId,
                                                    unsigned char* buffer, unsigned int size)
{
    if (callContext == 0)
    {
        return cResultInvalidArgument;
    }
    if (callContext->m_state == common::CallContext::cStateInProgress)
    {
        return cResultInvalidArgument;
    }
    if (m_callContext != 0)
    {
        return transport::cResultInvalidState;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallInProgress)
    {
        return transport::cResultInvalidState;
    }
    m_callContext = callContext;
    nn::nex::qVector<unsigned char> applicationBuffer;
    applicationBuffer.reserve(size);
    for (unsigned int i = 0; i < size; ++i)
    {
        applicationBuffer.push_back(buffer[i]);
    }
    if (!m_matchmakeExtensionClient->UpdateApplicationBuffer(&m_protocolCallContext, gatheringId, applicationBuffer))
    {
        m_callContext = 0;
        return cResultRequestFailed;
    }
    m_callContext->InitiateCall();
    return 0;
}

Result NexMatchmakeSession::UpdateAttributes(nn::pia::common::CallContext* callContext, unsigned int gatheringId,
                                             unsigned int* attributes)
{
    if (callContext == 0)
    {
        return ResultInvalidArgument();
    }
    if (callContext->m_state == common::CallContext::cStateInProgress)
    {
        return cResultInvalidArgument;
    }
    if (m_callContext != 0)
    {
        return transport::cResultInvalidState;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallInProgress)
    {
        return transport::cResultInvalidState;
    }
    m_callContext = callContext;
    if (!m_matchmakeExtensionClient->UpdateMatchmakeSessionAttribute(&m_protocolCallContext, gatheringId, attributes))
    {
        m_callContext = 0;
        return cResultRequestFailed;
    }
    m_callContext->InitiateCall();
    return 0;
}

Result NexMatchmakeSession::UpdateCommunity(nn::pia::common::CallContext* callContext)
{
    if (callContext == 0)
    {
        return cResultInvalidArgument;
    }
    if (callContext->m_state == common::CallContext::cStateInProgress)
    {
        return cResultInvalidArgument;
    }
    if (m_callContext != 0)
    {
        return transport::cResultInvalidState;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallInProgress)
    {
        return transport::cResultInvalidState;
    }
    m_callContext = callContext;
    if (!m_matchmakeExtensionClient->UpdatePersistentGathering(&m_protocolCallContext, *m_persistentGathering))
    {
        m_callContext = 0;
        return cResultRequestFailed;
    }
    m_callContext->InitiateCall();
    return 0;
}

Result NexMatchmakeSession::UpdateMatchmakeSessionHost(unsigned int stationId)
{
    if (stationId == 0)
    {
        return ResultInvalidArgument();
    }
    if (m_protocolCallContextB.GetState() == nn::nex::CallContext::CallInProgress)
    {
        m_protocolCallContextB.Cancel(nn::nex::CallContext::CallCanceled);
    }
    if (m_protocolCallContextB.GetState() != nn::nex::CallContext::Ready)
    {
        m_protocolCallContextB.Reset();
    }
    if (!m_matchmakeExtensionClient->UpdateSessionHost(&m_protocolCallContextB, stationId, true))
    {
        return ResultInvalidState();
    }
    m_matchmakeHost = true;
    return 0;
}

Result NexMatchmakeSession::UpdateSelectionPriority(unsigned int gatheringId, unsigned char priority)
{
    if (priority > cSelectionPriorityMax)
    {
        return cResultInvalidArgument;
    }
    if (gatheringId == 0)
    {
        return cResultInvalidArgument;
    }
    session::MeshLayerController* meshLayerController = session::g_piaSession->m_meshLayerController;
    if (meshLayerController->CheckConnectedStatus() != 0)
    {
        return meshLayerController->CheckConnectedStatus();
    }
    if (!IsUpdatableSelectionPriority(priority))
    {
        return cResultSelectionPriorityUpdateTooFrequent;
    }
    m_selectionPriorityUpdateTime.SetNow();
    if (m_matchmakeExtensionClient->UpdateProgressScore(gatheringId, priority))
    {
        common::SessionStateMonitoringContent* content = &common::g_sessionStateMonitoringContent;
        content->m_metric223[2] = content->m_metric223[2] != cMonitoringMetricUnset ? content->m_metric223[2] + 1 : 1;
        return 0;
    }
    return cResultRequestFailed;
}

Result NexMatchmakeSession::UpdateSessionSetting(nn::pia::common::CallContext* callContext, unsigned int gatheringId)
{
    if (callContext == 0)
    {
        return ResultInvalidArgument();
    }
    if (callContext->m_state == common::CallContext::cStateInProgress)
    {
        return ResultInvalidArgument();
    }
    if (m_callContext != 0)
    {
        return transport::cResultInvalidState;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallInProgress)
    {
        return transport::cResultInvalidState;
    }
    m_callContext = callContext;
    m_updateMatchmakeSessionParam->m_uiValue1 = gatheringId;
    if (!m_matchmakeExtensionClient->UpdateMatchmakeSession(&m_protocolCallContext, *m_updateMatchmakeSessionParam))
    {
        m_callContext = 0;
        return cResultRequestFailed;
    }
    m_callContext->InitiateCall();
    return 0;
}

bool NexMatchmakeSession::WaitBrowseMatchmakeSession()
{
    nn::nex::CallContext::State state = m_protocolCallContext.GetState();
    if (state != nn::nex::CallContext::CallInProgress)
    {
        if (state == nn::nex::CallContext::CallFailed)
        {
            nn::nex::qResult result = m_protocolCallContext.GetResult();
            if (result.Equals(cNexResultInvalidArgument))
            {
                m_callContext->SignalFailure(cResultInvalidArgument);
                m_callContext = 0;
                m_gatheringList->clear();
                UnregisterSearchCriteria();
                m_protocolCallContext.Reset();
                return true;
            }
            if (result.Equals(cNexResultGameServerMaintenance))
            {
                m_callContext->SignalFailure(cResultServerMaintenance);
                m_callContext = 0;
                m_gatheringList->clear();
                UnregisterSearchCriteria();
                m_protocolCallContext.Reset();
                return true;
            }
            Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
            m_callContext->SignalFailure(cResultNexError);
            m_callContext = 0;
            m_gatheringList->clear();
            UnregisterSearchCriteria();
            m_protocolCallContext.Reset();
            return true;
        }
        if (state == nn::nex::CallContext::CallSucceeded)
        {
            RegisterFindSessionResult(session::SessionSearchCriteria::cKeyTypeDefault);
            m_callContext->SignalSuccess(0);
            m_callContext = 0;
            m_gatheringList->clear();
            UnregisterSearchCriteria();
            m_protocolCallContext.Reset();
            return true;
        }
    }
    return false;
}

bool NexMatchmakeSession::WaitClearSystemPassword()
{
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallFailed)
    {
        nn::nex::qResult result = m_protocolCallContext.GetResult();
        if (result.Equals(cNexResultSessionVoid))
        {
            m_callContext->SignalFailure(cResultSessionVoid);
            m_callContext = 0;
            m_protocolCallContext.Reset();
            return true;
        }
        if (result.Equals(cNexResultPermissionDenied))
        {
            m_callContext->SignalFailure(transport::cResultInvalidState);
            m_callContext = 0;
            m_protocolCallContext.Reset();
            return true;
        }
        Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
        m_callContext->SignalFailure(cResultNexError);
        m_callContext = 0;
        m_protocolCallContext.Reset();
        return true;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallSucceeded)
    {
        m_callContext->SignalSuccess(0);
        m_callContext = 0;
        m_protocolCallContext.Reset();
        return true;
    }
    return false;
}

bool NexMatchmakeSession::WaitCloseParticipate()
{
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallFailed)
    {
        nn::nex::qResult result = m_protocolCallContext.GetResult();
        if (result.Equals(cNexResultSessionVoid))
        {
            m_callContext->SignalFailure(cResultSessionVoid);
            m_callContext = 0;
            m_protocolCallContext.Reset();
            return true;
        }
        if (result.Equals(cNexResultPermissionDenied))
        {
            m_callContext->SignalFailure(transport::cResultInvalidState);
            m_callContext = 0;
            m_protocolCallContext.Reset();
            return true;
        }
        Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
        m_callContext->SignalFailure(cResultNexError);
        m_callContext = 0;
        m_protocolCallContext.Reset();
        return true;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallSucceeded)
    {
        m_callContext->SignalSuccess(0);
        m_callContext = 0;
        m_protocolCallContext.Reset();
        common::g_sessionStateMonitoringContent.m_metric223[1] = false;
        return true;
    }
    return false;
}

bool NexMatchmakeSession::WaitCreateCommunity()
{
    nn::nex::CallContext::State state = m_protocolCallContext.GetState();
    if (state != nn::nex::CallContext::CallInProgress)
    {
        if (state == nn::nex::CallContext::CallFailed)
        {
            nn::nex::qResult result = m_protocolCallContext.GetResult();
            if (result.Equals(cNexResultInvalidArgument))
            {
                m_callContext->SignalFailure(cResultInvalidArgument);
                m_persistentGathering->Reset();
                m_protocolCallContext.Reset();
                m_callContext = 0;
                m_gatheringId = 0;
                return true;
            }
            if (result.Equals(cNexResultPersistentGatheringCreationMax))
            {
                m_callContext->SignalFailure(cResultCommunityCreationLimit);
                m_persistentGathering->Reset();
                m_protocolCallContext.Reset();
                m_callContext = 0;
                m_gatheringId = 0;
                return true;
            }
            if (result.Equals(cNexResultPersistentGatheringParticipationMax))
            {
                m_callContext->SignalFailure(cResultCommunityParticipationLimit);
                m_persistentGathering->Reset();
                m_protocolCallContext.Reset();
                m_callContext = 0;
                m_gatheringId = 0;
                return true;
            }
            if (result.Equals(cNexResultGameServerMaintenance))
            {
                m_callContext->SignalFailure(cResultServerMaintenance);
                m_persistentGathering->Reset();
                m_protocolCallContext.Reset();
                m_callContext = 0;
                m_gatheringId = 0;
                return true;
            }
            Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
            m_callContext->SignalFailure(cResultNexError);
            m_persistentGathering->Reset();
            m_protocolCallContext.Reset();
            m_callContext = 0;
            m_gatheringId = 0;
            return true;
        }
        if (state == nn::nex::CallContext::CallSucceeded)
        {
            m_callContext->SignalSuccess(0);
            m_callContext = 0;
            m_persistentGathering->Reset();
            m_protocolCallContext.Reset();
            m_gatheringId = 0;
            return true;
        }
    }
    return false;
}

bool NexMatchmakeSession::WaitCreateMatchmakeSession(unsigned int* gatheringId)
{
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallFailed)
    {
        nn::nex::qResult result = m_protocolCallContext.GetResult();
        if (result.Equals(cNexResultInvalidArgument))
        {
            m_callContext->SignalFailure(cResultInvalidArgument);
            m_callContext = 0;
            m_idList7c->clear();
            m_createMatchmakeSessionParam->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            m_foundSession->Reset();
            m_protocolCallContext.Reset();
            return true;
        }
        if (result.Equals(cNexResultGameServerMaintenance))
        {
            m_callContext->SignalFailure(cResultServerMaintenance);
            m_callContext = 0;
            m_idList7c->clear();
            m_createMatchmakeSessionParam->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            m_foundSession->Reset();
            m_protocolCallContext.Reset();
            return true;
        }
        if (result.Equals(cNexResultUserIsOffline))
        {
            m_callContext->SignalFailure(cResultOwnerOffline);
            m_callContext = 0;
            m_idList7c->clear();
            m_createMatchmakeSessionParam->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            m_foundSession->Reset();
            m_protocolCallContext.Reset();
            return true;
        }
        if (result.Equals(cNexResultNotParticipatedGathering))
        {
            m_callContext->SignalFailure(cResultOwnerOffline);
            m_callContext = 0;
            m_idList7c->clear();
            m_createMatchmakeSessionParam->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            m_foundSession->Reset();
            m_protocolCallContext.Reset();
            return true;
        }
        Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
        m_callContext->SignalFailure(cResultNexError);
        m_callContext = 0;
        m_idList7c->clear();
        m_createMatchmakeSessionParam->Reset();
        m_matchmakeParam->m_Parameters.clear();
        m_requestSession->Reset();
        m_foundSession->Reset();
        m_protocolCallContext.Reset();
        return true;
    }
    if (m_protocolCallContext.GetState() != nn::nex::CallContext::CallSucceeded)
    {
        return false;
    }
    Result keyResult = SetMatchmakeSessionKey(m_foundSession->m_SessionKey);
    if (keyResult == 0)
    {
        *gatheringId = m_foundSession->m_uiValue1;
        m_matchmakeHost = m_foundSession->m_uiValue3 == NexFacade::GetInstance()->GetLocalPrincipalId();
        m_matchmakeOwner = m_foundSession->m_uiValue2 == NexFacade::GetInstance()->GetLocalPrincipalId();
        if (m_matchmakeHost && m_matchmakeOwner)
        {
            m_sessionInfo.Clear();
            CopyMatchmakeSessionToSessionInfo(m_foundSession, &m_sessionInfo);
            m_matchmakeOwnerUniqueId = m_foundSession->m_uiValue2;
            SessionParticipantInfo* participantInfo = const_cast<SessionParticipantInfo*>(
                static_cast<NexMatchMeshLayerController*>(session::g_piaSession->m_meshLayerController)
                    ->GetSessionParticipantInfo(m_foundSession->m_uiValue1));
            if (participantInfo != 0)
            {
                participantInfo->RemoveObsoleteEntry();
            }
            m_callContext->SignalSuccess(0);
            m_callContext = 0;
            m_idList7c->clear();
            m_createMatchmakeSessionParam->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            m_foundSession->Reset();
            m_protocolCallContext.Reset();
            return true;
        }
        m_callContext->SignalFailure(cResultSessionInconsistent);
        m_callContext = 0;
        m_idList7c->clear();
        m_createMatchmakeSessionParam->Reset();
        m_matchmakeParam->m_Parameters.clear();
        m_requestSession->Reset();
        m_foundSession->Reset();
        m_protocolCallContext.Reset();
        return true;
    }
    m_callContext->SignalFailure(keyResult);
    m_callContext = 0;
    m_idList7c->clear();
    m_createMatchmakeSessionParam->Reset();
    m_matchmakeParam->m_Parameters.clear();
    m_requestSession->Reset();
    m_foundSession->Reset();
    m_protocolCallContext.Reset();
    return true;
}

bool NexMatchmakeSession::WaitCreateMatchmakeSessionWithTeam(unsigned int* gatheringId)
{
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallFailed)
    {
        nn::nex::qResult result = m_protocolCallContext.GetResult();
        if (result.Equals(cNexResultNotParticipatedGathering))
        {
            m_callContext->SignalFailure(cResultOwnerOffline);
            m_callContext = 0;
            m_idList7c->clear();
            m_createMatchmakeSessionParam->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            m_foundSession->Reset();
            m_protocolCallContext.Reset();
            return true;
        }
        if (result.Equals(cNexResultInvalidArgument))
        {
            m_callContext->SignalFailure(cResultInvalidArgument);
            m_callContext = 0;
            m_idList7c->clear();
            m_createMatchmakeSessionParam->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            m_foundSession->Reset();
            m_protocolCallContext.Reset();
            return true;
        }
        if (result.Equals(cNexResultGameServerMaintenance))
        {
            m_callContext->SignalFailure(cResultServerMaintenance);
            m_callContext = 0;
            m_idList7c->clear();
            m_createMatchmakeSessionParam->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            m_foundSession->Reset();
            m_protocolCallContext.Reset();
            return true;
        }
        if (result.Equals(cNexResultUserIsOffline))
        {
            m_callContext->SignalFailure(cResultOwnerOffline);
            m_callContext = 0;
            m_idList7c->clear();
            m_createMatchmakeSessionParam->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            m_foundSession->Reset();
            m_protocolCallContext.Reset();
            return true;
        }
        Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
        m_callContext->SignalFailure(cResultNexError);
        m_callContext = 0;
        m_idList7c->clear();
        m_createMatchmakeSessionParam->Reset();
        m_matchmakeParam->m_Parameters.clear();
        m_requestSession->Reset();
        m_foundSession->Reset();
        m_protocolCallContext.Reset();
        return true;
    }
    if (m_protocolCallContext.GetState() != nn::nex::CallContext::CallSucceeded)
    {
        return false;
    }
    Result keyResult = SetMatchmakeSessionKey(m_foundSession->m_SessionKey);
    if (keyResult == 0)
    {
        m_matchmakeHost = m_foundSession->m_uiValue3 == NexFacade::GetInstance()->GetLocalPrincipalId();
        m_matchmakeOwner = m_foundSession->m_uiValue2 == NexFacade::GetInstance()->GetLocalPrincipalId();
        if (m_matchmakeHost && m_matchmakeOwner)
        {
            m_sessionInfo.Clear();
            m_matchmakeOwnerUniqueId = m_foundSession->m_uiValue2;
            *gatheringId = m_foundSession->m_uiValue1;
            SessionParticipantInfo* participantInfo = const_cast<SessionParticipantInfo*>(
                static_cast<NexMatchMeshLayerController*>(session::g_piaSession->m_meshLayerController)
                    ->GetSessionParticipantInfo(m_foundSession->m_uiValue1));
            if (participantInfo != 0)
            {
                participantInfo->RemoveObsoleteEntry();
            }
            m_callContext->SignalSuccess(0);
            m_callContext = 0;
            m_idList7c->clear();
            m_createMatchmakeSessionParam->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            m_foundSession->Reset();
            m_protocolCallContext.Reset();
            return true;
        }
        m_callContext->SignalFailure(cResultSessionInconsistent);
        m_callContext = 0;
        m_idList7c->clear();
        m_createMatchmakeSessionParam->Reset();
        m_matchmakeParam->m_Parameters.clear();
        m_requestSession->Reset();
        m_foundSession->Reset();
        m_protocolCallContext.Reset();
        return true;
    }
    m_callContext->SignalFailure(transport::cResultInvalidState);
    m_callContext = 0;
    m_idList7c->clear();
    m_createMatchmakeSessionParam->Reset();
    m_matchmakeParam->m_Parameters.clear();
    m_requestSession->Reset();
    m_foundSession->Reset();
    m_protocolCallContext.Reset();
    return true;
}

bool NexMatchmakeSession::WaitFindCommunityByGatheringId()
{
    nn::nex::CallContext::State state = m_protocolCallContext.GetState();
    if (state != nn::nex::CallContext::CallInProgress)
    {
        if (state == nn::nex::CallContext::CallFailed)
        {
            nn::nex::qResult result = m_protocolCallContext.GetResult();
            if (result.Equals(cNexResultInvalidArgument))
            {
                m_callContext->SignalFailure(cResultInvalidArgument);
                m_persistentGatheringList->clear();
                m_idList68->clear();
                m_protocolCallContext.Reset();
                m_callContext = 0;
                return true;
            }
            if (result.Equals(cNexResultGameServerMaintenance))
            {
                m_callContext->SignalFailure(cResultServerMaintenance);
                m_persistentGatheringList->clear();
                m_idList68->clear();
                m_protocolCallContext.Reset();
                m_callContext = 0;
                return true;
            }
            Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
            m_callContext->SignalFailure(cResultNexError);
            m_persistentGatheringList->clear();
            m_idList68->clear();
            m_protocolCallContext.Reset();
            m_callContext = 0;
            return true;
        }
        if (state == nn::nex::CallContext::CallSucceeded)
        {
            RegisterFindCommunityResult();
            m_callContext->SignalSuccess(0);
            m_callContext = 0;
            m_persistentGatheringList->clear();
            m_idList68->clear();
            m_protocolCallContext.Reset();
            return true;
        }
    }
    return false;
}

bool NexMatchmakeSession::WaitFindCommunityByOwner()
{
    nn::nex::CallContext::State state = m_protocolCallContext.GetState();
    if (state != nn::nex::CallContext::CallInProgress)
    {
        if (state == nn::nex::CallContext::CallFailed)
        {
            nn::nex::qResult result = m_protocolCallContext.GetResult();
            if (result.GetReturnCode() == static_cast<unsigned int>(cNexResultInvalidArgument))
            {
                m_callContext->SignalFailure(cResultInvalidArgument);
                m_callContext = 0;
                m_gatheringList->clear();
                m_protocolCallContext.Reset();
                return true;
            }
            if (result.GetReturnCode() == static_cast<unsigned int>(cNexResultGameServerMaintenance))
            {
                m_callContext->SignalFailure(cResultServerMaintenance);
                m_callContext = 0;
                m_gatheringList->clear();
                m_protocolCallContext.Reset();
                return true;
            }
            Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
            m_callContext->SignalFailure(cResultNexError);
            m_callContext = 0;
            m_gatheringList->clear();
            m_protocolCallContext.Reset();
            return true;
        }
        if (state == nn::nex::CallContext::CallSucceeded)
        {
            for (nn::nex::qList<nn::nex::AnyObjectHolder<nn::nex::Gathering, nn::nex::String> >::const_iterator it =
                     m_gatheringList->begin();
                 it != m_gatheringList->end(); ++it)
            {
                if (it->GetObject()->IsA(nn::nex::String("PersistentGathering")) == true)
                {
                    NexCommunityInfo* communityInfo =
                        static_cast<NexCommunityInfoList*>(m_browsedCommunityInfoList)->AddCommunityInfo();
                    if (communityInfo == 0)
                    {
                        break;
                    }
                    CopyFoundPersistentGatheringToBrowsedCommunityInfoList(
                        static_cast<const nn::nex::PersistentGathering*>(it->GetObject()), communityInfo);
                }
            }
            m_callContext->SignalSuccess(0);
            m_callContext = 0;
            m_gatheringList->clear();
            m_protocolCallContext.Reset();
            return true;
        }
    }
    return false;
}

bool NexMatchmakeSession::WaitFindCommunityByParticipant()
{
    nn::nex::CallContext::State state = m_protocolCallContext.GetState();
    if (state != nn::nex::CallContext::CallInProgress)
    {
        if (state == nn::nex::CallContext::CallFailed)
        {
            nn::nex::qResult result = m_protocolCallContext.GetResult();
            if (result.Equals(cNexResultInvalidArgument))
            {
                m_callContext->SignalFailure(cResultInvalidArgument);
                m_callContext = 0;
                m_persistentGatheringList->clear();
                m_protocolCallContext.Reset();
                return true;
            }
            if (result.Equals(cNexResultGameServerMaintenance))
            {
                m_callContext->SignalFailure(cResultServerMaintenance);
                m_callContext = 0;
                m_persistentGatheringList->clear();
                m_protocolCallContext.Reset();
                return true;
            }
            Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
            m_callContext->SignalFailure(cResultNexError);
            m_callContext = 0;
            m_persistentGatheringList->clear();
            m_protocolCallContext.Reset();
            return true;
        }
        if (state == nn::nex::CallContext::CallSucceeded)
        {
            RegisterFindCommunityResult();
            m_callContext->SignalSuccess(0);
            m_callContext = 0;
            m_persistentGatheringList->clear();
            m_protocolCallContext.Reset();
            return true;
        }
    }
    return false;
}

bool NexMatchmakeSession::WaitFindOfficialCommunity()
{
    nn::nex::CallContext::State state = m_protocolCallContext.GetState();
    if (state != nn::nex::CallContext::CallInProgress)
    {
        if (state == nn::nex::CallContext::CallFailed)
        {
            nn::nex::qResult result = m_protocolCallContext.GetResult();
            if (result.Equals(cNexResultInvalidArgument))
            {
                m_callContext->SignalFailure(cResultInvalidArgument);
                m_callContext = 0;
                m_persistentGatheringList->clear();
                m_protocolCallContext.Reset();
                return true;
            }
            if (result.Equals(cNexResultGameServerMaintenance))
            {
                m_callContext->SignalFailure(cResultServerMaintenance);
                m_callContext = 0;
                m_persistentGatheringList->clear();
                m_protocolCallContext.Reset();
                return true;
            }
            Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
            m_callContext->SignalFailure(cResultNexError);
            m_callContext = 0;
            m_persistentGatheringList->clear();
            m_protocolCallContext.Reset();
            return true;
        }
        if (state == nn::nex::CallContext::CallSucceeded)
        {
            RegisterFindCommunityResult();
            m_callContext->SignalSuccess(0);
            m_callContext = 0;
            m_persistentGatheringList->clear();
            m_protocolCallContext.Reset();
            return true;
        }
    }
    return false;
}

bool NexMatchmakeSession::WaitFindSessionByOwner()
{
    nn::nex::CallContext::State state = m_protocolCallContext.GetState();
    if (state != nn::nex::CallContext::CallInProgress)
    {
        if (state == nn::nex::CallContext::CallFailed)
        {
            nn::nex::qResult result = m_protocolCallContext.GetResult();
            if (result.Equals(cNexResultInvalidArgument))
            {
                m_callContext->SignalFailure(cResultInvalidArgument);
                m_callContext = 0;
                m_gatheringList->clear();
                m_protocolCallContext.Reset();
                return true;
            }
            if (result.Equals(cNexResultGameServerMaintenance))
            {
                m_callContext->SignalFailure(cResultServerMaintenance);
                m_callContext = 0;
                m_gatheringList->clear();
                m_protocolCallContext.Reset();
                return true;
            }
            Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
            m_callContext->SignalFailure(cResultNexError);
            m_callContext = 0;
            m_gatheringList->clear();
            m_protocolCallContext.Reset();
            return true;
        }
        if (state == nn::nex::CallContext::CallSucceeded)
        {
            RegisterFindSessionResult(session::SessionSearchCriteria::cKeyTypeOwner);
            m_callContext->SignalSuccess(0);
            m_callContext = 0;
            m_gatheringList->clear();
            m_protocolCallContext.Reset();
            return true;
        }
    }
    return false;
}

bool NexMatchmakeSession::WaitFindSessionByParticipant()
{
    nn::nex::CallContext::State state = m_protocolCallContext.GetState();
    if (state != nn::nex::CallContext::CallInProgress)
    {
        if (state == nn::nex::CallContext::CallFailed)
        {
            nn::nex::qResult result = m_protocolCallContext.GetResult();
            if (result.Equals(cNexResultInvalidArgument))
            {
                m_callContext->SignalFailure(cResultInvalidArgument);
                m_callContext = 0;
                m_playingSessionList->clear();
                m_protocolCallContext.Reset();
                return true;
            }
            if (result.Equals(cNexResultGameServerMaintenance))
            {
                m_callContext->SignalFailure(cResultServerMaintenance);
                m_callContext = 0;
                m_playingSessionList->clear();
                m_protocolCallContext.Reset();
                return true;
            }
            Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
            m_callContext->SignalFailure(cResultNexError);
            m_callContext = 0;
            m_playingSessionList->clear();
            m_protocolCallContext.Reset();
            return true;
        }
        if (state == nn::nex::CallContext::CallSucceeded)
        {
            RegisterFindSessionResult(session::SessionSearchCriteria::cKeyTypeParticipant);
            m_callContext->SignalSuccess(0);
            m_callContext = 0;
            m_playingSessionList->clear();
            m_protocolCallContext.Reset();
            return true;
        }
    }
    return false;
}

bool NexMatchmakeSession::WaitFindSessionBySessionId()
{
    nn::nex::CallContext::State state = m_protocolCallContext.GetState();
    if (state != nn::nex::CallContext::CallInProgress)
    {
        if (state == nn::nex::CallContext::CallFailed)
        {
            nn::nex::qResult result = m_protocolCallContext.GetResult();
            if (result.GetReturnCode() == static_cast<unsigned int>(cNexResultSessionVoid))
            {
                m_callContext->SignalFailure(cResultSessionVoid);
                m_callContext = 0;
                m_foundSession->Reset();
                m_protocolCallContext.Reset();
                m_gatheringHolder.Release();
                return true;
            }
            if (result.GetReturnCode() == static_cast<unsigned int>(cNexResultGameServerMaintenance))
            {
                m_callContext->SignalFailure(cResultServerMaintenance);
                m_callContext = 0;
                m_foundSession->Reset();
                m_protocolCallContext.Reset();
                m_gatheringHolder.Release();
                return true;
            }
            Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
            m_callContext->SignalFailure(cResultNexError);
            m_callContext = 0;
            m_foundSession->Reset();
            m_protocolCallContext.Reset();
            m_gatheringHolder.Release();
            return true;
        }
        if (state == nn::nex::CallContext::CallSucceeded)
        {
            m_foundSession = static_cast<nn::nex::MatchmakeSession*>(m_gatheringHolder.GetObject());
            session::SessionInfoList<NexSessionInfoPrivate>* sessionInfoList =
                static_cast<session::SessionInfoList<NexSessionInfoPrivate>*>(m_browseMatchmakeSessionResult);
            session::ISessionInfo* sessionInfo = 0;
            if (sessionInfoList->m_maxSize != 0 && sessionInfoList->m_size < sessionInfoList->m_maxSize)
            {
                sessionInfo = sessionInfoList->m_items[sessionInfoList->m_size];
                sessionInfo->Clear();
                ++sessionInfoList->m_size;
            }
            if (sessionInfo != 0)
            {
                NexSessionInfo* nexSessionInfo = static_cast<NexSessionInfo*>(sessionInfo);
                CopyMatchmakeSessionToSessionInfo(m_foundSession, nexSessionInfo);
                nexSessionInfo->SetTargetPrincipalId(0);
                nexSessionInfo->Trace(cTraceSessionInfo);
            }
            m_callContext->SignalSuccess(0);
            m_callContext = 0;
            m_foundSession->Reset();
            m_protocolCallContext.Reset();
            m_gatheringHolder.Release();
            return true;
        }
        if (m_requestDeadline < common::g_piaJobScheduler->m_dispatchTime)
        {
            m_protocolCallContext.Cancel(nn::nex::CallContext::CallCanceled);
            m_callContext->SignalFailure(cResultRequestFailed);
            m_callContext = 0;
            m_foundSession->Reset();
            m_protocolCallContext.Reset();
            m_gatheringHolder.Release();
            return true;
        }
    }
    return false;
}

bool NexMatchmakeSession::WaitGenerateSystemPassword()
{
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallFailed)
    {
        nn::nex::qResult result = m_protocolCallContext.GetResult();
        if (result.Equals(cNexResultSessionVoid))
        {
            m_callContext->SignalFailure(cResultSessionVoid);
            m_callContext = 0;
            m_protocolCallContext.Reset();
            return true;
        }
        if (result.Equals(cNexResultPermissionDenied))
        {
            m_callContext->SignalFailure(transport::cResultInvalidState);
            m_callContext = 0;
            m_protocolCallContext.Reset();
            return true;
        }
        Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
        m_callContext->SignalFailure(cResultNexError);
        m_callContext = 0;
        m_protocolCallContext.Reset();
        return true;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallSucceeded)
    {
        m_callContext->SignalSuccess(0);
        m_callContext = 0;
        m_protocolCallContext.Reset();
        return true;
    }
    return false;
}

bool NexMatchmakeSession::WaitGetStationConnectionInfo(nn::pia::transport::StationConnectionInfo* info)
{
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallFailed)
    {
        nn::nex::qResult result = m_protocolCallContext.GetResult();
        if (result.Equals(cNexResultSessionVoid))
        {
            m_callContext->SignalFailure(cResultSessionVoid);
            m_callContext = 0;
            m_stationUrlList->clear();
            m_protocolCallContext.Reset();
            return true;
        }
        if (result.Equals(cNexResultPermissionDenied))
        {
            m_callContext->SignalFailure(transport::cResultInvalidState);
            m_callContext = 0;
            m_stationUrlList->clear();
            m_protocolCallContext.Reset();
            return true;
        }
        Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
        m_callContext->SignalFailure(cResultNexError);
        m_callContext = 0;
        m_stationUrlList->clear();
        m_protocolCallContext.Reset();
        return true;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallSucceeded)
    {
        Result result = NexFacade::ConvertNexStationUrlToStationConnectionInfo(*m_stationUrlList, info);
        m_stationUrlList->clear();
        if (result == 0)
        {
            m_callContext->SignalSuccess(0);
            m_callContext = 0;
            m_protocolCallContext.Reset();
            return true;
        }
        m_callContext->SignalFailure(result);
        m_callContext = 0;
        m_protocolCallContext.Reset();
        return true;
    }
    return false;
}

bool NexMatchmakeSession::WaitJoinCommunity()
{
    nn::nex::CallContext::State state = m_protocolCallContext.GetState();
    if (state != nn::nex::CallContext::CallInProgress)
    {
        if (state == nn::nex::CallContext::CallFailed)
        {
            nn::nex::qResult result = m_protocolCallContext.GetResult();
            if (result.Equals(cNexResultInvalidArgument))
            {
                m_callContext->SignalFailure(cResultInvalidArgument);
                m_callContext = 0;
                m_persistentGathering->Reset();
                m_protocolCallContext.Reset();
                return true;
            }
            if (result.Equals(cNexResultSessionFull))
            {
                m_callContext->SignalFailure(cResultCommunityFull);
                m_callContext = 0;
                m_persistentGathering->Reset();
                m_protocolCallContext.Reset();
                return true;
            }
            if (result.Equals(cNexResultSessionVoid))
            {
                m_callContext->SignalFailure(cResultCommunityVoid);
                m_callContext = 0;
                m_persistentGathering->Reset();
                m_protocolCallContext.Reset();
                return true;
            }
            if (result.Equals(cNexResultGameServerMaintenance))
            {
                m_callContext->SignalFailure(cResultServerMaintenance);
                m_callContext = 0;
                m_persistentGathering->Reset();
                m_protocolCallContext.Reset();
                return true;
            }
            if (result.Equals(cNexResultPersistentGatheringParticipationMax))
            {
                m_callContext->SignalFailure(cResultCommunityParticipationLimit);
                m_callContext = 0;
                m_persistentGathering->Reset();
                m_protocolCallContext.Reset();
                return true;
            }
            if (result.Equals(cNexResultInvalidGatheringPassword))
            {
                m_callContext->SignalFailure(cResultCommunityPasswordMismatch);
                m_callContext = 0;
                m_persistentGathering->Reset();
                m_protocolCallContext.Reset();
                return true;
            }
            if (result.Equals(cNexResultWithoutParticipationPeriod))
            {
                m_callContext->SignalFailure(cResultCommunityOutOfParticipationPeriod);
                m_callContext = 0;
                m_persistentGathering->Reset();
                m_protocolCallContext.Reset();
                return true;
            }
            if (result.Equals(cNexResultAlreadyParticipatedGathering))
            {
                m_callContext->SignalFailure(cResultAlreadyParticipated);
                m_callContext = 0;
                m_persistentGathering->Reset();
                m_protocolCallContext.Reset();
                return true;
            }
            Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
            m_callContext->SignalFailure(cResultNexError);
            m_callContext = 0;
            m_persistentGathering->Reset();
            m_protocolCallContext.Reset();
            return true;
        }
        if (state == nn::nex::CallContext::CallSucceeded)
        {
            m_callContext->SignalSuccess(0);
            m_callContext = 0;
            m_persistentGathering->Reset();
            m_protocolCallContext.Reset();
            return true;
        }
    }
    return false;
}

bool NexMatchmakeSession::WaitJoinMatchmakeSession()
{
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallFailed)
    {
        nn::nex::qResult result = m_protocolCallContext.GetResult();
        if (result.Equals(cNexResultInvalidArgument))
        {
            m_callContext->SignalFailure(cResultInvalidArgument);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_joinMatchmakeSessionParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeGatheringId = 0;
            m_temporaryOwnerChangeOwnerId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultSessionFull))
        {
            m_callContext->SignalFailure(cResultSessionFull);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_joinMatchmakeSessionParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeGatheringId = 0;
            m_temporaryOwnerChangeOwnerId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultSessionVoid))
        {
            m_callContext->SignalFailure(cResultSessionVoid);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_joinMatchmakeSessionParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeGatheringId = 0;
            m_temporaryOwnerChangeOwnerId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultDeniedByParticipants))
        {
            m_callContext->SignalFailure(cResultDeniedByParticipants);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_joinMatchmakeSessionParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeGatheringId = 0;
            m_temporaryOwnerChangeOwnerId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultParticipantInBlackList))
        {
            m_callContext->SignalFailure(cResultParticipantInBlackList);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_joinMatchmakeSessionParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeGatheringId = 0;
            m_temporaryOwnerChangeOwnerId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultGameServerMaintenance))
        {
            m_callContext->SignalFailure(cResultServerMaintenance);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_joinMatchmakeSessionParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeGatheringId = 0;
            m_temporaryOwnerChangeOwnerId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultNotParticipatedGathering))
        {
            m_callContext->SignalFailure(transport::cResultInvalidState);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_joinMatchmakeSessionParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeGatheringId = 0;
            m_temporaryOwnerChangeOwnerId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultUserPasswordUnmatch))
        {
            m_callContext->SignalFailure(cResultUserPasswordMismatch);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_joinMatchmakeSessionParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeGatheringId = 0;
            m_temporaryOwnerChangeOwnerId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultSystemPasswordUnmatch))
        {
            m_callContext->SignalFailure(cResultSystemPasswordMismatch);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_joinMatchmakeSessionParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeGatheringId = 0;
            m_temporaryOwnerChangeOwnerId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultUserIsOffline))
        {
            m_callContext->SignalFailure(cResultOwnerOffline);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_joinMatchmakeSessionParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeGatheringId = 0;
            m_temporaryOwnerChangeOwnerId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultAlreadyParticipatedGathering))
        {
            m_callContext->SignalFailure(transport::cResultInvalidState);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_joinMatchmakeSessionParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeGatheringId = 0;
            m_temporaryOwnerChangeOwnerId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultNotFriend))
        {
            m_callContext->SignalFailure(cResultNotFriend);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_joinMatchmakeSessionParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeGatheringId = 0;
            m_temporaryOwnerChangeOwnerId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultSessionClosed))
        {
            m_callContext->SignalFailure(cResultSessionClosed);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_joinMatchmakeSessionParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeGatheringId = 0;
            m_temporaryOwnerChangeOwnerId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
        m_callContext->SignalFailure(cResultNexError);
        m_callContext = 0;
        m_idList7c->clear();
        m_foundSession->Reset();
        m_joinMatchmakeSessionParam->Reset();
        m_protocolCallContext.Reset();
        m_temporaryOwnerChangeGatheringId = 0;
        m_temporaryOwnerChangeOwnerId = 0;
        m_processingJoinMatchmakeSession = false;
        return true;
    }
    if (m_protocolCallContext.GetState() != nn::nex::CallContext::CallSucceeded)
    {
        return false;
    }
    if (SetMatchmakeSessionKey(m_foundSession->m_SessionKey) == 0)
    {
        bool ownerChanged = false;
        unsigned int hostId = m_foundSession->m_uiValue3;
        unsigned int ownerId = m_foundSession->m_uiValue2;
        if (m_temporaryOwnerChangeGatheringId == m_foundSession->m_uiValue1)
        {
            if (m_temporaryOwnerChangeOwnerId != 0 && m_temporaryOwnerChangeOwnerId != ownerId)
            {
                ownerChanged = true;
            }
        }
        m_matchmakeHost = hostId == NexFacade::GetInstance()->GetLocalPrincipalId();
        m_matchmakeOwner = ownerId == NexFacade::GetInstance()->GetLocalPrincipalId();
        if (m_matchmakeHost || m_matchmakeOwner || ownerChanged)
        {
            m_callContext->SignalFailure(cResultSessionInconsistent);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_joinMatchmakeSessionParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeGatheringId = 0;
            m_temporaryOwnerChangeOwnerId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        m_matchmakeOwnerUniqueId = ownerId;
        m_sessionInfo.Clear();
        CopyMatchmakeSessionToSessionInfo(m_foundSession, &m_sessionInfo);
        SessionParticipantInfo* participantInfo = const_cast<SessionParticipantInfo*>(
            static_cast<NexMatchMeshLayerController*>(session::g_piaSession->m_meshLayerController)
                ->GetSessionParticipantInfo(m_foundSession->m_uiValue1));
        if (participantInfo != 0)
        {
            participantInfo->RemoveObsoleteEntry();
        }
        m_callContext->SignalSuccess(0);
    }
    else
    {
        m_callContext->SignalFailure(transport::cResultInvalidState);
    }
    m_callContext = 0;
    m_idList7c->clear();
    m_foundSession->Reset();
    m_joinMatchmakeSessionParam->Reset();
    m_protocolCallContext.Reset();
    m_temporaryOwnerChangeGatheringId = 0;
    m_temporaryOwnerChangeOwnerId = 0;
    m_processingJoinMatchmakeSession = false;
    return true;
}

bool NexMatchmakeSession::WaitJoinMatchmakeSessionWithTeam()
{
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallFailed)
    {
        nn::nex::qResult result = m_protocolCallContext.GetResult();
        if (result.Equals(cNexResultInvalidArgument))
        {
            m_callContext->SignalFailure(cResultInvalidArgument);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_joinMatchmakeSessionParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeGatheringId = 0;
            m_temporaryOwnerChangeOwnerId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultSessionFull))
        {
            m_callContext->SignalFailure(cResultSessionFull);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_joinMatchmakeSessionParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeGatheringId = 0;
            m_temporaryOwnerChangeOwnerId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultSessionVoid))
        {
            m_callContext->SignalFailure(cResultSessionVoid);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_joinMatchmakeSessionParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeGatheringId = 0;
            m_temporaryOwnerChangeOwnerId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultDeniedByParticipants))
        {
            m_callContext->SignalFailure(cResultDeniedByParticipants);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_joinMatchmakeSessionParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeGatheringId = 0;
            m_temporaryOwnerChangeOwnerId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultParticipantInBlackList))
        {
            m_callContext->SignalFailure(cResultParticipantInBlackList);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_joinMatchmakeSessionParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeGatheringId = 0;
            m_temporaryOwnerChangeOwnerId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultGameServerMaintenance))
        {
            m_callContext->SignalFailure(cResultServerMaintenance);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_joinMatchmakeSessionParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeGatheringId = 0;
            m_temporaryOwnerChangeOwnerId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultNotParticipatedGathering))
        {
            m_callContext->SignalFailure(cResultOwnerOffline);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_joinMatchmakeSessionParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeGatheringId = 0;
            m_temporaryOwnerChangeOwnerId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultUserPasswordUnmatch))
        {
            m_callContext->SignalFailure(cResultUserPasswordMismatch);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_joinMatchmakeSessionParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeGatheringId = 0;
            m_temporaryOwnerChangeOwnerId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultSystemPasswordUnmatch))
        {
            m_callContext->SignalFailure(cResultSystemPasswordMismatch);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_joinMatchmakeSessionParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeGatheringId = 0;
            m_temporaryOwnerChangeOwnerId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultUserIsOffline))
        {
            m_callContext->SignalFailure(cResultOwnerOffline);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_joinMatchmakeSessionParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeGatheringId = 0;
            m_temporaryOwnerChangeOwnerId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultAlreadyParticipatedGathering))
        {
            m_callContext->SignalFailure(transport::cResultInvalidState);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_joinMatchmakeSessionParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeGatheringId = 0;
            m_temporaryOwnerChangeOwnerId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultNotFriend))
        {
            m_callContext->SignalFailure(cResultNotFriend);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_joinMatchmakeSessionParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeGatheringId = 0;
            m_temporaryOwnerChangeOwnerId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultSessionClosed))
        {
            m_callContext->SignalFailure(cResultSessionClosed);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_joinMatchmakeSessionParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeGatheringId = 0;
            m_temporaryOwnerChangeOwnerId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
        m_callContext->SignalFailure(cResultNexError);
        m_callContext = 0;
        m_idList7c->clear();
        m_foundSession->Reset();
        m_joinMatchmakeSessionParam->Reset();
        m_protocolCallContext.Reset();
        m_temporaryOwnerChangeGatheringId = 0;
        m_temporaryOwnerChangeOwnerId = 0;
        m_processingJoinMatchmakeSession = false;
        return true;
    }
    if (m_protocolCallContext.GetState() != nn::nex::CallContext::CallSucceeded)
    {
        return false;
    }
    if (SetMatchmakeSessionKey(m_foundSession->m_SessionKey) == 0)
    {
        bool ownerChanged = false;
        unsigned int hostId = m_foundSession->m_uiValue3;
        unsigned int ownerId = m_foundSession->m_uiValue2;
        if (m_temporaryOwnerChangeGatheringId == m_foundSession->m_uiValue1)
        {
            if (m_temporaryOwnerChangeOwnerId != 0 && m_temporaryOwnerChangeOwnerId != ownerId)
            {
                ownerChanged = true;
            }
        }
        m_matchmakeHost = hostId == NexFacade::GetInstance()->GetLocalPrincipalId();
        m_matchmakeOwner = ownerId == NexFacade::GetInstance()->GetLocalPrincipalId();
        if (m_matchmakeHost || m_matchmakeOwner || ownerChanged)
        {
            m_callContext->SignalFailure(cResultSessionInconsistent);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_joinMatchmakeSessionParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeGatheringId = 0;
            m_temporaryOwnerChangeOwnerId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        m_matchmakeOwnerUniqueId = ownerId;
        m_sessionInfo.Clear();
        SessionParticipantInfo* participantInfo = const_cast<SessionParticipantInfo*>(
            static_cast<NexMatchMeshLayerController*>(session::g_piaSession->m_meshLayerController)
                ->GetSessionParticipantInfo(m_foundSession->m_uiValue1));
        if (participantInfo != 0)
        {
            participantInfo->RemoveObsoleteEntry();
        }
        m_callContext->SignalSuccess(0);
    }
    else
    {
        m_callContext->SignalFailure(transport::cResultInvalidState);
    }
    m_callContext = 0;
    m_idList7c->clear();
    m_foundSession->Reset();
    m_joinMatchmakeSessionParam->Reset();
    m_protocolCallContext.Reset();
    m_temporaryOwnerChangeGatheringId = 0;
    m_temporaryOwnerChangeOwnerId = 0;
    m_processingJoinMatchmakeSession = false;
    return true;
}

bool NexMatchmakeSession::WaitLeaveMatchmakeSession()
{
    nn::nex::CallContext::State state = m_protocolCallContext.GetState();
    if (state == nn::nex::CallContext::CallInProgress)
    {
        if (common::g_piaJobScheduler->m_dispatchTime <= m_requestDeadline)
        {
            return false;
        }
        m_protocolCallContext.Cancel(nn::nex::CallContext::CallCanceled);
        m_callContext->SignalFailure(cResultRequestFailed);
        m_protocolCallContext.Reset();
        m_foundSession->Reset();
        m_callContext = 0;
    }
    else if (state == nn::nex::CallContext::CallFailed)
    {
        nn::nex::qResult result = m_protocolCallContext.GetResult();
        if (result.Equals(cNexResultInvalidArgument))
        {
            m_callContext->SignalFailure(cResultInvalidArgument);
            m_protocolCallContext.Reset();
            m_foundSession->Reset();
            m_callContext = 0;
        }
        else if (result.Equals(cNexResultSessionVoid))
        {
            m_callContext->SignalFailure(cResultSessionVoid);
            m_protocolCallContext.Reset();
            m_foundSession->Reset();
            m_callContext = 0;
        }
        else if (result.Equals(cNexResultPermissionDenied))
        {
            m_callContext->SignalFailure(transport::cResultInvalidState);
            m_protocolCallContext.Reset();
            m_foundSession->Reset();
            m_callContext = 0;
        }
        else
        {
            Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
            m_callContext->SignalFailure(cResultNexError);
            m_protocolCallContext.Reset();
            m_foundSession->Reset();
            m_callContext = 0;
        }
    }
    else if (state == nn::nex::CallContext::CallSucceeded)
    {
        m_matchmakeHost = false;
        m_matchmakeOwnerUniqueId = 0;
        m_matchmakeOwner = false;
        m_callContext->SignalSuccess(0);
        m_protocolCallContext.Reset();
        m_foundSession->Reset();
        m_callContext = 0;
    }
    else
    {
        return false;
    }
    NexMatchMeshLayerController* meshLayerController = static_cast<NexMatchMeshLayerController*>(
        session::g_piaSession->m_meshLayerController);
    if (meshLayerController != 0)
    {
        meshLayerController->ClearSessionId(m_gatheringId);
    }
    m_gatheringId = 0;
    m_sessionInfo.Clear();
    return true;
}

bool NexMatchmakeSession::WaitLeaveMatchmakeSessionCompanion()
{
    nn::nex::CallContext::State state = m_protocolCallContext.GetState();
    if (state == nn::nex::CallContext::CallInProgress)
    {
        if (m_requestDeadline < common::g_piaJobScheduler->m_dispatchTime)
        {
            m_protocolCallContext.Cancel(nn::nex::CallContext::CallCanceled);
            m_callContext->SignalFailure(cResultRequestFailed);
            m_protocolCallContext.Reset();
            m_callContext = 0;
            return true;
        }
    }
    else if (state == nn::nex::CallContext::CallFailed)
    {
        nn::nex::qResult result = m_protocolCallContext.GetResult();
        if (result.Equals(cNexResultInvalidArgument))
        {
            m_callContext->SignalFailure(cResultInvalidArgument);
            m_protocolCallContext.Reset();
            m_callContext = 0;
            return true;
        }
        if (result.Equals(cNexResultSessionVoid))
        {
            m_callContext->SignalFailure(cResultSessionVoid);
            m_protocolCallContext.Reset();
            m_callContext = 0;
            return true;
        }
        if (result.Equals(cNexResultPermissionDenied))
        {
            m_callContext->SignalFailure(transport::cResultInvalidState);
            m_protocolCallContext.Reset();
            m_callContext = 0;
            return true;
        }
        Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
        m_callContext->SignalFailure(cResultNexError);
        m_protocolCallContext.Reset();
        m_callContext = 0;
        return true;
    }
    else if (state == nn::nex::CallContext::CallSucceeded)
    {
        m_callContext->SignalSuccess(0);
        m_protocolCallContext.Reset();
        m_callContext = 0;
        return true;
    }
    return false;
}

bool NexMatchmakeSession::WaitMigrateMatchmakeSessionOwner()
{
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallFailed)
    {
        nn::nex::qResult result = m_protocolCallContext.GetResult();
        if (result.Equals(cNexResultSessionVoid))
        {
            m_callContext->SignalFailure(cResultSessionVoid);
            m_callContext = 0;
            m_idList7c->clear();
            m_protocolCallContext.Reset();
            return true;
        }
        if (result.Equals(cNexResultPermissionDenied))
        {
            m_callContext->SignalFailure(transport::cResultInvalidState);
            m_callContext = 0;
            m_idList7c->clear();
            m_protocolCallContext.Reset();
            return true;
        }
        if (result.Equals(cNexResultNotParticipatedGathering))
        {
            m_callContext->SignalFailure(transport::cResultInvalidState);
            m_callContext = 0;
            m_idList7c->clear();
            m_protocolCallContext.Reset();
            return true;
        }
        if (result.Equals(cNexResultUserIsOffline))
        {
            m_callContext->SignalFailure(transport::cResultInvalidState);
            m_callContext = 0;
            m_idList7c->clear();
            m_protocolCallContext.Reset();
            return true;
        }
        Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
        m_callContext->SignalFailure(cResultNexError);
        m_callContext = 0;
        m_idList7c->clear();
        m_protocolCallContext.Reset();
        return true;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallSucceeded)
    {
        m_callContext->SignalSuccess(0);
        m_callContext = 0;
        m_idList7c->clear();
        m_protocolCallContext.Reset();
        return true;
    }
    if (m_requestDeadline < common::g_piaJobScheduler->m_dispatchTime)
    {
        m_protocolCallContext.Cancel(nn::nex::CallContext::CallCanceled);
        m_callContext->SignalFailure(cResultRequestFailed);
        m_idList7c->clear();
        m_protocolCallContext.Reset();
        m_callContext = 0;
        return true;
    }
    return false;
}

bool NexMatchmakeSession::WaitModifyAttribute()
{
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallFailed)
    {
        nn::nex::qResult result = m_protocolCallContext.GetResult();
        if (result.Equals(cNexResultSessionVoid))
        {
            m_callContext->SignalFailure(cResultSessionVoid);
            m_callContext = 0;
            m_protocolCallContext.Reset();
            return true;
        }
        if (result.Equals(cNexResultPermissionDenied))
        {
            m_callContext->SignalFailure(transport::cResultInvalidState);
            m_callContext = 0;
            m_protocolCallContext.Reset();
            return true;
        }
        Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
        m_callContext->SignalFailure(cResultNexError);
        m_callContext = 0;
        m_protocolCallContext.Reset();
        return true;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallSucceeded)
    {
        m_callContext->SignalSuccess(0);
        m_callContext = 0;
        m_protocolCallContext.Reset();
        return true;
    }
    return false;
}

bool NexMatchmakeSession::WaitOpenParticipate()
{
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallFailed)
    {
        nn::nex::qResult result = m_protocolCallContext.GetResult();
        if (result.Equals(cNexResultSessionVoid))
        {
            m_callContext->SignalFailure(cResultSessionVoid);
            m_callContext = 0;
            m_protocolCallContext.Reset();
            return true;
        }
        if (result.Equals(cNexResultPermissionDenied))
        {
            m_callContext->SignalFailure(transport::cResultInvalidState);
            m_callContext = 0;
            m_protocolCallContext.Reset();
            return true;
        }
        Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
        m_callContext->SignalFailure(cResultNexError);
        m_callContext = 0;
        m_protocolCallContext.Reset();
        return true;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallSucceeded)
    {
        m_callContext->SignalSuccess(0);
        m_callContext = 0;
        m_protocolCallContext.Reset();
        common::g_sessionStateMonitoringContent.m_metric223[1] = true;
        return true;
    }
    return false;
}

bool NexMatchmakeSession::WaitRandomMatchmakeSession(unsigned int* gatheringId, bool* found)
{
    nn::nex::CallContext::State state = m_protocolCallContext.GetState();
    if (state == nn::nex::CallContext::CallInProgress)
    {
        return false;
    }
    if (state == nn::nex::CallContext::CallFailed)
    {
        nn::nex::qResult result = m_protocolCallContext.GetResult();
        if (result.Equals(cNexResultInvalidArgument))
        {
            m_callContext->SignalFailure(cResultInvalidArgument);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            UnregisterSearchCriteria();
            m_autoMatchmakeParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeOwnerId = 0;
            m_temporaryOwnerChangeGatheringId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultSessionFull))
        {
            m_callContext->SignalFailure(cResultSessionFull);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            UnregisterSearchCriteria();
            m_autoMatchmakeParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeOwnerId = 0;
            m_temporaryOwnerChangeGatheringId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultSessionVoid))
        {
            m_callContext->SignalFailure(cResultSessionVoid);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            UnregisterSearchCriteria();
            m_autoMatchmakeParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeOwnerId = 0;
            m_temporaryOwnerChangeGatheringId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultDeniedByParticipants))
        {
            m_callContext->SignalFailure(cResultDeniedByParticipants);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            UnregisterSearchCriteria();
            m_autoMatchmakeParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeOwnerId = 0;
            m_temporaryOwnerChangeGatheringId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultParticipantInBlackList))
        {
            m_callContext->SignalFailure(cResultParticipantInBlackList);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            UnregisterSearchCriteria();
            m_autoMatchmakeParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeOwnerId = 0;
            m_temporaryOwnerChangeGatheringId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultGameServerMaintenance))
        {
            m_callContext->SignalFailure(cResultServerMaintenance);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            UnregisterSearchCriteria();
            m_autoMatchmakeParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeOwnerId = 0;
            m_temporaryOwnerChangeGatheringId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultOutOfRatingRange))
        {
            m_callContext->SignalFailure(transport::cResultInvalidState);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            UnregisterSearchCriteria();
            m_autoMatchmakeParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeOwnerId = 0;
            m_temporaryOwnerChangeGatheringId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultNotParticipatedGathering))
        {
            m_callContext->SignalFailure(transport::cResultInvalidState);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            UnregisterSearchCriteria();
            m_autoMatchmakeParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeOwnerId = 0;
            m_temporaryOwnerChangeGatheringId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultUserIsOffline))
        {
            m_callContext->SignalFailure(cResultOwnerOffline);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            UnregisterSearchCriteria();
            m_autoMatchmakeParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeOwnerId = 0;
            m_temporaryOwnerChangeGatheringId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultAlreadyParticipatedGathering))
        {
            m_callContext->SignalFailure(transport::cResultInvalidState);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            UnregisterSearchCriteria();
            m_autoMatchmakeParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeOwnerId = 0;
            m_temporaryOwnerChangeGatheringId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultNotFriend))
        {
            m_callContext->SignalFailure(cResultNotFriend);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            UnregisterSearchCriteria();
            m_autoMatchmakeParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeOwnerId = 0;
            m_temporaryOwnerChangeGatheringId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultSessionClosed))
        {
            m_callContext->SignalFailure(cResultSessionClosed);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            UnregisterSearchCriteria();
            m_autoMatchmakeParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeOwnerId = 0;
            m_temporaryOwnerChangeGatheringId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
        m_callContext->SignalFailure(cResultNexError);
        m_callContext = 0;
        m_idList7c->clear();
        m_foundSession->Reset();
        m_matchmakeParam->m_Parameters.clear();
        m_requestSession->Reset();
        UnregisterSearchCriteria();
        m_autoMatchmakeParam->Reset();
        m_protocolCallContext.Reset();
        m_temporaryOwnerChangeOwnerId = 0;
        m_temporaryOwnerChangeGatheringId = 0;
        m_processingJoinMatchmakeSession = false;
        return true;
    }
    if (state != nn::nex::CallContext::CallSucceeded)
    {
        return false;
    }
    Result keyResult = SetMatchmakeSessionKey(m_foundSession->m_SessionKey);
    if (keyResult == 0)
    {
        bool ownerChanged = false;
        unsigned int hostId = m_foundSession->m_uiValue3;
        unsigned int ownerId = m_foundSession->m_uiValue2;
        if (m_temporaryOwnerChangeGatheringId == m_foundSession->m_uiValue1)
        {
            if (m_temporaryOwnerChangeOwnerId != 0 && m_temporaryOwnerChangeOwnerId != ownerId)
            {
                ownerChanged = true;
            }
        }
        m_matchmakeHost = hostId == NexFacade::GetInstance()->GetLocalPrincipalId();
        m_matchmakeOwner = ownerId == NexFacade::GetInstance()->GetLocalPrincipalId();
        if (m_matchmakeHost != m_matchmakeOwner || ownerChanged)
        {
            m_callContext->SignalFailure(cResultSessionInconsistent);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            UnregisterSearchCriteria();
            m_autoMatchmakeParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeOwnerId = 0;
            m_temporaryOwnerChangeGatheringId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        m_matchmakeOwnerUniqueId = ownerId;
        *found = m_matchmakeOwner;
        *gatheringId = m_foundSession->m_uiValue1;
        m_sessionInfo.Clear();
        CopyMatchmakeSessionToSessionInfo(m_foundSession, &m_sessionInfo);
        SessionParticipantInfo* participantInfo = const_cast<SessionParticipantInfo*>(
            static_cast<NexMatchMeshLayerController*>(session::g_piaSession->m_meshLayerController)
                ->GetSessionParticipantInfo(m_foundSession->m_uiValue1));
        if (participantInfo != 0)
        {
            participantInfo->RemoveObsoleteEntry();
        }
        const nn::nex::MatchmakeSession* foundSession = m_foundSession;
        nn::nex::Variant score;
        nn::nex::qMap<nn::nex::String, nn::nex::Variant>::const_iterator it =
            foundSession->m_MatchmakeParam.m_Parameters.find(nn::nex::String(s_totalScoreKey));
        if (it != foundSession->m_MatchmakeParam.m_Parameters.end())
        {
            score = it->second;
            m_randomMatchmakeScore = score.GetUInt32Value();
            m_randomMatchmakeScoreAvailable = true;
        }
        else
        {
            m_randomMatchmakeScoreAvailable = false;
        }
        m_callContext->SignalSuccess(0);
        m_callContext = 0;
        m_idList7c->clear();
        m_foundSession->Reset();
        m_matchmakeParam->m_Parameters.clear();
        m_requestSession->Reset();
        UnregisterSearchCriteria();
        m_autoMatchmakeParam->Reset();
        m_protocolCallContext.Reset();
        m_temporaryOwnerChangeOwnerId = 0;
        m_temporaryOwnerChangeGatheringId = 0;
        m_processingJoinMatchmakeSession = false;
        return true;
    }
    m_callContext->SignalFailure(keyResult);
    m_callContext = 0;
    m_idList7c->clear();
    m_foundSession->Reset();
    m_matchmakeParam->m_Parameters.clear();
    m_requestSession->Reset();
    UnregisterSearchCriteria();
    m_autoMatchmakeParam->Reset();
    m_protocolCallContext.Reset();
    m_temporaryOwnerChangeOwnerId = 0;
    m_temporaryOwnerChangeGatheringId = 0;
    m_processingJoinMatchmakeSession = false;
    return true;
}

bool NexMatchmakeSession::WaitRandomMatchmakeSessionWithTeam(unsigned int* gatheringId, bool* found)
{
    nn::nex::CallContext::State state = m_protocolCallContext.GetState();
    if (state == nn::nex::CallContext::CallInProgress)
    {
        return false;
    }
    if (state == nn::nex::CallContext::CallFailed)
    {
        nn::nex::qResult result = m_protocolCallContext.GetResult();
        if (result.Equals(cNexResultNotParticipatedGathering))
        {
            m_callContext->SignalFailure(cResultOwnerOffline);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            UnregisterSearchCriteria();
            m_autoMatchmakeParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeOwnerId = 0;
            m_temporaryOwnerChangeGatheringId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultDeniedByParticipants))
        {
            m_callContext->SignalFailure(cResultDeniedByParticipants);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            UnregisterSearchCriteria();
            m_autoMatchmakeParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeOwnerId = 0;
            m_temporaryOwnerChangeGatheringId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultSessionFull))
        {
            m_callContext->SignalFailure(cResultSessionFull);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            UnregisterSearchCriteria();
            m_autoMatchmakeParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeOwnerId = 0;
            m_temporaryOwnerChangeGatheringId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultInvalidArgument))
        {
            m_callContext->SignalFailure(cResultInvalidArgument);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            UnregisterSearchCriteria();
            m_autoMatchmakeParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeOwnerId = 0;
            m_temporaryOwnerChangeGatheringId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultUserIsOffline))
        {
            m_callContext->SignalFailure(cResultOwnerOffline);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            UnregisterSearchCriteria();
            m_autoMatchmakeParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeOwnerId = 0;
            m_temporaryOwnerChangeGatheringId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultGameServerMaintenance))
        {
            m_callContext->SignalFailure(cResultServerMaintenance);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            UnregisterSearchCriteria();
            m_autoMatchmakeParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeOwnerId = 0;
            m_temporaryOwnerChangeGatheringId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultParticipantInBlackList))
        {
            m_callContext->SignalFailure(cResultParticipantInBlackList);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            UnregisterSearchCriteria();
            m_autoMatchmakeParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeOwnerId = 0;
            m_temporaryOwnerChangeGatheringId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultSessionClosed))
        {
            m_callContext->SignalFailure(cResultSessionClosed);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            UnregisterSearchCriteria();
            m_autoMatchmakeParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeOwnerId = 0;
            m_temporaryOwnerChangeGatheringId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultNotFriend))
        {
            m_callContext->SignalFailure(cResultNotFriend);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            UnregisterSearchCriteria();
            m_autoMatchmakeParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeOwnerId = 0;
            m_temporaryOwnerChangeGatheringId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        if (result.Equals(cNexResultAlreadyParticipatedGathering))
        {
            m_callContext->SignalFailure(transport::cResultInvalidState);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            UnregisterSearchCriteria();
            m_autoMatchmakeParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeOwnerId = 0;
            m_temporaryOwnerChangeGatheringId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
        m_callContext->SignalFailure(cResultNexError);
        m_callContext = 0;
        m_idList7c->clear();
        m_foundSession->Reset();
        m_matchmakeParam->m_Parameters.clear();
        m_requestSession->Reset();
        UnregisterSearchCriteria();
        m_autoMatchmakeParam->Reset();
        m_protocolCallContext.Reset();
        m_temporaryOwnerChangeOwnerId = 0;
        m_temporaryOwnerChangeGatheringId = 0;
        m_processingJoinMatchmakeSession = false;
        return true;
    }
    if (state != nn::nex::CallContext::CallSucceeded)
    {
        return false;
    }
    Result keyResult = SetMatchmakeSessionKey(m_foundSession->m_SessionKey);
    if (keyResult == 0)
    {
        bool ownerChanged = false;
        unsigned int hostId = m_foundSession->m_uiValue3;
        unsigned int ownerId = m_foundSession->m_uiValue2;
        if (m_temporaryOwnerChangeGatheringId == m_foundSession->m_uiValue1)
        {
            if (m_temporaryOwnerChangeOwnerId != 0 && m_temporaryOwnerChangeOwnerId != ownerId)
            {
                ownerChanged = true;
            }
        }
        m_matchmakeHost = hostId == NexFacade::GetInstance()->GetLocalPrincipalId();
        m_matchmakeOwner = ownerId == NexFacade::GetInstance()->GetLocalPrincipalId();
        if (m_matchmakeHost != m_matchmakeOwner || ownerChanged)
        {
            m_callContext->SignalFailure(cResultSessionInconsistent);
            m_callContext = 0;
            m_idList7c->clear();
            m_foundSession->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_requestSession->Reset();
            UnregisterSearchCriteria();
            m_autoMatchmakeParam->Reset();
            m_protocolCallContext.Reset();
            m_temporaryOwnerChangeOwnerId = 0;
            m_temporaryOwnerChangeGatheringId = 0;
            m_processingJoinMatchmakeSession = false;
            return true;
        }
        m_matchmakeOwnerUniqueId = ownerId;
        *found = m_matchmakeOwner;
        *gatheringId = m_foundSession->m_uiValue1;
        m_sessionInfo.Clear();
        SessionParticipantInfo* participantInfo = const_cast<SessionParticipantInfo*>(
            static_cast<NexMatchMeshLayerController*>(session::g_piaSession->m_meshLayerController)
                ->GetSessionParticipantInfo(m_foundSession->m_uiValue1));
        if (participantInfo != 0)
        {
            participantInfo->RemoveObsoleteEntry();
        }
        const nn::nex::MatchmakeSession* foundSession = m_foundSession;
        nn::nex::Variant score;
        nn::nex::qMap<nn::nex::String, nn::nex::Variant>::const_iterator it =
            foundSession->m_MatchmakeParam.m_Parameters.find(nn::nex::String(s_totalScoreKey));
        if (it != foundSession->m_MatchmakeParam.m_Parameters.end())
        {
            score = it->second;
            m_randomMatchmakeScore = score.GetUInt32Value();
            m_randomMatchmakeScoreAvailable = true;
        }
        else
        {
            m_randomMatchmakeScoreAvailable = false;
        }
        m_callContext->SignalSuccess(0);
        m_callContext = 0;
        m_idList7c->clear();
        m_foundSession->Reset();
        m_matchmakeParam->m_Parameters.clear();
        m_requestSession->Reset();
        UnregisterSearchCriteria();
        m_autoMatchmakeParam->Reset();
        m_protocolCallContext.Reset();
        m_temporaryOwnerChangeOwnerId = 0;
        m_temporaryOwnerChangeGatheringId = 0;
        m_processingJoinMatchmakeSession = false;
        return true;
    }
    m_callContext->SignalFailure(keyResult);
    m_callContext = 0;
    m_idList7c->clear();
    m_foundSession->Reset();
    m_matchmakeParam->m_Parameters.clear();
    m_requestSession->Reset();
    UnregisterSearchCriteria();
    m_autoMatchmakeParam->Reset();
    m_protocolCallContext.Reset();
    m_temporaryOwnerChangeOwnerId = 0;
    m_temporaryOwnerChangeGatheringId = 0;
    m_processingJoinMatchmakeSession = false;
    return true;
}

bool NexMatchmakeSession::WaitRequestSessionInfo()
{
    nn::nex::CallContext::State state = m_protocolCallContext.GetState();
    if (state != nn::nex::CallContext::CallInProgress)
    {
        if (state == nn::nex::CallContext::CallFailed)
        {
            nn::nex::qResult result = m_protocolCallContext.GetResult();
            if (result.Equals(cNexResultSessionVoid))
            {
                m_callContext->SignalFailure(cResultSessionVoid);
                m_callContext = 0;
                m_foundSession->Reset();
                m_protocolCallContext.Reset();
                return true;
            }
            if (result.Equals(cNexResultPermissionDenied))
            {
                m_callContext->SignalFailure(transport::cResultInvalidState);
                m_callContext = 0;
                m_foundSession->Reset();
                m_protocolCallContext.Reset();
                return true;
            }
            Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
            m_callContext->SignalFailure(cResultNexError);
            m_callContext = 0;
            m_foundSession->Reset();
            m_protocolCallContext.Reset();
            return true;
        }
        if (state == nn::nex::CallContext::CallSucceeded)
        {
            if (SetMatchmakeSessionKey(m_foundSession->m_SessionKey) != 0)
            {
                m_callContext->SignalFailure(transport::cResultInvalidState);
            }
            m_sessionInfo.Clear();
            CopyMatchmakeSessionToSessionInfo(m_foundSession, &m_sessionInfo);
            m_matchmakeOwnerUniqueId = m_foundSession->m_uiValue2;
            m_matchmakeOwner = m_matchmakeOwnerUniqueId == NexFacade::GetInstance()->GetLocalPrincipalId();
            m_matchmakeHost = m_foundSession->m_uiValue3 == NexFacade::GetInstance()->GetLocalPrincipalId();
            m_callContext->SignalSuccess(0);
            m_callContext = 0;
            m_foundSession->Reset();
            m_protocolCallContext.Reset();
            return true;
        }
        if (m_requestDeadline < common::g_piaJobScheduler->m_dispatchTime)
        {
            m_protocolCallContext.Cancel(nn::nex::CallContext::CallCanceled);
            m_callContext->SignalFailure(cResultRequestFailed);
            m_callContext = 0;
            m_foundSession->Reset();
            m_protocolCallContext.Reset();
            return true;
        }
    }
    return false;
}

bool NexMatchmakeSession::WaitRequestSessionInfoWithoutPermission()
{
    nn::nex::CallContext::State state = m_protocolCallContext.GetState();
    if (state != nn::nex::CallContext::CallInProgress)
    {
        if (state == nn::nex::CallContext::CallFailed)
        {
            nn::nex::qResult result = m_protocolCallContext.GetResult();
            if (result.GetReturnCode() == static_cast<unsigned int>(cNexResultSessionVoid))
            {
                m_callContext->SignalFailure(cResultSessionVoid);
                m_callContext = 0;
                m_foundSession->Reset();
                m_protocolCallContext.Reset();
                m_gatheringHolder.Release();
                return true;
            }
            if (result.GetReturnCode() == static_cast<unsigned int>(cNexResultGameServerMaintenance))
            {
                m_callContext->SignalFailure(cResultServerMaintenance);
                m_callContext = 0;
                m_foundSession->Reset();
                m_protocolCallContext.Reset();
                m_gatheringHolder.Release();
                return true;
            }
            Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
            m_callContext->SignalFailure(cResultNexError);
            m_callContext = 0;
            m_foundSession->Reset();
            m_protocolCallContext.Reset();
            m_gatheringHolder.Release();
            return true;
        }
        if (state == nn::nex::CallContext::CallSucceeded)
        {
            m_sessionInfo.Clear();
            m_foundSession = static_cast<nn::nex::MatchmakeSession*>(m_gatheringHolder.GetObject());
            CopyMatchmakeSessionToSessionInfo(m_foundSession, &m_sessionInfo);
            m_matchmakeOwnerUniqueId = m_foundSession->m_uiValue2;
            m_matchmakeOwner = m_matchmakeOwnerUniqueId == NexFacade::GetInstance()->GetLocalPrincipalId();
            m_matchmakeHost = m_foundSession->m_uiValue3 == NexFacade::GetInstance()->GetLocalPrincipalId();
            m_callContext->SignalSuccess(0);
            m_callContext = 0;
            m_foundSession->Reset();
            m_protocolCallContext.Reset();
            m_gatheringHolder.Release();
            return true;
        }
        if (m_requestDeadline < common::g_piaJobScheduler->m_dispatchTime)
        {
            m_protocolCallContext.Cancel(nn::nex::CallContext::CallCanceled);
            m_callContext->SignalFailure(cResultRequestFailed);
            m_callContext = 0;
            m_foundSession->Reset();
            m_protocolCallContext.Reset();
            m_gatheringHolder.Release();
            return true;
        }
    }
    return false;
}

bool NexMatchmakeSession::WaitUnregisterMatchmakeSession()
{
    nn::nex::CallContext::State state = m_protocolCallContext.GetState();
    if (state == nn::nex::CallContext::CallInProgress)
    {
        if (common::g_piaJobScheduler->m_dispatchTime <= m_requestDeadline)
        {
            return false;
        }
        m_protocolCallContext.Cancel(nn::nex::CallContext::CallCanceled);
        m_callContext->SignalFailure(cResultRequestFailed);
        m_protocolCallContext.Reset();
        m_foundSession->Reset();
        m_callContext = 0;
    }
    else if (state == nn::nex::CallContext::CallFailed)
    {
        nn::nex::qResult result = m_protocolCallContext.GetResult();
        if (result.Equals(cNexResultSessionVoid))
        {
            m_callContext->SignalFailure(cResultSessionVoid);
            m_protocolCallContext.Reset();
            m_foundSession->Reset();
            m_callContext = 0;
        }
        else if (result.Equals(cNexResultPermissionDenied))
        {
            m_callContext->SignalFailure(transport::cResultInvalidState);
            m_protocolCallContext.Reset();
            m_foundSession->Reset();
            m_callContext = 0;
        }
        else
        {
            Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
            m_callContext->SignalFailure(cResultNexError);
            m_protocolCallContext.Reset();
            m_foundSession->Reset();
            m_callContext = 0;
        }
    }
    else if (state == nn::nex::CallContext::CallSucceeded)
    {
        m_matchmakeOwner = false;
        m_matchmakeOwnerUniqueId = 0;
        m_matchmakeHost = false;
        m_callContext->SignalSuccess(0);
        m_protocolCallContext.Reset();
        m_foundSession->Reset();
        m_callContext = 0;
    }
    else
    {
        return false;
    }
    NexMatchMeshLayerController* meshLayerController = static_cast<NexMatchMeshLayerController*>(
        session::g_piaSession->m_meshLayerController);
    if (meshLayerController != 0)
    {
        meshLayerController->ClearSessionId(m_gatheringId);
    }
    m_gatheringId = 0;
    m_sessionInfo.Clear();
    return true;
}

bool NexMatchmakeSession::WaitUpdateApplicationBuffer()
{
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallFailed)
    {
        nn::nex::qResult result = m_protocolCallContext.GetResult();
        if (result.Equals(cNexResultSessionVoid))
        {
            m_callContext->SignalFailure(cResultSessionVoid);
            m_callContext = 0;
            m_protocolCallContext.Reset();
            return true;
        }
        if (result.Equals(cNexResultInvalidArgument))
        {
            m_callContext->SignalFailure(cResultInvalidArgument);
            m_callContext = 0;
            m_protocolCallContext.Reset();
            return true;
        }
        if (result.Equals(cNexResultGameServerMaintenance))
        {
            m_callContext->SignalFailure(cResultServerMaintenance);
            m_callContext = 0;
            m_protocolCallContext.Reset();
            return true;
        }
        if (result.Equals(cNexResultPermissionDenied))
        {
            m_callContext->SignalFailure(transport::cResultInvalidState);
            m_callContext = 0;
            m_protocolCallContext.Reset();
            return true;
        }
        Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
        m_callContext->SignalFailure(cResultNexError);
        m_callContext = 0;
        m_protocolCallContext.Reset();
        return true;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallSucceeded)
    {
        m_callContext->SignalSuccess(0);
        m_callContext = 0;
        m_protocolCallContext.Reset();
        return true;
    }
    return false;
}

bool NexMatchmakeSession::WaitUpdateAttributes()
{
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallFailed)
    {
        nn::nex::qResult result = m_protocolCallContext.GetResult();
        if (result.Equals(cNexResultSessionVoid))
        {
            m_callContext->SignalFailure(cResultSessionVoid);
            m_callContext = 0;
            m_protocolCallContext.Reset();
            return true;
        }
        if (result.Equals(cNexResultPermissionDenied))
        {
            m_callContext->SignalFailure(transport::cResultInvalidState);
            m_callContext = 0;
            m_protocolCallContext.Reset();
            return true;
        }
        Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
        m_callContext->SignalFailure(cResultNexError);
        m_callContext = 0;
        m_protocolCallContext.Reset();
        return true;
    }
    if (m_protocolCallContext.GetState() == nn::nex::CallContext::CallSucceeded)
    {
        m_callContext->SignalSuccess(0);
        m_callContext = 0;
        m_protocolCallContext.Reset();
        return true;
    }
    return false;
}

bool NexMatchmakeSession::WaitUpdateCommunity()
{
    nn::nex::CallContext::State state = m_protocolCallContext.GetState();
    if (state != nn::nex::CallContext::CallInProgress)
    {
        if (state == nn::nex::CallContext::CallFailed)
        {
            nn::nex::qResult result = m_protocolCallContext.GetResult();
            if (result.Equals(cNexResultInvalidArgument))
            {
                m_callContext->SignalFailure(cResultInvalidArgument);
                m_persistentGathering->Reset();
                m_protocolCallContext.Reset();
                m_callContext = 0;
                return true;
            }
            if (result.Equals(cNexResultSessionVoid))
            {
                m_callContext->SignalFailure(cResultSessionVoid);
                m_persistentGathering->Reset();
                m_protocolCallContext.Reset();
                m_callContext = 0;
                return true;
            }
            if (result.Equals(cNexResultPermissionDenied))
            {
                m_callContext->SignalFailure(transport::cResultInvalidState);
                m_persistentGathering->Reset();
                m_protocolCallContext.Reset();
                m_callContext = 0;
                return true;
            }
            Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
            m_callContext->SignalFailure(cResultNexError);
            m_persistentGathering->Reset();
            m_protocolCallContext.Reset();
            m_callContext = 0;
            return true;
        }
        if (state == nn::nex::CallContext::CallSucceeded)
        {
            m_callContext->SignalSuccess(0);
            m_callContext = 0;
            m_persistentGathering->Reset();
            m_protocolCallContext.Reset();
            return true;
        }
    }
    return false;
}

bool NexMatchmakeSession::WaitUpdateSessionSetting()
{
    nn::nex::CallContext::State state = m_protocolCallContext.GetState();
    if (state != nn::nex::CallContext::CallInProgress)
    {
        if (state == nn::nex::CallContext::CallFailed)
        {
            nn::nex::qResult result = m_protocolCallContext.GetResult();
            if (result.Equals(cNexResultInvalidArgument))
            {
                m_callContext->SignalFailure(cResultInvalidArgument);
            }
            else if (result.Equals(cNexResultSessionVoid))
            {
                m_callContext->SignalFailure(cResultSessionVoid);
            }
            else if (result.Equals(cNexResultPermissionDenied))
            {
                m_callContext->SignalFailure(transport::cResultInvalidState);
            }
            else
            {
                Result::SetNexNetworkErrorCode(nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(result));
                m_callContext->SignalFailure(cResultNexError);
            }
            m_callContext = 0;
            m_updateMatchmakeSessionParam->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_protocolCallContext.Reset();
            return true;
        }
        if (state == nn::nex::CallContext::CallSucceeded)
        {
            m_callContext->SignalSuccess(0);
            m_callContext = 0;
            m_updateMatchmakeSessionParam->Reset();
            m_matchmakeParam->m_Parameters.clear();
            m_protocolCallContext.Reset();
            return true;
        }
    }
    return false;
}

}
}
}
