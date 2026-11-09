#include "nn/nex/MatchmakeExtensionProtocolClient.hpp"

#include "nn/nex/AutoMatchmakeParam.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/CreateMatchmakeSessionParam.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/DDLClassMetaType.hpp"
#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Gathering.hpp"
#include "nn/nex/GatheringURLs.hpp"
#include "nn/nex/JoinMatchmakeSessionParam.hpp"
#include "nn/nex/MatchmakeSession.hpp"
#include "nn/nex/MatchmakeSessionSearchCriteria.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/NotificationEvent.hpp"
#include "nn/nex/PersistentGathering.hpp"
#include "nn/nex/PlayingSession.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/ProtocolRegistry.hpp"
#include "nn/nex/ProtocolRequestBrokerInterface.hpp"
#include "nn/nex/RVClientCore.hpp"
#include "nn/nex/ResultRange.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/SimpleCommunity.hpp"
#include "nn/nex/SimplePlayingSession.hpp"
#include "nn/nex/UpdateMatchmakeSessionParam.hpp"
#include "nn/nex/_Type_qlist.hpp"

namespace nn
{
namespace nex
{

namespace
{

typedef void (*ProtoReturnFunction)(Message* pMessage, ProtocolCallContext* pCallContext);
typedef _Type_any<DDLClassMetaType<Gathering>, _Type_string> GatheringAny;
typedef MatchmakeExtensionProtocolClient::GatheringHolder GatheringHolder;
typedef MatchmakeExtensionProtocolClient::GatheringHolderList GatheringHolderList;

const unsigned int METHOD_ID_MASK = 0x7FFF;
enum
{
    RESULT_NOT_IMPLEMENTED = static_cast<int>(0x80010002u)
};
const unsigned int METHOD_ID_COUNT = 48;
const unsigned int PROTOCOL_FLAG_UNLOCK_BEFORE_SEND = 0x10;

enum MethodId
{
    METHOD_CLOSE_PARTICIPATION = 1,
    METHOD_OPEN_PARTICIPATION = 2,
    METHOD_AUTO_MATCHMAKE_POSTPONE = 3,
    METHOD_BROWSE_MATCHMAKE_SESSION = 4,
    METHOD_BROWSE_MATCHMAKE_SESSION_WITH_HOST_URLS = 5,
    METHOD_CREATE_MATCHMAKE_SESSION = 6,
    METHOD_JOIN_MATCHMAKE_SESSION = 7,
    METHOD_MODIFY_CURRENT_GAME_ATTRIBUTE = 8,
    METHOD_UPDATE_NOTIFICATION_DATA = 9,
    METHOD_GET_FRIEND_NOTIFICATION_DATA = 10,
    METHOD_UPDATE_APPLICATION_BUFFER = 11,
    METHOD_UPDATE_MATCHMAKE_SESSION_ATTRIBUTE = 12,
    METHOD_GETLST_FRIEND_NOTIFICATION_DATA = 13,
    METHOD_UPDATE_MATCHMAKE_SESSION = 14,
    METHOD_AUTO_MATCHMAKE_WITH_SEARCH_CRITERIA_POSTPONE = 15,
    METHOD_GET_PLAYING_SESSION = 16,
    METHOD_CREATE_COMMUNITY = 17,
    METHOD_UPDATE_COMMUNITY = 18,
    METHOD_JOIN_COMMUNITY = 19,
    METHOD_FIND_COMMUNITY_BY_GATHERING_ID = 20,
    METHOD_FIND_OFFICIAL_COMMUNITY = 21,
    METHOD_FIND_COMMUNITY_BY_PARTICIPANT = 22,
    METHOD_UPDATE_PRIVACY_SETTING = 23,
    METHOD_GET_MY_BLACK_LIST = 24,
    METHOD_ADD_TO_BLACK_LIST = 25,
    METHOD_REMOVE_FROM_BLACK_LIST = 26,
    METHOD_CLEAR_MY_BLACK_LIST = 27,
    METHOD_REPORT_VIOLATION = 28,
    METHOD_IS_VIOLATION_USER = 29,
    METHOD_JOIN_MATCHMAKE_SESSION_EX = 30,
    METHOD_GET_SIMPLE_PLAYING_SESSION = 31,
    METHOD_GET_SIMPLE_COMMUNITY = 32,
    METHOD_AUTO_MATCHMAKE_WITH_GATHERING_ID_POSTPONE = 33,
    METHOD_UPDATE_PROGRESS_SCORE = 34,
    METHOD_DEBUG_NOTIFY_EVENT = 35,
    METHOD_GENERATE_MATCHMAKE_SESSION_SYSTEM_PASSWORD = 36,
    METHOD_CLEAR_MATCHMAKE_SESSION_SYSTEM_PASSWORD = 37,
    METHOD_CREATE_MATCHMAKE_SESSION_WITH_PARAM = 38,
    METHOD_JOIN_MATCHMAKE_SESSION_WITH_PARAM = 39,
    METHOD_AUTO_MATCHMAKE_WITH_PARAM_POSTPONE = 40,
    METHOD_FIND_MATCHMAKE_SESSION_BY_GATHERING_ID_DETAIL = 41,
    METHOD_BROWSE_MATCHMAKE_SESSION_NO_HOLDER = 42,
    METHOD_BROWSE_MATCHMAKE_SESSION_WITH_HOST_URLS_NO_HOLDER = 43,
    METHOD_UPDATE_MATCHMAKE_SESSION_PART = 44,
    METHOD_REQUEST_MATCHMAKING = 45,
    METHOD_WITHDRAW_MATCHMAKING = 46,
    METHOD_WITHDRAW_MATCHMAKING_ALL = 47
};

ProtoReturnFunction s_ProtoReturn[METHOD_ID_COUNT];

}

MatchmakeExtensionProtocolClient::MatchmakeExtensionProtocolClient(unsigned short uiProtocolID) : ClientProtocol(1)
{
    if (uiProtocolID == 0)
        return;
    SetProtocolID(uiProtocolID);
    RVClientCore::GetInstance()->GetProtocolRegistry()->RegisterProtocol(uiProtocolID,
                                                                         new MatchmakeExtensionProtocolClient());
}

void MatchmakeExtensionProtocolClient::ExtractCallSpecificResults(Message* pMessage, ProtocolCallContext* pCallContext)
{
    const unsigned int uiMethodID = Protocol::ExtractMethodID(pMessage) & METHOD_ID_MASK;
    if (uiMethodID - 1 >= METHOD_ID_COUNT - 1)
    {
        ClientProtocol::SetCallError(qResult(RESULT_NOT_IMPLEMENTED));
        return;
    }
    if (s_ProtoReturn[uiMethodID] != 0)
        s_ProtoReturn[uiMethodID](pMessage, pCallContext);
}

Protocol* MatchmakeExtensionProtocolClient::CreateResponder() const
{
    return new MatchmakeExtensionProtocolClient();
}

bool MatchmakeExtensionProtocolClient::CallCloseParticipation(ProtocolCallContext* pCallContext,
                                                              const unsigned int& rGatheringID)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_CLOSE_PARTICIPATION);
    _Type_uint32::Add(&message, rGatheringID);
    s_ProtoReturn[METHOD_CLOSE_PARTICIPATION] = ProtoReturn_CloseParticipation;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::CloseParticipation_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                         Message* pMessage,
                                                                         const unsigned int& rGatheringID)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_CLOSE_PARTICIPATION);
    _Type_uint32::Add(pMessage, rGatheringID);
    s_ProtoReturn[METHOD_CLOSE_PARTICIPATION] = ProtoReturn_CloseParticipation;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallOpenParticipation(ProtocolCallContext* pCallContext,
                                                             const unsigned int& rGatheringID)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_OPEN_PARTICIPATION);
    _Type_uint32::Add(&message, rGatheringID);
    s_ProtoReturn[METHOD_OPEN_PARTICIPATION] = ProtoReturn_OpenParticipation;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::OpenParticipation_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                        Message* pMessage,
                                                                        const unsigned int& rGatheringID)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_OPEN_PARTICIPATION);
    _Type_uint32::Add(pMessage, rGatheringID);
    s_ProtoReturn[METHOD_OPEN_PARTICIPATION] = ProtoReturn_OpenParticipation;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallAutoMatchmake_Postpone(ProtocolCallContext* pCallContext,
                                                                  const GatheringHolder& rGathering,
                                                                  GatheringHolder* pJoinedGathering,
                                                                  const String& rMessage)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_AUTO_MATCHMAKE_POSTPONE);
    GatheringAny::Add(&message, rGathering);
    _Type_string::Add(&message, rMessage);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pJoinedGathering);
    }
    s_ProtoReturn[METHOD_AUTO_MATCHMAKE_POSTPONE] = ProtoReturn_AutoMatchmake_Postpone;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::AutoMatchmake_Postpone_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                             Message* pMessage,
                                                                             const GatheringHolder& rGathering,
                                                                             GatheringHolder* pJoinedGathering,
                                                                             const String& rMessage)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_AUTO_MATCHMAKE_POSTPONE);
    GatheringAny::Add(pMessage, rGathering);
    _Type_string::Add(pMessage, rMessage);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pJoinedGathering);
    }
    s_ProtoReturn[METHOD_AUTO_MATCHMAKE_POSTPONE] = ProtoReturn_AutoMatchmake_Postpone;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallBrowseMatchmakeSession(ProtocolCallContext* pCallContext,
                                                                  const MatchmakeSessionSearchCriteria& rSearchCriteria,
                                                                  const ResultRange& rRange,
                                                                  GatheringHolderList* pGatherings)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_BROWSE_MATCHMAKE_SESSION);
    _DDL_MatchmakeSessionSearchCriteria::Add(&message, rSearchCriteria);
    _DDL_ResultRange::Add(&message, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pGatherings);
    }
    s_ProtoReturn[METHOD_BROWSE_MATCHMAKE_SESSION] = ProtoReturn_BrowseMatchmakeSession;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::BrowseMatchmakeSession_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const MatchmakeSessionSearchCriteria& rSearchCriteria,
    const ResultRange& rRange, GatheringHolderList* pGatherings)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_BROWSE_MATCHMAKE_SESSION);
    _DDL_MatchmakeSessionSearchCriteria::Add(pMessage, rSearchCriteria);
    _DDL_ResultRange::Add(pMessage, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pGatherings);
    }
    s_ProtoReturn[METHOD_BROWSE_MATCHMAKE_SESSION] = ProtoReturn_BrowseMatchmakeSession;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallBrowseMatchmakeSessionWithHostUrls(
    ProtocolCallContext* pCallContext, const MatchmakeSessionSearchCriteria& rSearchCriteria, const ResultRange& rRange,
    GatheringHolderList* pGatherings, qList<GatheringURLs>* pGatheringURLs)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_BROWSE_MATCHMAKE_SESSION_WITH_HOST_URLS);
    _DDL_MatchmakeSessionSearchCriteria::Add(&message, rSearchCriteria);
    _DDL_ResultRange::Add(&message, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pGatherings);
        pCallContext->AddReturnValuePtr(pGatheringURLs);
    }
    s_ProtoReturn[METHOD_BROWSE_MATCHMAKE_SESSION_WITH_HOST_URLS] = ProtoReturn_BrowseMatchmakeSessionWithHostUrls;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::BrowseMatchmakeSessionWithHostUrls_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const MatchmakeSessionSearchCriteria& rSearchCriteria,
    const ResultRange& rRange, GatheringHolderList* pGatherings, qList<GatheringURLs>* pGatheringURLs)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_BROWSE_MATCHMAKE_SESSION_WITH_HOST_URLS);
    _DDL_MatchmakeSessionSearchCriteria::Add(pMessage, rSearchCriteria);
    _DDL_ResultRange::Add(pMessage, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pGatherings);
        pCallContext->AddReturnValuePtr(pGatheringURLs);
    }
    s_ProtoReturn[METHOD_BROWSE_MATCHMAKE_SESSION_WITH_HOST_URLS] = ProtoReturn_BrowseMatchmakeSessionWithHostUrls;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallCreateMatchmakeSession(ProtocolCallContext* pCallContext,
                                                                  const GatheringHolder& rGathering,
                                                                  unsigned int* pGatheringID, const String& rMessage,
                                                                  qVector<unsigned char>* pSessionKey,
                                                                  const unsigned short& rParticipationCount)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_CREATE_MATCHMAKE_SESSION);
    GatheringAny::Add(&message, rGathering);
    _Type_string::Add(&message, rMessage);
    _Type_uint16::Add(&message, rParticipationCount);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pGatheringID);
        pCallContext->AddReturnValuePtr(pSessionKey);
    }
    s_ProtoReturn[METHOD_CREATE_MATCHMAKE_SESSION] = ProtoReturn_CreateMatchmakeSession;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::CreateMatchmakeSession_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const GatheringHolder& rGathering, unsigned int* pGatheringID,
    const String& rMessage, qVector<unsigned char>* pSessionKey, const unsigned short& rParticipationCount)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_CREATE_MATCHMAKE_SESSION);
    GatheringAny::Add(pMessage, rGathering);
    _Type_string::Add(pMessage, rMessage);
    _Type_uint16::Add(pMessage, rParticipationCount);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pGatheringID);
        pCallContext->AddReturnValuePtr(pSessionKey);
    }
    s_ProtoReturn[METHOD_CREATE_MATCHMAKE_SESSION] = ProtoReturn_CreateMatchmakeSession;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallJoinMatchmakeSession(ProtocolCallContext* pCallContext,
                                                                const unsigned int& rGatheringID,
                                                                const String& rMessage,
                                                                qVector<unsigned char>* pSessionKey)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_JOIN_MATCHMAKE_SESSION);
    _Type_uint32::Add(&message, rGatheringID);
    _Type_string::Add(&message, rMessage);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSessionKey);
    }
    s_ProtoReturn[METHOD_JOIN_MATCHMAKE_SESSION] = ProtoReturn_JoinMatchmakeSession;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::JoinMatchmakeSession_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                           Message* pMessage,
                                                                           const unsigned int& rGatheringID,
                                                                           const String& rMessage,
                                                                           qVector<unsigned char>* pSessionKey)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_JOIN_MATCHMAKE_SESSION);
    _Type_uint32::Add(pMessage, rGatheringID);
    _Type_string::Add(pMessage, rMessage);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSessionKey);
    }
    s_ProtoReturn[METHOD_JOIN_MATCHMAKE_SESSION] = ProtoReturn_JoinMatchmakeSession;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallModifyCurrentGameAttribute(ProtocolCallContext* pCallContext,
                                                                      const unsigned int& rGatheringID,
                                                                      const unsigned int& rAttributeIndex,
                                                                      const unsigned int& rNewValue)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_MODIFY_CURRENT_GAME_ATTRIBUTE);
    _Type_uint32::Add(&message, rGatheringID);
    _Type_uint32::Add(&message, rAttributeIndex);
    _Type_uint32::Add(&message, rNewValue);
    s_ProtoReturn[METHOD_MODIFY_CURRENT_GAME_ATTRIBUTE] = ProtoReturn_ModifyCurrentGameAttribute;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::ModifyCurrentGameAttribute_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                                 Message* pMessage,
                                                                                 const unsigned int& rGatheringID,
                                                                                 const unsigned int& rAttributeIndex,
                                                                                 const unsigned int& rNewValue)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_MODIFY_CURRENT_GAME_ATTRIBUTE);
    _Type_uint32::Add(pMessage, rGatheringID);
    _Type_uint32::Add(pMessage, rAttributeIndex);
    _Type_uint32::Add(pMessage, rNewValue);
    s_ProtoReturn[METHOD_MODIFY_CURRENT_GAME_ATTRIBUTE] = ProtoReturn_ModifyCurrentGameAttribute;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallUpdateNotificationData(ProtocolCallContext* pCallContext,
                                                                  const unsigned int& rType,
                                                                  const unsigned int& rParam1,
                                                                  const unsigned int& rParam2, const String& rParam3)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_UPDATE_NOTIFICATION_DATA);
    _Type_uint32::Add(&message, rType);
    _Type_uint32::Add(&message, rParam1);
    _Type_uint32::Add(&message, rParam2);
    _Type_string::Add(&message, rParam3);
    s_ProtoReturn[METHOD_UPDATE_NOTIFICATION_DATA] = ProtoReturn_UpdateNotificationData;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::UpdateNotificationData_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const unsigned int& rType, const unsigned int& rParam1,
    const unsigned int& rParam2, const String& rParam3)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_UPDATE_NOTIFICATION_DATA);
    _Type_uint32::Add(pMessage, rType);
    _Type_uint32::Add(pMessage, rParam1);
    _Type_uint32::Add(pMessage, rParam2);
    _Type_string::Add(pMessage, rParam3);
    s_ProtoReturn[METHOD_UPDATE_NOTIFICATION_DATA] = ProtoReturn_UpdateNotificationData;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallGetFriendNotificationData(ProtocolCallContext* pCallContext,
                                                                     qList<NotificationEvent>* pNotificationEvents,
                                                                     const int& rType)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_FRIEND_NOTIFICATION_DATA);
    message.Append(reinterpret_cast<const unsigned char*>(&rType), sizeof(rType), 1);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pNotificationEvents);
    }
    s_ProtoReturn[METHOD_GET_FRIEND_NOTIFICATION_DATA] = ProtoReturn_GetFriendNotificationData;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::GetFriendNotificationData_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, qList<NotificationEvent>* pNotificationEvents,
    const int& rType)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_GET_FRIEND_NOTIFICATION_DATA);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rType), sizeof(rType), 1);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pNotificationEvents);
    }
    s_ProtoReturn[METHOD_GET_FRIEND_NOTIFICATION_DATA] = ProtoReturn_GetFriendNotificationData;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallUpdateApplicationBuffer(ProtocolCallContext* pCallContext,
                                                                   const unsigned int& rGatheringID,
                                                                   const qVector<unsigned char>& rApplicationBuffer)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_UPDATE_APPLICATION_BUFFER);
    _Type_uint32::Add(&message, rGatheringID);
    _Type_qvector<_Type_byte>::Add(&message, rApplicationBuffer);
    s_ProtoReturn[METHOD_UPDATE_APPLICATION_BUFFER] = ProtoReturn_UpdateApplicationBuffer;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::UpdateApplicationBuffer_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const unsigned int& rGatheringID,
    const qVector<unsigned char>& rApplicationBuffer)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_UPDATE_APPLICATION_BUFFER);
    _Type_uint32::Add(pMessage, rGatheringID);
    _Type_qvector<_Type_byte>::Add(pMessage, rApplicationBuffer);
    s_ProtoReturn[METHOD_UPDATE_APPLICATION_BUFFER] = ProtoReturn_UpdateApplicationBuffer;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallUpdateMatchmakeSessionAttribute(ProtocolCallContext* pCallContext,
                                                                           const unsigned int& rGatheringID,
                                                                           const qVector<unsigned int>& rAttributes)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_UPDATE_MATCHMAKE_SESSION_ATTRIBUTE);
    _Type_uint32::Add(&message, rGatheringID);
    _Type_qvector<_Type_uint32>::Add(&message, rAttributes);
    s_ProtoReturn[METHOD_UPDATE_MATCHMAKE_SESSION_ATTRIBUTE] = ProtoReturn_UpdateMatchmakeSessionAttribute;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::UpdateMatchmakeSessionAttribute_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const unsigned int& rGatheringID,
    const qVector<unsigned int>& rAttributes)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_UPDATE_MATCHMAKE_SESSION_ATTRIBUTE);
    _Type_uint32::Add(pMessage, rGatheringID);
    _Type_qvector<_Type_uint32>::Add(pMessage, rAttributes);
    s_ProtoReturn[METHOD_UPDATE_MATCHMAKE_SESSION_ATTRIBUTE] = ProtoReturn_UpdateMatchmakeSessionAttribute;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallGetlstFriendNotificationData(ProtocolCallContext* pCallContext,
                                                                        qList<NotificationEvent>* pNotificationEvents,
                                                                        const qList<unsigned int>& rTypes)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GETLST_FRIEND_NOTIFICATION_DATA);
    _Type_qlist<_Type_uint32>::Add(&message, rTypes);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pNotificationEvents);
    }
    s_ProtoReturn[METHOD_GETLST_FRIEND_NOTIFICATION_DATA] = ProtoReturn_GetlstFriendNotificationData;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::GetlstFriendNotificationData_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, qList<NotificationEvent>* pNotificationEvents,
    const qList<unsigned int>& rTypes)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_GETLST_FRIEND_NOTIFICATION_DATA);
    _Type_qlist<_Type_uint32>::Add(pMessage, rTypes);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pNotificationEvents);
    }
    s_ProtoReturn[METHOD_GETLST_FRIEND_NOTIFICATION_DATA] = ProtoReturn_GetlstFriendNotificationData;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallUpdateMatchmakeSession(ProtocolCallContext* pCallContext,
                                                                  const GatheringHolder& rGathering)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_UPDATE_MATCHMAKE_SESSION);
    GatheringAny::Add(&message, rGathering);
    s_ProtoReturn[METHOD_UPDATE_MATCHMAKE_SESSION] = ProtoReturn_UpdateMatchmakeSession;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::UpdateMatchmakeSession_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                             Message* pMessage,
                                                                             const GatheringHolder& rGathering)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_UPDATE_MATCHMAKE_SESSION);
    GatheringAny::Add(pMessage, rGathering);
    s_ProtoReturn[METHOD_UPDATE_MATCHMAKE_SESSION] = ProtoReturn_UpdateMatchmakeSession;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallAutoMatchmakeWithSearchCriteria_Postpone(
    ProtocolCallContext* pCallContext, const qList<MatchmakeSessionSearchCriteria>& rSearchCriteria,
    const GatheringHolder& rGathering, GatheringHolder* pJoinedGathering, const String& rMessage)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_AUTO_MATCHMAKE_WITH_SEARCH_CRITERIA_POSTPONE);
    _Type_qlist<DDLClassMetaType<MatchmakeSessionSearchCriteria> >::Add(&message, rSearchCriteria);
    GatheringAny::Add(&message, rGathering);
    _Type_string::Add(&message, rMessage);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pJoinedGathering);
    }
    s_ProtoReturn[METHOD_AUTO_MATCHMAKE_WITH_SEARCH_CRITERIA_POSTPONE] =
        ProtoReturn_AutoMatchmakeWithSearchCriteria_Postpone;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::AutoMatchmakeWithSearchCriteria_Postpone_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const qList<MatchmakeSessionSearchCriteria>& rSearchCriteria,
    const GatheringHolder& rGathering, GatheringHolder* pJoinedGathering, const String& rMessage)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_AUTO_MATCHMAKE_WITH_SEARCH_CRITERIA_POSTPONE);
    _Type_qlist<DDLClassMetaType<MatchmakeSessionSearchCriteria> >::Add(pMessage, rSearchCriteria);
    GatheringAny::Add(pMessage, rGathering);
    _Type_string::Add(pMessage, rMessage);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pJoinedGathering);
    }
    s_ProtoReturn[METHOD_AUTO_MATCHMAKE_WITH_SEARCH_CRITERIA_POSTPONE] =
        ProtoReturn_AutoMatchmakeWithSearchCriteria_Postpone;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallGetPlayingSession(ProtocolCallContext* pCallContext,
                                                             const qList<unsigned int>& rPrincipalIDs,
                                                             qList<PlayingSession>* pPlayingSessions)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_PLAYING_SESSION);
    _Type_qlist<_Type_uint32>::Add(&message, rPrincipalIDs);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pPlayingSessions);
    }
    s_ProtoReturn[METHOD_GET_PLAYING_SESSION] = ProtoReturn_GetPlayingSession;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::GetPlayingSession_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                        Message* pMessage,
                                                                        const qList<unsigned int>& rPrincipalIDs,
                                                                        qList<PlayingSession>* pPlayingSessions)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_GET_PLAYING_SESSION);
    _Type_qlist<_Type_uint32>::Add(pMessage, rPrincipalIDs);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pPlayingSessions);
    }
    s_ProtoReturn[METHOD_GET_PLAYING_SESSION] = ProtoReturn_GetPlayingSession;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallCreateCommunity(ProtocolCallContext* pCallContext,
                                                           const PersistentGathering& rCommunity,
                                                           unsigned int* pGatheringID, const String& rMessage)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_CREATE_COMMUNITY);
    _DDL_PersistentGathering::Add(&message, rCommunity);
    _Type_string::Add(&message, rMessage);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pGatheringID);
    }
    s_ProtoReturn[METHOD_CREATE_COMMUNITY] = ProtoReturn_CreateCommunity;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::CreateCommunity_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                      Message* pMessage,
                                                                      const PersistentGathering& rCommunity,
                                                                      unsigned int* pGatheringID,
                                                                      const String& rMessage)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_CREATE_COMMUNITY);
    _DDL_PersistentGathering::Add(pMessage, rCommunity);
    _Type_string::Add(pMessage, rMessage);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pGatheringID);
    }
    s_ProtoReturn[METHOD_CREATE_COMMUNITY] = ProtoReturn_CreateCommunity;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallUpdateCommunity(ProtocolCallContext* pCallContext,
                                                           const PersistentGathering& rCommunity)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_UPDATE_COMMUNITY);
    _DDL_PersistentGathering::Add(&message, rCommunity);
    s_ProtoReturn[METHOD_UPDATE_COMMUNITY] = ProtoReturn_UpdateCommunity;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::UpdateCommunity_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                      Message* pMessage,
                                                                      const PersistentGathering& rCommunity)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_UPDATE_COMMUNITY);
    _DDL_PersistentGathering::Add(pMessage, rCommunity);
    s_ProtoReturn[METHOD_UPDATE_COMMUNITY] = ProtoReturn_UpdateCommunity;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallJoinCommunity(ProtocolCallContext* pCallContext,
                                                         const unsigned int& rGatheringID, const String& rMessage,
                                                         const String& rPassword)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_JOIN_COMMUNITY);
    _Type_uint32::Add(&message, rGatheringID);
    _Type_string::Add(&message, rMessage);
    _Type_string::Add(&message, rPassword);
    s_ProtoReturn[METHOD_JOIN_COMMUNITY] = ProtoReturn_JoinCommunity;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::JoinCommunity_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                    Message* pMessage, const unsigned int& rGatheringID,
                                                                    const String& rMessage, const String& rPassword)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_JOIN_COMMUNITY);
    _Type_uint32::Add(pMessage, rGatheringID);
    _Type_string::Add(pMessage, rMessage);
    _Type_string::Add(pMessage, rPassword);
    s_ProtoReturn[METHOD_JOIN_COMMUNITY] = ProtoReturn_JoinCommunity;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallFindCommunityByGatheringId(ProtocolCallContext* pCallContext,
                                                                      const qList<unsigned int>& rGatheringIDs,
                                                                      qList<PersistentGathering>* pCommunities)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_FIND_COMMUNITY_BY_GATHERING_ID);
    _Type_qlist<_Type_uint32>::Add(&message, rGatheringIDs);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pCommunities);
    }
    s_ProtoReturn[METHOD_FIND_COMMUNITY_BY_GATHERING_ID] = ProtoReturn_FindCommunityByGatheringId;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::FindCommunityByGatheringId_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const qList<unsigned int>& rGatheringIDs,
    qList<PersistentGathering>* pCommunities)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_FIND_COMMUNITY_BY_GATHERING_ID);
    _Type_qlist<_Type_uint32>::Add(pMessage, rGatheringIDs);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pCommunities);
    }
    s_ProtoReturn[METHOD_FIND_COMMUNITY_BY_GATHERING_ID] = ProtoReturn_FindCommunityByGatheringId;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallFindOfficialCommunity(ProtocolCallContext* pCallContext,
                                                                 qList<PersistentGathering>* pCommunities,
                                                                 const bool& rAvailableOnly, const ResultRange& rRange)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_FIND_OFFICIAL_COMMUNITY);
    message << rAvailableOnly;
    _DDL_ResultRange::Add(&message, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pCommunities);
    }
    s_ProtoReturn[METHOD_FIND_OFFICIAL_COMMUNITY] = ProtoReturn_FindOfficialCommunity;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::FindOfficialCommunity_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                            Message* pMessage,
                                                                            qList<PersistentGathering>* pCommunities,
                                                                            const bool& rAvailableOnly,
                                                                            const ResultRange& rRange)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_FIND_OFFICIAL_COMMUNITY);
    *pMessage << rAvailableOnly;
    _DDL_ResultRange::Add(pMessage, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pCommunities);
    }
    s_ProtoReturn[METHOD_FIND_OFFICIAL_COMMUNITY] = ProtoReturn_FindOfficialCommunity;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallFindCommunityByParticipant(ProtocolCallContext* pCallContext,
                                                                      qList<PersistentGathering>* pCommunities,
                                                                      const unsigned int& rPrincipalID,
                                                                      const ResultRange& rRange)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_FIND_COMMUNITY_BY_PARTICIPANT);
    _Type_uint32::Add(&message, rPrincipalID);
    _DDL_ResultRange::Add(&message, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pCommunities);
    }
    s_ProtoReturn[METHOD_FIND_COMMUNITY_BY_PARTICIPANT] = ProtoReturn_FindCommunityByParticipant;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::FindCommunityByParticipant_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, qList<PersistentGathering>* pCommunities,
    const unsigned int& rPrincipalID, const ResultRange& rRange)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_FIND_COMMUNITY_BY_PARTICIPANT);
    _Type_uint32::Add(pMessage, rPrincipalID);
    _DDL_ResultRange::Add(pMessage, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pCommunities);
    }
    s_ProtoReturn[METHOD_FIND_COMMUNITY_BY_PARTICIPANT] = ProtoReturn_FindCommunityByParticipant;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallUpdatePrivacySetting(ProtocolCallContext* pCallContext, const bool& rPublic,
                                                                const bool& rPublicParticipation)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_UPDATE_PRIVACY_SETTING);
    message << rPublic;
    message << rPublicParticipation;
    s_ProtoReturn[METHOD_UPDATE_PRIVACY_SETTING] = ProtoReturn_UpdatePrivacySetting;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::UpdatePrivacySetting_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                           Message* pMessage, const bool& rPublic,
                                                                           const bool& rPublicParticipation)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_UPDATE_PRIVACY_SETTING);
    *pMessage << rPublic;
    *pMessage << rPublicParticipation;
    s_ProtoReturn[METHOD_UPDATE_PRIVACY_SETTING] = ProtoReturn_UpdatePrivacySetting;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallGetMyBlackList(ProtocolCallContext* pCallContext,
                                                          qList<unsigned int>* pPrincipalIDs)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_MY_BLACK_LIST);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pPrincipalIDs);
    }
    s_ProtoReturn[METHOD_GET_MY_BLACK_LIST] = ProtoReturn_GetMyBlackList;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::GetMyBlackList_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                     Message* pMessage,
                                                                     qList<unsigned int>* pPrincipalIDs)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_GET_MY_BLACK_LIST);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pPrincipalIDs);
    }
    s_ProtoReturn[METHOD_GET_MY_BLACK_LIST] = ProtoReturn_GetMyBlackList;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallAddToBlackList(ProtocolCallContext* pCallContext,
                                                          const qList<unsigned int>& rPrincipalIDs)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_ADD_TO_BLACK_LIST);
    _Type_qlist<_Type_uint32>::Add(&message, rPrincipalIDs);
    s_ProtoReturn[METHOD_ADD_TO_BLACK_LIST] = ProtoReturn_AddToBlackList;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::AddToBlackList_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                     Message* pMessage,
                                                                     const qList<unsigned int>& rPrincipalIDs)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_ADD_TO_BLACK_LIST);
    _Type_qlist<_Type_uint32>::Add(pMessage, rPrincipalIDs);
    s_ProtoReturn[METHOD_ADD_TO_BLACK_LIST] = ProtoReturn_AddToBlackList;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallRemoveFromBlackList(ProtocolCallContext* pCallContext,
                                                               const qList<unsigned int>& rPrincipalIDs)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_REMOVE_FROM_BLACK_LIST);
    _Type_qlist<_Type_uint32>::Add(&message, rPrincipalIDs);
    s_ProtoReturn[METHOD_REMOVE_FROM_BLACK_LIST] = ProtoReturn_RemoveFromBlackList;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::RemoveFromBlackList_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                          Message* pMessage,
                                                                          const qList<unsigned int>& rPrincipalIDs)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_REMOVE_FROM_BLACK_LIST);
    _Type_qlist<_Type_uint32>::Add(pMessage, rPrincipalIDs);
    s_ProtoReturn[METHOD_REMOVE_FROM_BLACK_LIST] = ProtoReturn_RemoveFromBlackList;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallClearMyBlackList(ProtocolCallContext* pCallContext)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_CLEAR_MY_BLACK_LIST);
    s_ProtoReturn[METHOD_CLEAR_MY_BLACK_LIST] = ProtoReturn_ClearMyBlackList;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::ClearMyBlackList_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                       Message* pMessage)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_CLEAR_MY_BLACK_LIST);
    s_ProtoReturn[METHOD_CLEAR_MY_BLACK_LIST] = ProtoReturn_ClearMyBlackList;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallReportViolation(ProtocolCallContext* pCallContext,
                                                           const unsigned int& rPrincipalID, const String& rUserName,
                                                           const unsigned int& rViolationCode)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_REPORT_VIOLATION);
    _Type_uint32::Add(&message, rPrincipalID);
    _Type_string::Add(&message, rUserName);
    _Type_uint32::Add(&message, rViolationCode);
    s_ProtoReturn[METHOD_REPORT_VIOLATION] = ProtoReturn_ReportViolation;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::ReportViolation_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                      Message* pMessage,
                                                                      const unsigned int& rPrincipalID,
                                                                      const String& rUserName,
                                                                      const unsigned int& rViolationCode)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_REPORT_VIOLATION);
    _Type_uint32::Add(pMessage, rPrincipalID);
    _Type_string::Add(pMessage, rUserName);
    _Type_uint32::Add(pMessage, rViolationCode);
    s_ProtoReturn[METHOD_REPORT_VIOLATION] = ProtoReturn_ReportViolation;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallIsViolationUser(ProtocolCallContext* pCallContext, bool* pViolationUser,
                                                           unsigned int* pScore)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_IS_VIOLATION_USER);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pViolationUser);
        pCallContext->AddReturnValuePtr(pScore);
    }
    s_ProtoReturn[METHOD_IS_VIOLATION_USER] = ProtoReturn_IsViolationUser;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::IsViolationUser_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                      Message* pMessage, bool* pViolationUser,
                                                                      unsigned int* pScore)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_IS_VIOLATION_USER);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pViolationUser);
        pCallContext->AddReturnValuePtr(pScore);
    }
    s_ProtoReturn[METHOD_IS_VIOLATION_USER] = ProtoReturn_IsViolationUser;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallJoinMatchmakeSessionEx(ProtocolCallContext* pCallContext,
                                                                  const unsigned int& rGatheringID,
                                                                  const String& rMessage, const bool& rIgnoreBlockList,
                                                                  qVector<unsigned char>* pSessionKey,
                                                                  const unsigned short& rParticipationCount)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_JOIN_MATCHMAKE_SESSION_EX);
    _Type_uint32::Add(&message, rGatheringID);
    _Type_string::Add(&message, rMessage);
    message << rIgnoreBlockList;
    _Type_uint16::Add(&message, rParticipationCount);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSessionKey);
    }
    s_ProtoReturn[METHOD_JOIN_MATCHMAKE_SESSION_EX] = ProtoReturn_JoinMatchmakeSessionEx;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::JoinMatchmakeSessionEx_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const unsigned int& rGatheringID, const String& rMessage,
    const bool& rIgnoreBlockList, qVector<unsigned char>* pSessionKey, const unsigned short& rParticipationCount)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_JOIN_MATCHMAKE_SESSION_EX);
    _Type_uint32::Add(pMessage, rGatheringID);
    _Type_string::Add(pMessage, rMessage);
    *pMessage << rIgnoreBlockList;
    _Type_uint16::Add(pMessage, rParticipationCount);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSessionKey);
    }
    s_ProtoReturn[METHOD_JOIN_MATCHMAKE_SESSION_EX] = ProtoReturn_JoinMatchmakeSessionEx;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallGetSimplePlayingSession(ProtocolCallContext* pCallContext,
                                                                   const qList<unsigned int>& rPrincipalIDs,
                                                                   const bool& rIncludeLoginUser,
                                                                   qList<SimplePlayingSession>* pSimplePlayingSessions)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_SIMPLE_PLAYING_SESSION);
    _Type_qlist<_Type_uint32>::Add(&message, rPrincipalIDs);
    message << rIncludeLoginUser;
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSimplePlayingSessions);
    }
    s_ProtoReturn[METHOD_GET_SIMPLE_PLAYING_SESSION] = ProtoReturn_GetSimplePlayingSession;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::GetSimplePlayingSession_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const qList<unsigned int>& rPrincipalIDs,
    const bool& rIncludeLoginUser, qList<SimplePlayingSession>* pSimplePlayingSessions)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_GET_SIMPLE_PLAYING_SESSION);
    _Type_qlist<_Type_uint32>::Add(pMessage, rPrincipalIDs);
    *pMessage << rIncludeLoginUser;
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSimplePlayingSessions);
    }
    s_ProtoReturn[METHOD_GET_SIMPLE_PLAYING_SESSION] = ProtoReturn_GetSimplePlayingSession;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallGetSimpleCommunity(ProtocolCallContext* pCallContext,
                                                              const qList<unsigned int>& rGatheringIDs,
                                                              qList<SimpleCommunity>* pSimpleCommunities)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_SIMPLE_COMMUNITY);
    _Type_qlist<_Type_uint32>::Add(&message, rGatheringIDs);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSimpleCommunities);
    }
    s_ProtoReturn[METHOD_GET_SIMPLE_COMMUNITY] = ProtoReturn_GetSimpleCommunity;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::GetSimpleCommunity_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                         Message* pMessage,
                                                                         const qList<unsigned int>& rGatheringIDs,
                                                                         qList<SimpleCommunity>* pSimpleCommunities)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_GET_SIMPLE_COMMUNITY);
    _Type_qlist<_Type_uint32>::Add(pMessage, rGatheringIDs);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSimpleCommunities);
    }
    s_ProtoReturn[METHOD_GET_SIMPLE_COMMUNITY] = ProtoReturn_GetSimpleCommunity;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallAutoMatchmakeWithGatheringId_Postpone(
    ProtocolCallContext* pCallContext, const qList<unsigned int>& rGatheringIDs, const GatheringHolder& rGathering,
    GatheringHolder* pJoinedGathering, const String& rMessage)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_AUTO_MATCHMAKE_WITH_GATHERING_ID_POSTPONE);
    _Type_qlist<_Type_uint32>::Add(&message, rGatheringIDs);
    GatheringAny::Add(&message, rGathering);
    _Type_string::Add(&message, rMessage);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pJoinedGathering);
    }
    s_ProtoReturn[METHOD_AUTO_MATCHMAKE_WITH_GATHERING_ID_POSTPONE] = ProtoReturn_AutoMatchmakeWithGatheringId_Postpone;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::AutoMatchmakeWithGatheringId_Postpone_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const qList<unsigned int>& rGatheringIDs,
    const GatheringHolder& rGathering, GatheringHolder* pJoinedGathering, const String& rMessage)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_AUTO_MATCHMAKE_WITH_GATHERING_ID_POSTPONE);
    _Type_qlist<_Type_uint32>::Add(pMessage, rGatheringIDs);
    GatheringAny::Add(pMessage, rGathering);
    _Type_string::Add(pMessage, rMessage);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pJoinedGathering);
    }
    s_ProtoReturn[METHOD_AUTO_MATCHMAKE_WITH_GATHERING_ID_POSTPONE] = ProtoReturn_AutoMatchmakeWithGatheringId_Postpone;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallUpdateProgressScore(ProtocolCallContext* pCallContext,
                                                               const unsigned int& rGatheringID,
                                                               const unsigned char& rProgressScore)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_UPDATE_PROGRESS_SCORE);
    _Type_uint32::Add(&message, rGatheringID);
    _Type_byte::Add(&message, rProgressScore);
    s_ProtoReturn[METHOD_UPDATE_PROGRESS_SCORE] = ProtoReturn_UpdateProgressScore;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::UpdateProgressScore_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                          Message* pMessage,
                                                                          const unsigned int& rGatheringID,
                                                                          const unsigned char& rProgressScore)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_UPDATE_PROGRESS_SCORE);
    _Type_uint32::Add(pMessage, rGatheringID);
    _Type_byte::Add(pMessage, rProgressScore);
    s_ProtoReturn[METHOD_UPDATE_PROGRESS_SCORE] = ProtoReturn_UpdateProgressScore;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallDebugNotifyEvent(ProtocolCallContext* pCallContext,
                                                            const unsigned int& rPrincipalID,
                                                            const unsigned int& rMainType, const unsigned int& rSubType,
                                                            const unsigned int& rParam1, const unsigned int& rParam2,
                                                            const String& rParam3)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_DEBUG_NOTIFY_EVENT);
    _Type_uint32::Add(&message, rPrincipalID);
    _Type_uint32::Add(&message, rMainType);
    _Type_uint32::Add(&message, rSubType);
    _Type_uint32::Add(&message, rParam1);
    _Type_uint32::Add(&message, rParam2);
    _Type_string::Add(&message, rParam3);
    s_ProtoReturn[METHOD_DEBUG_NOTIFY_EVENT] = ProtoReturn_DebugNotifyEvent;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::DebugNotifyEvent_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const unsigned int& rPrincipalID,
    const unsigned int& rMainType, const unsigned int& rSubType, const unsigned int& rParam1,
    const unsigned int& rParam2, const String& rParam3)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_DEBUG_NOTIFY_EVENT);
    _Type_uint32::Add(pMessage, rPrincipalID);
    _Type_uint32::Add(pMessage, rMainType);
    _Type_uint32::Add(pMessage, rSubType);
    _Type_uint32::Add(pMessage, rParam1);
    _Type_uint32::Add(pMessage, rParam2);
    _Type_string::Add(pMessage, rParam3);
    s_ProtoReturn[METHOD_DEBUG_NOTIFY_EVENT] = ProtoReturn_DebugNotifyEvent;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallGenerateMatchmakeSessionSystemPassword(ProtocolCallContext* pCallContext,
                                                                                  const unsigned int& rGatheringID,
                                                                                  String* pPassword)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GENERATE_MATCHMAKE_SESSION_SYSTEM_PASSWORD);
    _Type_uint32::Add(&message, rGatheringID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pPassword);
    }
    s_ProtoReturn[METHOD_GENERATE_MATCHMAKE_SESSION_SYSTEM_PASSWORD] =
        ProtoReturn_GenerateMatchmakeSessionSystemPassword;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::GenerateMatchmakeSessionSystemPassword_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const unsigned int& rGatheringID, String* pPassword)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_GENERATE_MATCHMAKE_SESSION_SYSTEM_PASSWORD);
    _Type_uint32::Add(pMessage, rGatheringID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pPassword);
    }
    s_ProtoReturn[METHOD_GENERATE_MATCHMAKE_SESSION_SYSTEM_PASSWORD] =
        ProtoReturn_GenerateMatchmakeSessionSystemPassword;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallClearMatchmakeSessionSystemPassword(ProtocolCallContext* pCallContext,
                                                                               const unsigned int& rGatheringID)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_CLEAR_MATCHMAKE_SESSION_SYSTEM_PASSWORD);
    _Type_uint32::Add(&message, rGatheringID);
    s_ProtoReturn[METHOD_CLEAR_MATCHMAKE_SESSION_SYSTEM_PASSWORD] = ProtoReturn_ClearMatchmakeSessionSystemPassword;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::ClearMatchmakeSessionSystemPassword_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const unsigned int& rGatheringID)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_CLEAR_MATCHMAKE_SESSION_SYSTEM_PASSWORD);
    _Type_uint32::Add(pMessage, rGatheringID);
    s_ProtoReturn[METHOD_CLEAR_MATCHMAKE_SESSION_SYSTEM_PASSWORD] = ProtoReturn_ClearMatchmakeSessionSystemPassword;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallCreateMatchmakeSessionWithParam(ProtocolCallContext* pCallContext,
                                                                           const CreateMatchmakeSessionParam& rParam,
                                                                           MatchmakeSession* pMatchmakeSession)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_CREATE_MATCHMAKE_SESSION_WITH_PARAM);
    _DDL_CreateMatchmakeSessionParam::Add(&message, rParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pMatchmakeSession);
    }
    s_ProtoReturn[METHOD_CREATE_MATCHMAKE_SESSION_WITH_PARAM] = ProtoReturn_CreateMatchmakeSessionWithParam;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::CreateMatchmakeSessionWithParam_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const CreateMatchmakeSessionParam& rParam,
    MatchmakeSession* pMatchmakeSession)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_CREATE_MATCHMAKE_SESSION_WITH_PARAM);
    _DDL_CreateMatchmakeSessionParam::Add(pMessage, rParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pMatchmakeSession);
    }
    s_ProtoReturn[METHOD_CREATE_MATCHMAKE_SESSION_WITH_PARAM] = ProtoReturn_CreateMatchmakeSessionWithParam;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallJoinMatchmakeSessionWithParam(ProtocolCallContext* pCallContext,
                                                                         const JoinMatchmakeSessionParam& rParam,
                                                                         MatchmakeSession* pMatchmakeSession)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_JOIN_MATCHMAKE_SESSION_WITH_PARAM);
    _DDL_JoinMatchmakeSessionParam::Add(&message, rParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pMatchmakeSession);
    }
    s_ProtoReturn[METHOD_JOIN_MATCHMAKE_SESSION_WITH_PARAM] = ProtoReturn_JoinMatchmakeSessionWithParam;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::JoinMatchmakeSessionWithParam_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const JoinMatchmakeSessionParam& rParam,
    MatchmakeSession* pMatchmakeSession)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_JOIN_MATCHMAKE_SESSION_WITH_PARAM);
    _DDL_JoinMatchmakeSessionParam::Add(pMessage, rParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pMatchmakeSession);
    }
    s_ProtoReturn[METHOD_JOIN_MATCHMAKE_SESSION_WITH_PARAM] = ProtoReturn_JoinMatchmakeSessionWithParam;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallAutoMatchmakeWithParam_Postpone(ProtocolCallContext* pCallContext,
                                                                           const AutoMatchmakeParam& rParam,
                                                                           MatchmakeSession* pMatchmakeSession)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_AUTO_MATCHMAKE_WITH_PARAM_POSTPONE);
    _DDL_AutoMatchmakeParam::Add(&message, rParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pMatchmakeSession);
    }
    s_ProtoReturn[METHOD_AUTO_MATCHMAKE_WITH_PARAM_POSTPONE] = ProtoReturn_AutoMatchmakeWithParam_Postpone;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::AutoMatchmakeWithParam_Postpone_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const AutoMatchmakeParam& rParam,
    MatchmakeSession* pMatchmakeSession)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_AUTO_MATCHMAKE_WITH_PARAM_POSTPONE);
    _DDL_AutoMatchmakeParam::Add(pMessage, rParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pMatchmakeSession);
    }
    s_ProtoReturn[METHOD_AUTO_MATCHMAKE_WITH_PARAM_POSTPONE] = ProtoReturn_AutoMatchmakeWithParam_Postpone;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallFindMatchmakeSessionByGatheringIdDetail(ProtocolCallContext* pCallContext,
                                                                                   const unsigned int& rGatheringID,
                                                                                   MatchmakeSession* pMatchmakeSession)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_FIND_MATCHMAKE_SESSION_BY_GATHERING_ID_DETAIL);
    _Type_uint32::Add(&message, rGatheringID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pMatchmakeSession);
    }
    s_ProtoReturn[METHOD_FIND_MATCHMAKE_SESSION_BY_GATHERING_ID_DETAIL] =
        ProtoReturn_FindMatchmakeSessionByGatheringIdDetail;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::FindMatchmakeSessionByGatheringIdDetail_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const unsigned int& rGatheringID,
    MatchmakeSession* pMatchmakeSession)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_FIND_MATCHMAKE_SESSION_BY_GATHERING_ID_DETAIL);
    _Type_uint32::Add(pMessage, rGatheringID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pMatchmakeSession);
    }
    s_ProtoReturn[METHOD_FIND_MATCHMAKE_SESSION_BY_GATHERING_ID_DETAIL] =
        ProtoReturn_FindMatchmakeSessionByGatheringIdDetail;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallBrowseMatchmakeSessionNoHolder(
    ProtocolCallContext* pCallContext, const MatchmakeSessionSearchCriteria& rSearchCriteria, const ResultRange& rRange,
    qList<MatchmakeSession>* pMatchmakeSessions)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_BROWSE_MATCHMAKE_SESSION_NO_HOLDER);
    _DDL_MatchmakeSessionSearchCriteria::Add(&message, rSearchCriteria);
    _DDL_ResultRange::Add(&message, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pMatchmakeSessions);
    }
    s_ProtoReturn[METHOD_BROWSE_MATCHMAKE_SESSION_NO_HOLDER] = ProtoReturn_BrowseMatchmakeSessionNoHolder;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::BrowseMatchmakeSessionNoHolder_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const MatchmakeSessionSearchCriteria& rSearchCriteria,
    const ResultRange& rRange, qList<MatchmakeSession>* pMatchmakeSessions)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_BROWSE_MATCHMAKE_SESSION_NO_HOLDER);
    _DDL_MatchmakeSessionSearchCriteria::Add(pMessage, rSearchCriteria);
    _DDL_ResultRange::Add(pMessage, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pMatchmakeSessions);
    }
    s_ProtoReturn[METHOD_BROWSE_MATCHMAKE_SESSION_NO_HOLDER] = ProtoReturn_BrowseMatchmakeSessionNoHolder;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallBrowseMatchmakeSessionWithHostUrlsNoHolder(
    ProtocolCallContext* pCallContext, const MatchmakeSessionSearchCriteria& rSearchCriteria, const ResultRange& rRange,
    qList<MatchmakeSession>* pMatchmakeSessions, qList<GatheringURLs>* pGatheringURLs)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_BROWSE_MATCHMAKE_SESSION_WITH_HOST_URLS_NO_HOLDER);
    _DDL_MatchmakeSessionSearchCriteria::Add(&message, rSearchCriteria);
    _DDL_ResultRange::Add(&message, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pMatchmakeSessions);
        pCallContext->AddReturnValuePtr(pGatheringURLs);
    }
    s_ProtoReturn[METHOD_BROWSE_MATCHMAKE_SESSION_WITH_HOST_URLS_NO_HOLDER] =
        ProtoReturn_BrowseMatchmakeSessionWithHostUrlsNoHolder;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::BrowseMatchmakeSessionWithHostUrlsNoHolder_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const MatchmakeSessionSearchCriteria& rSearchCriteria,
    const ResultRange& rRange, qList<MatchmakeSession>* pMatchmakeSessions, qList<GatheringURLs>* pGatheringURLs)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_BROWSE_MATCHMAKE_SESSION_WITH_HOST_URLS_NO_HOLDER);
    _DDL_MatchmakeSessionSearchCriteria::Add(pMessage, rSearchCriteria);
    _DDL_ResultRange::Add(pMessage, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pMatchmakeSessions);
        pCallContext->AddReturnValuePtr(pGatheringURLs);
    }
    s_ProtoReturn[METHOD_BROWSE_MATCHMAKE_SESSION_WITH_HOST_URLS_NO_HOLDER] =
        ProtoReturn_BrowseMatchmakeSessionWithHostUrlsNoHolder;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallUpdateMatchmakeSessionPart(ProtocolCallContext* pCallContext,
                                                                      const UpdateMatchmakeSessionParam& rParam)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_UPDATE_MATCHMAKE_SESSION_PART);
    _DDL_UpdateMatchmakeSessionParam::Add(&message, rParam);
    s_ProtoReturn[METHOD_UPDATE_MATCHMAKE_SESSION_PART] = ProtoReturn_UpdateMatchmakeSessionPart;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::UpdateMatchmakeSessionPart_PrepareRequest(
    ProtocolCallContext* pCallContext, Message* pMessage, const UpdateMatchmakeSessionParam& rParam)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_UPDATE_MATCHMAKE_SESSION_PART);
    _DDL_UpdateMatchmakeSessionParam::Add(pMessage, rParam);
    s_ProtoReturn[METHOD_UPDATE_MATCHMAKE_SESSION_PART] = ProtoReturn_UpdateMatchmakeSessionPart;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallRequestMatchmaking(ProtocolCallContext* pCallContext,
                                                              const AutoMatchmakeParam& rParam,
                                                              unsigned long long* pRequestID)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_REQUEST_MATCHMAKING);
    _DDL_AutoMatchmakeParam::Add(&message, rParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pRequestID);
    }
    s_ProtoReturn[METHOD_REQUEST_MATCHMAKING] = ProtoReturn_RequestMatchmaking;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::RequestMatchmaking_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                         Message* pMessage,
                                                                         const AutoMatchmakeParam& rParam,
                                                                         unsigned long long* pRequestID)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_REQUEST_MATCHMAKING);
    _DDL_AutoMatchmakeParam::Add(pMessage, rParam);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pRequestID);
    }
    s_ProtoReturn[METHOD_REQUEST_MATCHMAKING] = ProtoReturn_RequestMatchmaking;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallWithdrawMatchmaking(ProtocolCallContext* pCallContext,
                                                               const unsigned long long& rRequestID)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_WITHDRAW_MATCHMAKING);
    _Type_uint64::Add(&message, rRequestID);
    s_ProtoReturn[METHOD_WITHDRAW_MATCHMAKING] = ProtoReturn_WithdrawMatchmaking;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::WithdrawMatchmaking_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                          Message* pMessage,
                                                                          const unsigned long long& rRequestID)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_WITHDRAW_MATCHMAKING);
    _Type_uint64::Add(pMessage, rRequestID);
    s_ProtoReturn[METHOD_WITHDRAW_MATCHMAKING] = ProtoReturn_WithdrawMatchmaking;
    return true;
}

