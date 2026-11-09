#include "nn/nex/_Proto_MatchmakeExtensionProtocolServer.hpp"

#include <stdint.h>

#include "nn/nex/AutoMatchmakeParam.hpp"
#include "nn/nex/CreateMatchmakeSessionParam.hpp"
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
#include "nn/nex/ResultRange.hpp"
#include "nn/nex/SimpleCommunity.hpp"
#include "nn/nex/SimplePlayingSession.hpp"
#include "nn/nex/UpdateMatchmakeSessionParam.hpp"
#include "nn/nex/_Type_qlist.hpp"

namespace nn
{
namespace nex
{

inline void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_CloseParticipation(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->CloseParticipation(uiGatheringID);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_CLOSE_PARTICIPATION);
}

inline void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_OpenParticipation(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->OpenParticipation(uiGatheringID);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_OPEN_PARTICIPATION);
}

inline void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_AutoMatchmake_Postpone(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    AnyObjectHolder<Gathering, String> gathering;
    _Type_any<DDLClassMetaType<Gathering>, _Type_string>::Extract(pRequest, &gathering);
    if (pRequest->HasError())
    {
        return;
    }
    AnyObjectHolder<Gathering, String> joinedGathering;
    String message;
    _Type_string::Extract(pRequest, &message);
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->AutoMatchmake_Postpone(gathering, &joinedGathering, message);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_AUTO_MATCHMAKE_POSTPONE);
    _Type_any<DDLClassMetaType<Gathering>, _Type_string>::Add(pResponse, joinedGathering);
}

inline void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_BrowseMatchmakeSession(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    MatchmakeSessionSearchCriteria searchCriteria;
    DDLClassMetaType<MatchmakeSessionSearchCriteria>::Extract(pRequest, &searchCriteria);
    if (pRequest->HasError())
    {
        return;
    }
    ResultRange range;
    DDLClassMetaType<ResultRange>::Extract(pRequest, &range);
    if (pRequest->HasError())
    {
        return;
    }
    qList<AnyObjectHolder<Gathering, String> > gatherings;
    pProtocol->BrowseMatchmakeSession(searchCriteria, range, &gatherings);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_BROWSE_MATCHMAKE_SESSION);
    _Type_qlist<_Type_any<DDLClassMetaType<Gathering>, _Type_string> >::Add(pResponse, gatherings);
}

inline void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_BrowseMatchmakeSessionWithHostUrls(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    MatchmakeSessionSearchCriteria searchCriteria;
    DDLClassMetaType<MatchmakeSessionSearchCriteria>::Extract(pRequest, &searchCriteria);
    if (pRequest->HasError())
    {
        return;
    }
    ResultRange range;
    DDLClassMetaType<ResultRange>::Extract(pRequest, &range);
    if (pRequest->HasError())
    {
        return;
    }
    qList<AnyObjectHolder<Gathering, String> > gatherings;
    qList<GatheringURLs> gatheringURLs;
    pProtocol->BrowseMatchmakeSessionWithHostUrls(searchCriteria, range, &gatherings, &gatheringURLs);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_BROWSE_MATCHMAKE_SESSION_WITH_HOST_URLS);
    _Type_qlist<_Type_any<DDLClassMetaType<Gathering>, _Type_string> >::Add(pResponse, gatherings);
    _Type_qlist<DDLClassMetaType<GatheringURLs> >::Add(pResponse, gatheringURLs);
}

inline void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_CreateMatchmakeSession(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    AnyObjectHolder<Gathering, String> gathering;
    _Type_any<DDLClassMetaType<Gathering>, _Type_string>::Extract(pRequest, &gathering);
    if (pRequest->HasError())
    {
        return;
    }
    String message;
    _Type_string::Extract(pRequest, &message);
    if (pRequest->HasError())
    {
        return;
    }
    qVector<unsigned char> sessionKey;
    unsigned short uiParticipationCount;
    _Type_uint16::Extract(pRequest, &uiParticipationCount);
    if (pRequest->HasError())
    {
        return;
    }
    unsigned int uiGatheringID;
    pProtocol->CreateMatchmakeSession(gathering, &uiGatheringID, message, &sessionKey, uiParticipationCount);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_CREATE_MATCHMAKE_SESSION);
    _Type_uint32::Add(pResponse, uiGatheringID);
    _Type_qvector<_Type_byte>::Add(pResponse, sessionKey);
}

inline void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_JoinMatchmakeSession(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    String message;
    _Type_string::Extract(pRequest, &message);
    if (pRequest->HasError())
    {
        return;
    }
    qVector<unsigned char> sessionKey;
    pProtocol->JoinMatchmakeSession(uiGatheringID, message, &sessionKey);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_JOIN_MATCHMAKE_SESSION);
    _Type_qvector<_Type_byte>::Add(pResponse, sessionKey);
}

inline void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_ModifyCurrentGameAttribute(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    unsigned int uiAttributeIndex;
    _Type_uint32::Extract(pRequest, &uiAttributeIndex);
    if (pRequest->HasError())
    {
        return;
    }
    unsigned int uiNewValue;
    _Type_uint32::Extract(pRequest, &uiNewValue);
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->ModifyCurrentGameAttribute(uiGatheringID, uiAttributeIndex, uiNewValue);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_MODIFY_CURRENT_GAME_ATTRIBUTE);
}

inline void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_UpdateNotificationData(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    unsigned int uiType;
    _Type_uint32::Extract(pRequest, &uiType);
    if (pRequest->HasError())
    {
        return;
    }
    unsigned int uiParam1;
    _Type_uint32::Extract(pRequest, &uiParam1);
    if (pRequest->HasError())
    {
        return;
    }
    unsigned int uiParam2;
    _Type_uint32::Extract(pRequest, &uiParam2);
    if (pRequest->HasError())
    {
        return;
    }
    String param3;
    _Type_string::Extract(pRequest, &param3);
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->UpdateNotificationData(uiType, uiParam1, uiParam2, param3);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_UPDATE_NOTIFICATION_DATA);
}

inline void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_GetFriendNotificationData(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    qList<NotificationEvent> notificationEvents;
    int iType;
    pRequest->ExtractPrimitive(&iType);
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->GetFriendNotificationData(&notificationEvents, iType);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GET_FRIEND_NOTIFICATION_DATA);
    _Type_qlist<DDLClassMetaType<NotificationEvent> >::Add(pResponse, notificationEvents);
}

inline void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_UpdateApplicationBuffer(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    qVector<unsigned char> applicationBuffer;
    _Type_qvector<_Type_byte>::Extract(pRequest, &applicationBuffer);
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->UpdateApplicationBuffer(uiGatheringID, applicationBuffer);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_UPDATE_APPLICATION_BUFFER);
}

inline void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_UpdateMatchmakeSessionAttribute(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    qVector<unsigned int> attributes;
    _Type_qvector<_Type_uint32>::Extract(pRequest, &attributes);
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->UpdateMatchmakeSessionAttribute(uiGatheringID, attributes);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_UPDATE_MATCHMAKE_SESSION_ATTRIBUTE);
}

inline void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_GetlstFriendNotificationData(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    qList<NotificationEvent> notificationEvents;
    qList<unsigned int> types;
    _Type_qlist<_Type_uint32>::Extract(pRequest, &types);
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->GetlstFriendNotificationData(&notificationEvents, types);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GETLST_FRIEND_NOTIFICATION_DATA);
    _Type_qlist<DDLClassMetaType<NotificationEvent> >::Add(pResponse, notificationEvents);
}

inline void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_UpdateMatchmakeSession(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    AnyObjectHolder<Gathering, String> gathering;
    _Type_any<DDLClassMetaType<Gathering>, _Type_string>::Extract(pRequest, &gathering);
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->UpdateMatchmakeSession(gathering);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_UPDATE_MATCHMAKE_SESSION);
}

inline void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_AutoMatchmakeWithSearchCriteria_Postpone(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    qList<MatchmakeSessionSearchCriteria> searchCriteria;
    _Type_qlist<DDLClassMetaType<MatchmakeSessionSearchCriteria> >::Extract(pRequest, &searchCriteria);
    if (pRequest->HasError())
    {
        return;
    }
    AnyObjectHolder<Gathering, String> gathering;
    _Type_any<DDLClassMetaType<Gathering>, _Type_string>::Extract(pRequest, &gathering);
    if (pRequest->HasError())
    {
        return;
    }
    AnyObjectHolder<Gathering, String> joinedGathering;
    String message;
    _Type_string::Extract(pRequest, &message);
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->AutoMatchmakeWithSearchCriteria_Postpone(searchCriteria, gathering, &joinedGathering, message);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_AUTO_MATCHMAKE_WITH_SEARCH_CRITERIA_POSTPONE);
    _Type_any<DDLClassMetaType<Gathering>, _Type_string>::Add(pResponse, joinedGathering);
}