bool MatchmakeExtensionProtocolClient::CallWithdrawMatchmakingAll(ProtocolCallContext* pCallContext)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_WITHDRAW_MATCHMAKING_ALL);
    s_ProtoReturn[METHOD_WITHDRAW_MATCHMAKING_ALL] = ProtoReturn_WithdrawMatchmakingAll;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchmakeExtensionProtocolClient::WithdrawMatchmakingAll_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                             Message* pMessage)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_WITHDRAW_MATCHMAKING_ALL);
    s_ProtoReturn[METHOD_WITHDRAW_MATCHMAKING_ALL] = ProtoReturn_WithdrawMatchmakingAll;
    return true;
}

void MatchmakeExtensionProtocolClient::ProtoReturn_CloseParticipation(Message*, ProtocolCallContext*)
{
}

void MatchmakeExtensionProtocolClient::ProtoReturn_OpenParticipation(Message*, ProtocolCallContext*)
{
}

void MatchmakeExtensionProtocolClient::ProtoReturn_AutoMatchmake_Postpone(Message* pMessage,
                                                                          ProtocolCallContext* pCallContext)
{
    GatheringHolder* pGathering = static_cast<GatheringHolder*>(pCallContext->GetReturnValuePtr(0));
    if (pGathering != 0)
    {
        GatheringAny::Extract(pMessage, pGathering);
    }
    else
    {
        GatheringHolder discarded[1];
        GatheringAny::Extract(pMessage, &discarded[0]);
    }
}

void MatchmakeExtensionProtocolClient::ProtoReturn_BrowseMatchmakeSession(Message* pMessage,
                                                                          ProtocolCallContext* pCallContext)
{
    void* pReturn = pCallContext->GetReturnValuePtr(0);
    if (pReturn != 0)
    {
        _Type_qlist<GatheringAny>::Extract(pMessage, static_cast<GatheringHolderList*>(pReturn));
        return;
    }
    GatheringHolderList discarded[1];
    _Type_qlist<GatheringAny>::Extract(pMessage, &discarded[0]);
}

void MatchmakeExtensionProtocolClient::ProtoReturn_BrowseMatchmakeSessionWithHostUrls(Message* pMessage,
                                                                                      ProtocolCallContext* pCallContext)
{
    GatheringHolderList* pFirstList = static_cast<GatheringHolderList*>(pCallContext->GetReturnValuePtr(0));
    if (pFirstList != 0)
    {
        _Type_qlist<GatheringAny>::Extract(pMessage, pFirstList);
        if (pMessage->HasError())
            return;
    }
    else
    {
        GatheringHolderList discarded[1];
        _Type_qlist<GatheringAny>::Extract(pMessage, &discarded[0]);
        if (pMessage->HasError())
            return;
    }
    qList<GatheringURLs>* pSecondList = static_cast<qList<GatheringURLs>*>(pCallContext->GetReturnValuePtr(1));
    if (pSecondList != 0)
    {
        _Type_qlist<DDLClassMetaType<GatheringURLs> >::Extract(pMessage, pSecondList);
        return;
    }
    qList<GatheringURLs> discarded[1];
    _Type_qlist<DDLClassMetaType<GatheringURLs> >::Extract(pMessage, &discarded[0]);
}