void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_GetPlayingSession(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    qList<unsigned int> principalIDs;
    _Type_qlist<_Type_uint32>::Extract(pRequest, &principalIDs);
    if (pRequest->HasError())
    {
        return;
    }
    qList<PlayingSession> playingSessions;
    pProtocol->GetPlayingSession(principalIDs, &playingSessions);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GET_PLAYING_SESSION);
    _Type_qlist<DDLClassMetaType<PlayingSession> >::Add(pResponse, playingSessions);
}

void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_CreateCommunity(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    PersistentGathering community;
    DDLClassMetaType<PersistentGathering>::Extract(pRequest, &community);
    if (pRequest->HasError())
    {
        return;
    }
    String message;
    _Type_string::Extract(pRequest, &message);
    if (pRequest->HasError())
    {
        return;
    }
    unsigned int uiGatheringID;
    pProtocol->CreateCommunity(community, &uiGatheringID, message);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_CREATE_COMMUNITY);
    _Type_uint32::Add(pResponse, uiGatheringID);
}

void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_UpdateCommunity(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    PersistentGathering community;
    DDLClassMetaType<PersistentGathering>::Extract(pRequest, &community);
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->UpdateCommunity(community);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_UPDATE_COMMUNITY);
}

void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_JoinCommunity(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    String message;
    _Type_string::Extract(pRequest, &message);
    if (pRequest->HasError())
    {
        return;
    }
    String password;
    _Type_string::Extract(pRequest, &password);
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->JoinCommunity(uiGatheringID, message, password);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_JOIN_COMMUNITY);
}

void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_FindCommunityByGatheringId(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    qList<unsigned int> gatheringIDs;
    _Type_qlist<_Type_uint32>::Extract(pRequest, &gatheringIDs);
    if (pRequest->HasError())
    {
        return;
    }
    qList<PersistentGathering> communities;
    pProtocol->FindCommunityByGatheringId(gatheringIDs, &communities);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_FIND_COMMUNITY_BY_GATHERING_ID);
    _Type_qlist<DDLClassMetaType<PersistentGathering> >::Add(pResponse, communities);
}

void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_FindOfficialCommunity(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    qList<PersistentGathering> communities;
    bool bAvailableOnly;
    *pRequest >> bAvailableOnly;
    if (pRequest->HasError())
    {
        return;
    }
    ResultRange range;
    DDLClassMetaType<ResultRange>::Extract(pRequest, &range);
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->FindOfficialCommunity(&communities, bAvailableOnly, range);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_FIND_OFFICIAL_COMMUNITY);
    _Type_qlist<DDLClassMetaType<PersistentGathering> >::Add(pResponse, communities);
}

void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_FindCommunityByParticipant(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    qList<PersistentGathering> communities;
    unsigned int uiPrincipalID;
    _Type_uint32::Extract(pRequest, &uiPrincipalID);
    if (pRequest->HasError())
    {
        return;
    }
    ResultRange range;
    DDLClassMetaType<ResultRange>::Extract(pRequest, &range);
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->FindCommunityByParticipant(&communities, uiPrincipalID, range);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_FIND_COMMUNITY_BY_PARTICIPANT);
    _Type_qlist<DDLClassMetaType<PersistentGathering> >::Add(pResponse, communities);
}

inline void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_UpdatePrivacySetting(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    bool bPublic;
    *pRequest >> bPublic;
    if (pRequest->HasError())
    {
        return;
    }
    bool bPublicParticipation;
    *pRequest >> bPublicParticipation;
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->UpdatePrivacySetting(bPublic, bPublicParticipation);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_UPDATE_PRIVACY_SETTING);
}

void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_GetMyBlackList(
    Message*, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    qList<unsigned int> principalIDs;
    pProtocol->GetMyBlackList(&principalIDs);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GET_MY_BLACK_LIST);
    _Type_qlist<_Type_uint32>::Add(pResponse, principalIDs);
}

void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_AddToBlackList(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    qList<unsigned int> principalIDs;
    _Type_qlist<_Type_uint32>::Extract(pRequest, &principalIDs);
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->AddToBlackList(principalIDs);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_ADD_TO_BLACK_LIST);
}