void MatchmakeExtensionProtocolClient::ProtoReturn_CreateMatchmakeSession(Message* pMessage,
                                                                          ProtocolCallContext* pCallContext)
{
    unsigned int uiDiscardedGatheringID;
    unsigned int* pGatheringID = static_cast<unsigned int*>(pCallContext->GetReturnValuePtr(0));
    unsigned int* pDestination = &uiDiscardedGatheringID;
    if (pGatheringID != 0)
        pDestination = pGatheringID;
    _Type_uint32::Extract(pMessage, pDestination);
    if (pMessage->HasError())
        return;
    qVector<unsigned char>* pBuffer = static_cast<qVector<unsigned char>*>(pCallContext->GetReturnValuePtr(1));
    if (pBuffer != 0)
    {
        _Type_qvector<_Type_byte>::Extract(pMessage, pBuffer);
        return;
    }
    qVector<unsigned char> discarded[1];
    _Type_qvector<_Type_byte>::Extract(pMessage, &discarded[0]);
}

void MatchmakeExtensionProtocolClient::ProtoReturn_JoinMatchmakeSession(Message* pMessage,
                                                                        ProtocolCallContext* pCallContext)
{
    qVector<unsigned char>* pBuffer = static_cast<qVector<unsigned char>*>(pCallContext->GetReturnValuePtr(0));
    if (pBuffer != 0)
    {
        _Type_qvector<_Type_byte>::Extract(pMessage, pBuffer);
        return;
    }
    qVector<unsigned char> discarded[1];
    _Type_qvector<_Type_byte>::Extract(pMessage, &discarded[0]);
}