void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_RemoveFromBlackList(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    qList<unsigned int> principalIDs;
    _Type_qlist<_Type_uint32>::Extract(pRequest, &principalIDs);
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->RemoveFromBlackList(principalIDs);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_REMOVE_FROM_BLACK_LIST);
}

void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_ClearMyBlackList(
    Message*, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    pProtocol->ClearMyBlackList();
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_CLEAR_MY_BLACK_LIST);
}

void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_ReportViolation(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    unsigned int uiPrincipalID;
    _Type_uint32::Extract(pRequest, &uiPrincipalID);
    if (pRequest->HasError())
    {
        return;
    }
    String userName;
    _Type_string::Extract(pRequest, &userName);
    if (pRequest->HasError())
    {
        return;
    }
    unsigned int uiViolationCode;
    _Type_uint32::Extract(pRequest, &uiViolationCode);
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->ReportViolation(uiPrincipalID, userName, uiViolationCode);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_REPORT_VIOLATION);
}

void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_IsViolationUser(
    Message*, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    bool bViolationUser;
    unsigned int uiScore;
    pProtocol->IsViolationUser(&bViolationUser, &uiScore);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_IS_VIOLATION_USER);
    *pResponse << bViolationUser;
    _Type_uint32::Add(pResponse, uiScore);
}

void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_JoinMatchmakeSessionEx(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    String message;
    _Type_string::Extract(pRequest, &message);
    if (pRequest->HasError())
    {
        return;
    }
    bool bIgnoreBlockList;
    *pRequest >> bIgnoreBlockList;
    if (pRequest->HasError())
    {
        return;
    }
    qVector<unsigned char> sessionKey;
    unsigned short uiParticipationCount;
    _Type_uint16::Extract(pRequest, &uiParticipationCount);
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->JoinMatchmakeSessionEx(uiGatheringID, message, bIgnoreBlockList, &sessionKey, uiParticipationCount);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_JOIN_MATCHMAKE_SESSION_EX);
    _Type_qvector<_Type_byte>::Add(pResponse, sessionKey);
}

void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_GetSimplePlayingSession(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    qList<unsigned int> principalIDs;
    _Type_qlist<_Type_uint32>::Extract(pRequest, &principalIDs);
    if (pRequest->HasError())
    {
        return;
    }
    bool bIncludeLoginUser;
    *pRequest >> bIncludeLoginUser;
    if (pRequest->HasError())
    {
        return;
    }
    qList<SimplePlayingSession> simplePlayingSessions;
    pProtocol->GetSimplePlayingSession(principalIDs, bIncludeLoginUser, &simplePlayingSessions);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GET_SIMPLE_PLAYING_SESSION);
    _Type_qlist<DDLClassMetaType<SimplePlayingSession> >::Add(pResponse, simplePlayingSessions);
}

void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_GetSimpleCommunity(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    qList<unsigned int> gatheringIDs;
    _Type_qlist<_Type_uint32>::Extract(pRequest, &gatheringIDs);
    if (pRequest->HasError())
    {
        return;
    }
    qList<SimpleCommunity> simpleCommunities;
    pProtocol->GetSimpleCommunity(gatheringIDs, &simpleCommunities);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GET_SIMPLE_COMMUNITY);
    _Type_qlist<DDLClassMetaType<SimpleCommunity> >::Add(pResponse, simpleCommunities);
}

void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_AutoMatchmakeWithGatheringId_Postpone(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    qList<unsigned int> gatheringIDs;
    _Type_qlist<_Type_uint32>::Extract(pRequest, &gatheringIDs);
    if (pRequest->HasError())
    {
        return;
    }
    AnyObjectHolder<Gathering, String> gathering;
    _Type_any<DDLClassMetaType<Gathering>, _Type_string>::Extract(pRequest, &gathering);
    if (pRequest->HasError())
    {
        return;
    }
    AnyObjectHolder<Gathering, String> joinedGathering;
    String message;
    _Type_string::Extract(pRequest, &message);
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->AutoMatchmakeWithGatheringId_Postpone(gatheringIDs, gathering, &joinedGathering, message);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_AUTO_MATCHMAKE_WITH_GATHERING_ID_POSTPONE);
    _Type_any<DDLClassMetaType<Gathering>, _Type_string>::Add(pResponse, joinedGathering);
}