void MatchmakeExtensionProtocolClient::ProtoReturn_ModifyCurrentGameAttribute(Message*, ProtocolCallContext*)
{
}

void MatchmakeExtensionProtocolClient::ProtoReturn_UpdateNotificationData(Message*, ProtocolCallContext*)
{
}

void MatchmakeExtensionProtocolClient::ProtoReturn_GetFriendNotificationData(Message* pMessage,
                                                                             ProtocolCallContext* pCallContext)
{
    void* pReturn = pCallContext->GetReturnValuePtr(0);
    if (pReturn != 0)
    {
        _Type_qlist<DDLClassMetaType<NotificationEvent> >::Extract(pMessage,
                                                                   static_cast<qList<NotificationEvent>*>(pReturn));
        return;
    }
    qList<NotificationEvent> discarded[1];
    _Type_qlist<DDLClassMetaType<NotificationEvent> >::Extract(pMessage, &discarded[0]);
}

void MatchmakeExtensionProtocolClient::ProtoReturn_UpdateApplicationBuffer(Message*, ProtocolCallContext*)
{
}

void MatchmakeExtensionProtocolClient::ProtoReturn_UpdateMatchmakeSessionAttribute(Message*, ProtocolCallContext*)
{
}

void MatchmakeExtensionProtocolClient::ProtoReturn_GetlstFriendNotificationData(Message* pMessage,
                                                                                ProtocolCallContext* pCallContext)
{
    void* pReturn = pCallContext->GetReturnValuePtr(0);
    if (pReturn != 0)
    {
        _Type_qlist<DDLClassMetaType<NotificationEvent> >::Extract(pMessage,
                                                                   static_cast<qList<NotificationEvent>*>(pReturn));
        return;
    }
    qList<NotificationEvent> discarded[1];
    _Type_qlist<DDLClassMetaType<NotificationEvent> >::Extract(pMessage, &discarded[0]);
}

void MatchmakeExtensionProtocolClient::ProtoReturn_UpdateMatchmakeSession(Message*, ProtocolCallContext*)
{
}

void MatchmakeExtensionProtocolClient::ProtoReturn_AutoMatchmakeWithSearchCriteria_Postpone(
    Message* pMessage, ProtocolCallContext* pCallContext)
{
    GatheringHolder* pGathering = static_cast<GatheringHolder*>(pCallContext->GetReturnValuePtr(0));
    if (pGathering != 0)
    {
        GatheringAny::Extract(pMessage, pGathering);
    }
    else
    {
        GatheringHolder discarded[1];
        GatheringAny::Extract(pMessage, &discarded[0]);
    }
}

void MatchmakeExtensionProtocolClient::ProtoReturn_GetPlayingSession(Message* pMessage,
                                                                     ProtocolCallContext* pCallContext)
{
    void* pReturn = pCallContext->GetReturnValuePtr(0);
    if (pReturn != 0)
    {
        _Type_qlist<DDLClassMetaType<PlayingSession> >::Extract(pMessage, static_cast<qList<PlayingSession>*>(pReturn));
        return;
    }
    qList<PlayingSession> discarded[1];
    _Type_qlist<DDLClassMetaType<PlayingSession> >::Extract(pMessage, &discarded[0]);
}