inline void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_UpdateProgressScore(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    unsigned char ucProgressScore;
    _Type_byte::Extract(pRequest, &ucProgressScore);
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->UpdateProgressScore(uiGatheringID, ucProgressScore);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_UPDATE_PROGRESS_SCORE);
}

inline void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_DebugNotifyEvent(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    unsigned int uiPrincipalID;
    _Type_uint32::Extract(pRequest, &uiPrincipalID);
    if (pRequest->HasError())
    {
        return;
    }
    unsigned int uiMainType;
    _Type_uint32::Extract(pRequest, &uiMainType);
    if (pRequest->HasError())
    {
        return;
    }
    unsigned int uiSubType;
    _Type_uint32::Extract(pRequest, &uiSubType);
    if (pRequest->HasError())
    {
        return;
    }
    unsigned int uiParam1;
    _Type_uint32::Extract(pRequest, &uiParam1);
    if (pRequest->HasError())
    {
        return;
    }
    unsigned int uiParam2;
    _Type_uint32::Extract(pRequest, &uiParam2);
    if (pRequest->HasError())
    {
        return;
    }
    String param3;
    _Type_string::Extract(pRequest, &param3);
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->DebugNotifyEvent(uiPrincipalID, uiMainType, uiSubType, uiParam1, uiParam2, param3);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_DEBUG_NOTIFY_EVENT);
}

inline void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_GenerateMatchmakeSessionSystemPassword(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (!pRequest->HasError())
    {
        String password;
        pProtocol->GenerateMatchmakeSessionSystemPassword(uiGatheringID, &password);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GENERATE_MATCHMAKE_SESSION_SYSTEM_PASSWORD);
        _Type_string::Add(pResponse, password);
    }
}

void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_ClearMatchmakeSessionSystemPassword(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->ClearMatchmakeSessionSystemPassword(uiGatheringID);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_CLEAR_MATCHMAKE_SESSION_SYSTEM_PASSWORD);
}

inline void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_CreateMatchmakeSessionWithParam(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    CreateMatchmakeSessionParam param;
    DDLClassMetaType<CreateMatchmakeSessionParam>::Extract(pRequest, &param);
    if (!pRequest->HasError())
    {
        MatchmakeSession matchmakeSession;
        pProtocol->CreateMatchmakeSessionWithParam(param, &matchmakeSession);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_CREATE_MATCHMAKE_SESSION_WITH_PARAM);
        DDLClassMetaType<MatchmakeSession>::Add(pResponse, matchmakeSession);
    }
}

inline void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_JoinMatchmakeSessionWithParam(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    JoinMatchmakeSessionParam param;
    DDLClassMetaType<JoinMatchmakeSessionParam>::Extract(pRequest, &param);
    if (pRequest->HasError())
    {
        return;
    }
    MatchmakeSession matchmakeSession;
    pProtocol->JoinMatchmakeSessionWithParam(param, &matchmakeSession);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_JOIN_MATCHMAKE_SESSION_WITH_PARAM);
    DDLClassMetaType<MatchmakeSession>::Add(pResponse, matchmakeSession);
}

inline void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_AutoMatchmakeWithParam_Postpone(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    AutoMatchmakeParam param;
    DDLClassMetaType<AutoMatchmakeParam>::Extract(pRequest, &param);
    if (!pRequest->HasError())
    {
        MatchmakeSession matchmakeSession;
        pProtocol->AutoMatchmakeWithParam_Postpone(param, &matchmakeSession);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_AUTO_MATCHMAKE_WITH_PARAM_POSTPONE);
        DDLClassMetaType<MatchmakeSession>::Add(pResponse, matchmakeSession);
    }
}

void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_FindMatchmakeSessionByGatheringIdDetail(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    MatchmakeSession matchmakeSession;
    pProtocol->FindMatchmakeSessionByGatheringIdDetail(uiGatheringID, &matchmakeSession);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_FIND_MATCHMAKE_SESSION_BY_GATHERING_ID_DETAIL);
    DDLClassMetaType<MatchmakeSession>::Add(pResponse, matchmakeSession);
}

void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_BrowseMatchmakeSessionNoHolder(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    MatchmakeSessionSearchCriteria searchCriteria;
    DDLClassMetaType<MatchmakeSessionSearchCriteria>::Extract(pRequest, &searchCriteria);
    if (pRequest->HasError())
    {
        return;
    }
    ResultRange range;
    DDLClassMetaType<ResultRange>::Extract(pRequest, &range);
    if (pRequest->HasError())
    {
        return;
    }
    qList<MatchmakeSession> matchmakeSessions;
    pProtocol->BrowseMatchmakeSessionNoHolder(searchCriteria, range, &matchmakeSessions);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_BROWSE_MATCHMAKE_SESSION_NO_HOLDER);
    _Type_qlist<DDLClassMetaType<MatchmakeSession> >::Add(pResponse, matchmakeSessions);
}

void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_BrowseMatchmakeSessionWithHostUrlsNoHolder(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    MatchmakeSessionSearchCriteria searchCriteria;
    DDLClassMetaType<MatchmakeSessionSearchCriteria>::Extract(pRequest, &searchCriteria);
    if (pRequest->HasError())
    {
        return;
    }
    ResultRange range;
    DDLClassMetaType<ResultRange>::Extract(pRequest, &range);
    if (pRequest->HasError())
    {
        return;
    }
    qList<MatchmakeSession> matchmakeSessions;
    qList<GatheringURLs> gatheringURLs;
    pProtocol->BrowseMatchmakeSessionWithHostUrlsNoHolder(searchCriteria, range, &matchmakeSessions, &gatheringURLs);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_BROWSE_MATCHMAKE_SESSION_WITH_HOST_URLS_NO_HOLDER);
    _Type_qlist<DDLClassMetaType<MatchmakeSession> >::Add(pResponse, matchmakeSessions);
    _Type_qlist<DDLClassMetaType<GatheringURLs> >::Add(pResponse, gatheringURLs);
}

inline void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_UpdateMatchmakeSessionPart(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    UpdateMatchmakeSessionParam param;
    DDLClassMetaType<UpdateMatchmakeSessionParam>::Extract(pRequest, &param);
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->UpdateMatchmakeSessionPart(param);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_UPDATE_MATCHMAKE_SESSION_PART);
}

inline void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_RequestMatchmaking(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    AutoMatchmakeParam param;
    DDLClassMetaType<AutoMatchmakeParam>::Extract(pRequest, &param);
    if (!pRequest->HasError())
    {
        unsigned long long ullRequestID;
        pProtocol->RequestMatchmaking(param, &ullRequestID);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_REQUEST_MATCHMAKING);
        _Type_uint64::Add(pResponse, ullRequestID);
    }
}

void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_WithdrawMatchmaking(
    Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    unsigned long long ullRequestID;
    _Type_uint64::Extract(pRequest, &ullRequestID);
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->WithdrawMatchmaking(ullRequestID);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_WITHDRAW_MATCHMAKING);
}

void _Proto_MatchmakeExtensionProtocolServer::ProtoStub_WithdrawMatchmakingAll(
    Message*, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol)
{
    pProtocol->WithdrawMatchmakingAll();
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_WITHDRAW_MATCHMAKING_ALL);
}