void MatchmakeExtensionProtocolClient::ProtoReturn_CreateCommunity(Message* pMessage, ProtocolCallContext* pCallContext)
{
    unsigned int uiDiscardedValue;
    unsigned int* pValue = static_cast<unsigned int*>(pCallContext->GetReturnValuePtr(0));
    unsigned int* pDestination = &uiDiscardedValue;
    if (pValue != 0)
        pDestination = pValue;
    _Type_uint32::Extract(pMessage, pDestination);
}

void MatchmakeExtensionProtocolClient::ProtoReturn_UpdateCommunity(Message*, ProtocolCallContext*)
{
}

void MatchmakeExtensionProtocolClient::ProtoReturn_JoinCommunity(Message*, ProtocolCallContext*)
{
}

void MatchmakeExtensionProtocolClient::ProtoReturn_FindCommunityByGatheringId(Message* pMessage,
                                                                              ProtocolCallContext* pCallContext)
{
    void* pReturn = pCallContext->GetReturnValuePtr(0);
    if (pReturn != 0)
    {
        _Type_qlist<DDLClassMetaType<PersistentGathering> >::Extract(pMessage,
                                                                     static_cast<qList<PersistentGathering>*>(pReturn));
        return;
    }
    qList<PersistentGathering> discarded[1];
    _Type_qlist<DDLClassMetaType<PersistentGathering> >::Extract(pMessage, &discarded[0]);
}

void MatchmakeExtensionProtocolClient::ProtoReturn_FindOfficialCommunity(Message* pMessage,
                                                                         ProtocolCallContext* pCallContext)
{
    void* pReturn = pCallContext->GetReturnValuePtr(0);
    if (pReturn != 0)
    {
        _Type_qlist<DDLClassMetaType<PersistentGathering> >::Extract(pMessage,
                                                                     static_cast<qList<PersistentGathering>*>(pReturn));
        return;
    }
    qList<PersistentGathering> discarded[1];
    _Type_qlist<DDLClassMetaType<PersistentGathering> >::Extract(pMessage, &discarded[0]);
}

void MatchmakeExtensionProtocolClient::ProtoReturn_FindCommunityByParticipant(Message* pMessage,
                                                                              ProtocolCallContext* pCallContext)
{
    void* pReturn = pCallContext->GetReturnValuePtr(0);
    if (pReturn != 0)
    {
        _Type_qlist<DDLClassMetaType<PersistentGathering> >::Extract(pMessage,
                                                                     static_cast<qList<PersistentGathering>*>(pReturn));
        return;
    }
    qList<PersistentGathering> discarded[1];
    _Type_qlist<DDLClassMetaType<PersistentGathering> >::Extract(pMessage, &discarded[0]);
}

void MatchmakeExtensionProtocolClient::ProtoReturn_UpdatePrivacySetting(Message*, ProtocolCallContext*)
{
}

void MatchmakeExtensionProtocolClient::ProtoReturn_GetMyBlackList(Message* pMessage, ProtocolCallContext* pCallContext)
{
    void* pReturn = pCallContext->GetReturnValuePtr(0);
    if (pReturn != 0)
    {
        _Type_qlist<_Type_uint32>::Extract(pMessage, static_cast<qList<unsigned int>*>(pReturn));
        return;
    }
    qList<unsigned int> discarded[1];
    _Type_qlist<_Type_uint32>::Extract(pMessage, &discarded[0]);
}

void MatchmakeExtensionProtocolClient::ProtoReturn_AddToBlackList(Message*, ProtocolCallContext*)
{
}

void MatchmakeExtensionProtocolClient::ProtoReturn_RemoveFromBlackList(Message*, ProtocolCallContext*)
{
}

void MatchmakeExtensionProtocolClient::ProtoReturn_ClearMyBlackList(Message*, ProtocolCallContext*)
{
}