void _Proto_MatchmakeExtensionProtocolServer::DispatchProtocolMessage(Message* pRequest, Message* pResponse,
                                                                      bool* pResponseRequired, EndPoint*)
{
    switch (static_cast<uint16_t>(ExtractMethodID(pRequest)))
    {
    case METHOD_CLOSE_PARTICIPATION:
        ProtoStub_CloseParticipation(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_OPEN_PARTICIPATION:
        ProtoStub_OpenParticipation(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_AUTO_MATCHMAKE_POSTPONE:
        ProtoStub_AutoMatchmake_Postpone(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_BROWSE_MATCHMAKE_SESSION:
        ProtoStub_BrowseMatchmakeSession(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_BROWSE_MATCHMAKE_SESSION_WITH_HOST_URLS:
        ProtoStub_BrowseMatchmakeSessionWithHostUrls(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_CREATE_MATCHMAKE_SESSION:
        ProtoStub_CreateMatchmakeSession(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_JOIN_MATCHMAKE_SESSION:
        ProtoStub_JoinMatchmakeSession(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_MODIFY_CURRENT_GAME_ATTRIBUTE:
        ProtoStub_ModifyCurrentGameAttribute(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_UPDATE_NOTIFICATION_DATA:
        ProtoStub_UpdateNotificationData(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_GET_FRIEND_NOTIFICATION_DATA:
        ProtoStub_GetFriendNotificationData(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_UPDATE_APPLICATION_BUFFER:
        ProtoStub_UpdateApplicationBuffer(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_UPDATE_MATCHMAKE_SESSION_ATTRIBUTE:
        ProtoStub_UpdateMatchmakeSessionAttribute(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_GETLST_FRIEND_NOTIFICATION_DATA:
        ProtoStub_GetlstFriendNotificationData(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_UPDATE_MATCHMAKE_SESSION:
        ProtoStub_UpdateMatchmakeSession(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_AUTO_MATCHMAKE_WITH_SEARCH_CRITERIA_POSTPONE:
        ProtoStub_AutoMatchmakeWithSearchCriteria_Postpone(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_GET_PLAYING_SESSION:
        ProtoStub_GetPlayingSession(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_CREATE_COMMUNITY:
        ProtoStub_CreateCommunity(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_UPDATE_COMMUNITY:
        ProtoStub_UpdateCommunity(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_JOIN_COMMUNITY:
        ProtoStub_JoinCommunity(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_FIND_COMMUNITY_BY_GATHERING_ID:
        ProtoStub_FindCommunityByGatheringId(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_FIND_OFFICIAL_COMMUNITY:
        ProtoStub_FindOfficialCommunity(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_FIND_COMMUNITY_BY_PARTICIPANT:
        ProtoStub_FindCommunityByParticipant(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_UPDATE_PRIVACY_SETTING:
        ProtoStub_UpdatePrivacySetting(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_GET_MY_BLACK_LIST:
        ProtoStub_GetMyBlackList(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_ADD_TO_BLACK_LIST:
        ProtoStub_AddToBlackList(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_REMOVE_FROM_BLACK_LIST:
        ProtoStub_RemoveFromBlackList(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_CLEAR_MY_BLACK_LIST:
        ProtoStub_ClearMyBlackList(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_REPORT_VIOLATION:
        ProtoStub_ReportViolation(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_IS_VIOLATION_USER:
        ProtoStub_IsViolationUser(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_JOIN_MATCHMAKE_SESSION_EX:
        ProtoStub_JoinMatchmakeSessionEx(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_GET_SIMPLE_PLAYING_SESSION:
        ProtoStub_GetSimplePlayingSession(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_GET_SIMPLE_COMMUNITY:
        ProtoStub_GetSimpleCommunity(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_AUTO_MATCHMAKE_WITH_GATHERING_ID_POSTPONE:
        ProtoStub_AutoMatchmakeWithGatheringId_Postpone(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_UPDATE_PROGRESS_SCORE:
        ProtoStub_UpdateProgressScore(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_DEBUG_NOTIFY_EVENT:
        ProtoStub_DebugNotifyEvent(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_GENERATE_MATCHMAKE_SESSION_SYSTEM_PASSWORD:
        ProtoStub_GenerateMatchmakeSessionSystemPassword(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_CLEAR_MATCHMAKE_SESSION_SYSTEM_PASSWORD:
        ProtoStub_ClearMatchmakeSessionSystemPassword(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_CREATE_MATCHMAKE_SESSION_WITH_PARAM:
        ProtoStub_CreateMatchmakeSessionWithParam(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_JOIN_MATCHMAKE_SESSION_WITH_PARAM:
        ProtoStub_JoinMatchmakeSessionWithParam(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_AUTO_MATCHMAKE_WITH_PARAM_POSTPONE:
        ProtoStub_AutoMatchmakeWithParam_Postpone(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_FIND_MATCHMAKE_SESSION_BY_GATHERING_ID_DETAIL:
        ProtoStub_FindMatchmakeSessionByGatheringIdDetail(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_BROWSE_MATCHMAKE_SESSION_NO_HOLDER:
        ProtoStub_BrowseMatchmakeSessionNoHolder(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_BROWSE_MATCHMAKE_SESSION_WITH_HOST_URLS_NO_HOLDER:
        ProtoStub_BrowseMatchmakeSessionWithHostUrlsNoHolder(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_UPDATE_MATCHMAKE_SESSION_PART:
        ProtoStub_UpdateMatchmakeSessionPart(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_REQUEST_MATCHMAKING:
        ProtoStub_RequestMatchmaking(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_WITHDRAW_MATCHMAKING:
        ProtoStub_WithdrawMatchmaking(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_WITHDRAW_MATCHMAKING_ALL:
        ProtoStub_WithdrawMatchmakingAll(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case 0:
    default:
        SetCallError(qResult(RESULT_NOT_IMPLEMENTED));
        return;
    }
}

}
}