void MatchmakeExtensionProtocolClient::ProtoReturn_ReportViolation(Message*, ProtocolCallContext*)
{
}

void MatchmakeExtensionProtocolClient::ProtoReturn_IsViolationUser(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedViolationUser;
    bool* pViolationUser = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pViolationDestination = &bDiscardedViolationUser;
    if (pViolationUser != 0)
        pViolationDestination = pViolationUser;
    *pMessage >> *pViolationDestination;
    if (!pMessage->HasError())
    {
        unsigned int uiDiscardedScore;
        unsigned int* pScore = static_cast<unsigned int*>(pCallContext->GetReturnValuePtr(1));
        unsigned int* pScoreDestination = &uiDiscardedScore;
        if (pScore != 0)
            pScoreDestination = pScore;
        _Type_uint32::Extract(pMessage, pScoreDestination);
    }
}

void MatchmakeExtensionProtocolClient::ProtoReturn_JoinMatchmakeSessionEx(Message* pMessage,
                                                                          ProtocolCallContext* pCallContext)
{
    qVector<unsigned char>* pBuffer = static_cast<qVector<unsigned char>*>(pCallContext->GetReturnValuePtr(0));
    if (pBuffer != 0)
    {
        _Type_qvector<_Type_byte>::Extract(pMessage, pBuffer);
        return;
    }
    qVector<unsigned char> discarded[1];
    _Type_qvector<_Type_byte>::Extract(pMessage, &discarded[0]);
}

void MatchmakeExtensionProtocolClient::ProtoReturn_GetSimplePlayingSession(Message* pMessage,
                                                                           ProtocolCallContext* pCallContext)
{
    void* pReturn = pCallContext->GetReturnValuePtr(0);
    if (pReturn != 0)
    {
        _Type_qlist<DDLClassMetaType<SimplePlayingSession> >::Extract(
            pMessage, static_cast<qList<SimplePlayingSession>*>(pReturn));
        return;
    }
    qList<SimplePlayingSession> discarded[1];
    _Type_qlist<DDLClassMetaType<SimplePlayingSession> >::Extract(pMessage, &discarded[0]);
}

void MatchmakeExtensionProtocolClient::ProtoReturn_GetSimpleCommunity(Message* pMessage,
                                                                      ProtocolCallContext* pCallContext)
{
    void* pReturn = pCallContext->GetReturnValuePtr(0);
    if (pReturn != 0)
    {
        _Type_qlist<DDLClassMetaType<SimpleCommunity> >::Extract(pMessage,
                                                                 static_cast<qList<SimpleCommunity>*>(pReturn));
        return;
    }
    qList<SimpleCommunity> discarded[1];
    _Type_qlist<DDLClassMetaType<SimpleCommunity> >::Extract(pMessage, &discarded[0]);
}

void MatchmakeExtensionProtocolClient::ProtoReturn_AutoMatchmakeWithGatheringId_Postpone(
    Message* pMessage, ProtocolCallContext* pCallContext)
{
    GatheringHolder* pGathering = static_cast<GatheringHolder*>(pCallContext->GetReturnValuePtr(0));
    if (pGathering != 0)
    {
        GatheringAny::Extract(pMessage, pGathering);
    }
    else
    {
        GatheringHolder discarded[1];
        GatheringAny::Extract(pMessage, &discarded[0]);
    }
}

void MatchmakeExtensionProtocolClient::ProtoReturn_UpdateProgressScore(Message*, ProtocolCallContext*)
{
}

void MatchmakeExtensionProtocolClient::ProtoReturn_DebugNotifyEvent(Message*, ProtocolCallContext*)
{
}

void MatchmakeExtensionProtocolClient::ProtoReturn_GenerateMatchmakeSessionSystemPassword(
    Message* pMessage, ProtocolCallContext* pCallContext)
{
    String* pString = static_cast<String*>(pCallContext->GetReturnValuePtr(0));
    if (pString != 0)
    {
        _Type_string::Extract(pMessage, pString);
        return;
    }
    String discarded[1];
    _Type_string::Extract(pMessage, &discarded[0]);
}

void MatchmakeExtensionProtocolClient::ProtoReturn_ClearMatchmakeSessionSystemPassword(Message*, ProtocolCallContext*)
{
}

void MatchmakeExtensionProtocolClient::ProtoReturn_CreateMatchmakeSessionWithParam(Message* pMessage,
                                                                                   ProtocolCallContext* pCallContext)
{
    MatchmakeSession* pMatchmakeSession = static_cast<MatchmakeSession*>(pCallContext->GetReturnValuePtr(0));
    if (pMatchmakeSession != 0)
    {
        _DDL_MatchmakeSession::Extract(pMessage, pMatchmakeSession);
        return;
    }
    MatchmakeSession discarded[1];
    _DDL_MatchmakeSession::Extract(pMessage, &discarded[0]);
}

void MatchmakeExtensionProtocolClient::ProtoReturn_JoinMatchmakeSessionWithParam(Message* pMessage,
                                                                                 ProtocolCallContext* pCallContext)
{
    MatchmakeSession* pMatchmakeSession = static_cast<MatchmakeSession*>(pCallContext->GetReturnValuePtr(0));
    if (pMatchmakeSession != 0)
    {
        _DDL_MatchmakeSession::Extract(pMessage, pMatchmakeSession);
        return;
    }
    MatchmakeSession discarded[1];
    _DDL_MatchmakeSession::Extract(pMessage, &discarded[0]);
}

void MatchmakeExtensionProtocolClient::ProtoReturn_AutoMatchmakeWithParam_Postpone(Message* pMessage,
                                                                                   ProtocolCallContext* pCallContext)
{
    MatchmakeSession* pMatchmakeSession = static_cast<MatchmakeSession*>(pCallContext->GetReturnValuePtr(0));
    if (pMatchmakeSession != 0)
    {
        _DDL_MatchmakeSession::Extract(pMessage, pMatchmakeSession);
        return;
    }
    MatchmakeSession discarded[1];
    _DDL_MatchmakeSession::Extract(pMessage, &discarded[0]);
}

void MatchmakeExtensionProtocolClient::ProtoReturn_FindMatchmakeSessionByGatheringIdDetail(
    Message* pMessage, ProtocolCallContext* pCallContext)
{
    MatchmakeSession* pMatchmakeSession = static_cast<MatchmakeSession*>(pCallContext->GetReturnValuePtr(0));
    if (pMatchmakeSession != 0)
    {
        _DDL_MatchmakeSession::Extract(pMessage, pMatchmakeSession);
        return;
    }
    MatchmakeSession discarded[1];
    _DDL_MatchmakeSession::Extract(pMessage, &discarded[0]);
}

void MatchmakeExtensionProtocolClient::ProtoReturn_BrowseMatchmakeSessionNoHolder(Message* pMessage,
                                                                                  ProtocolCallContext* pCallContext)
{
    void* pReturn = pCallContext->GetReturnValuePtr(0);
    if (pReturn != 0)
    {
        _Type_qlist<DDLClassMetaType<MatchmakeSession> >::Extract(pMessage,
                                                                  static_cast<qList<MatchmakeSession>*>(pReturn));
        return;
    }
    qList<MatchmakeSession> discarded[1];
    _Type_qlist<DDLClassMetaType<MatchmakeSession> >::Extract(pMessage, &discarded[0]);
}

void MatchmakeExtensionProtocolClient::ProtoReturn_BrowseMatchmakeSessionWithHostUrlsNoHolder(
    Message* pMessage, ProtocolCallContext* pCallContext)
{
    qList<MatchmakeSession>* pFirstList = static_cast<qList<MatchmakeSession>*>(pCallContext->GetReturnValuePtr(0));
    if (pFirstList != 0)
    {
        _Type_qlist<DDLClassMetaType<MatchmakeSession> >::Extract(pMessage, pFirstList);
        if (pMessage->HasError())
            return;
    }
    else
    {
        qList<MatchmakeSession> discarded[1];
        _Type_qlist<DDLClassMetaType<MatchmakeSession> >::Extract(pMessage, &discarded[0]);
        if (pMessage->HasError())
            return;
    }
    qList<GatheringURLs>* pSecondList = static_cast<qList<GatheringURLs>*>(pCallContext->GetReturnValuePtr(1));
    if (pSecondList != 0)
    {
        _Type_qlist<DDLClassMetaType<GatheringURLs> >::Extract(pMessage, pSecondList);
        return;
    }
    qList<GatheringURLs> discarded[1];
    _Type_qlist<DDLClassMetaType<GatheringURLs> >::Extract(pMessage, &discarded[0]);
}

void MatchmakeExtensionProtocolClient::ProtoReturn_UpdateMatchmakeSessionPart(Message*, ProtocolCallContext*)
{
}

void MatchmakeExtensionProtocolClient::ProtoReturn_RequestMatchmaking(Message* pMessage,
                                                                      ProtocolCallContext* pCallContext)
{
    unsigned long long uiDiscardedRequestID;
    unsigned long long* pRequestID = static_cast<unsigned long long*>(pCallContext->GetReturnValuePtr(0));
    unsigned long long* pDestination = &uiDiscardedRequestID;
    if (pRequestID != 0)
        pDestination = pRequestID;
    _Type_uint64::Extract(pMessage, pDestination);
}

void MatchmakeExtensionProtocolClient::ProtoReturn_WithdrawMatchmaking(Message*, ProtocolCallContext*)
{
}

void MatchmakeExtensionProtocolClient::ProtoReturn_WithdrawMatchmakingAll(Message*, ProtocolCallContext*)
{
}

}
}
