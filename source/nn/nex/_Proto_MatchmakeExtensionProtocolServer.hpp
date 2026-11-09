#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__PROTO_MATCHMAKEEXTENSIONPROTOCOLSERVER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__PROTO_MATCHMAKEEXTENSIONPROTOCOLSERVER_HPP

#include "nn/nex/AnyObjectHolder.hpp"
#include "nn/nex/ServerProtocol.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qList.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class AutoMatchmakeParam;
class CreateMatchmakeSessionParam;
class Gathering;
class GatheringURLs;
class JoinMatchmakeSessionParam;
class MatchmakeSession;
class MatchmakeSessionSearchCriteria;
class NotificationEvent;
class PersistentGathering;
class PlayingSession;
class ResultRange;
class SimpleCommunity;
class SimplePlayingSession;
class UpdateMatchmakeSessionParam;

class _Proto_MatchmakeExtensionProtocolServer : public ServerProtocol
{
public:
    typedef AnyObjectHolder<Gathering, String> GatheringHolder;
    typedef qList<GatheringHolder> GatheringHolderList;

    enum _MethodID
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

    explicit _Proto_MatchmakeExtensionProtocolServer(unsigned int uiFlags) : ServerProtocol(uiFlags) {}

    virtual void DispatchProtocolMessage(Message* pRequest, Message* pResponse, bool* pResponseRequired,
                                         EndPoint* pEndPoint);
    virtual void CloseParticipation(const unsigned int& rGatheringID) = 0;
    virtual void OpenParticipation(const unsigned int& rGatheringID) = 0;
    virtual void AutoMatchmake_Postpone(const GatheringHolder& rGathering, GatheringHolder* pJoinedGathering,
                                        const String& rMessage) = 0;
    virtual void BrowseMatchmakeSession(const MatchmakeSessionSearchCriteria& rSearchCriteria,
                                        const ResultRange& rRange, GatheringHolderList* pGatherings) = 0;
    virtual void BrowseMatchmakeSessionWithHostUrls(const MatchmakeSessionSearchCriteria& rSearchCriteria,
                                                    const ResultRange& rRange, GatheringHolderList* pGatherings,
                                                    qList<GatheringURLs>* pGatheringURLs) = 0;
    virtual void CreateMatchmakeSession(const GatheringHolder& rGathering, unsigned int* pGatheringID,
                                        const String& rMessage, qVector<unsigned char>* pSessionKey,
                                        const unsigned short& rParticipationCount) = 0;
    virtual void JoinMatchmakeSession(const unsigned int& rGatheringID, const String& rMessage,
                                      qVector<unsigned char>* pSessionKey) = 0;
    virtual void ModifyCurrentGameAttribute(const unsigned int& rGatheringID, const unsigned int& rAttributeIndex,
                                            const unsigned int& rNewValue) = 0;
    virtual void UpdateNotificationData(const unsigned int& rType, const unsigned int& rParam1,
                                        const unsigned int& rParam2, const String& rParam3) = 0;
    virtual void GetFriendNotificationData(qList<NotificationEvent>* pNotificationEvents, const int& rType) = 0;
    virtual void UpdateApplicationBuffer(const unsigned int& rGatheringID,
                                         const qVector<unsigned char>& rApplicationBuffer) = 0;
    virtual void UpdateMatchmakeSessionAttribute(const unsigned int& rGatheringID,
                                                 const qVector<unsigned int>& rAttributes) = 0;
    virtual void GetlstFriendNotificationData(qList<NotificationEvent>* pNotificationEvents,
                                              const qList<unsigned int>& rTypes) = 0;
    virtual void UpdateMatchmakeSession(const GatheringHolder& rGathering) = 0;
    virtual void AutoMatchmakeWithSearchCriteria_Postpone(const qList<MatchmakeSessionSearchCriteria>& rSearchCriteria,
                                                          const GatheringHolder& rGathering,
                                                          GatheringHolder* pJoinedGathering,
                                                          const String& rMessage) = 0;
    virtual void GetPlayingSession(const qList<unsigned int>& rPrincipalIDs,
                                   qList<PlayingSession>* pPlayingSessions) = 0;
    virtual void CreateCommunity(const PersistentGathering& rCommunity, unsigned int* pGatheringID,
                                 const String& rMessage) = 0;
    virtual void UpdateCommunity(const PersistentGathering& rCommunity) = 0;
    virtual void JoinCommunity(const unsigned int& rGatheringID, const String& rMessage, const String& rPassword) = 0;
    virtual void FindCommunityByGatheringId(const qList<unsigned int>& rGatheringIDs,
                                            qList<PersistentGathering>* pCommunities) = 0;
    virtual void FindOfficialCommunity(qList<PersistentGathering>* pCommunities, const bool& rAvailableOnly,
                                       const ResultRange& rRange) = 0;
    virtual void FindCommunityByParticipant(qList<PersistentGathering>* pCommunities, const unsigned int& rPrincipalID,
                                            const ResultRange& rRange) = 0;
    virtual void UpdatePrivacySetting(const bool& rPublic, const bool& rPublicParticipation) = 0;
    virtual void GetMyBlackList(qList<unsigned int>* pPrincipalIDs) = 0;
    virtual void AddToBlackList(const qList<unsigned int>& rPrincipalIDs) = 0;
    virtual void RemoveFromBlackList(const qList<unsigned int>& rPrincipalIDs) = 0;
    virtual void ClearMyBlackList() = 0;
    virtual void ReportViolation(const unsigned int& rPrincipalID, const String& rUserName,
                                 const unsigned int& rViolationCode) = 0;
    virtual void IsViolationUser(bool* pViolationUser, unsigned int* pScore) = 0;
    virtual void JoinMatchmakeSessionEx(const unsigned int& rGatheringID, const String& rMessage,
                                        const bool& rIgnoreBlockList, qVector<unsigned char>* pSessionKey,
                                        const unsigned short& rParticipationCount) = 0;
    virtual void GetSimplePlayingSession(const qList<unsigned int>& rPrincipalIDs, const bool& rIncludeLoginUser,
                                         qList<SimplePlayingSession>* pSimplePlayingSessions) = 0;
    virtual void GetSimpleCommunity(const qList<unsigned int>& rGatheringIDs,
                                    qList<SimpleCommunity>* pSimpleCommunities) = 0;
    virtual void AutoMatchmakeWithGatheringId_Postpone(const qList<unsigned int>& rGatheringIDs,
                                                       const GatheringHolder& rGathering,
                                                       GatheringHolder* pJoinedGathering, const String& rMessage) = 0;
    virtual void UpdateProgressScore(const unsigned int& rGatheringID, const unsigned char& rProgressScore) = 0;
    virtual void DebugNotifyEvent(const unsigned int& rPrincipalID, const unsigned int& rMainType,
                                  const unsigned int& rSubType, const unsigned int& rParam1,
                                  const unsigned int& rParam2, const String& rParam3) = 0;
    virtual void GenerateMatchmakeSessionSystemPassword(const unsigned int& rGatheringID, String* pPassword) = 0;
    virtual void ClearMatchmakeSessionSystemPassword(const unsigned int& rGatheringID) = 0;
    virtual void CreateMatchmakeSessionWithParam(const CreateMatchmakeSessionParam& rParam,
                                                 MatchmakeSession* pMatchmakeSession) = 0;
    virtual void JoinMatchmakeSessionWithParam(const JoinMatchmakeSessionParam& rParam,
                                               MatchmakeSession* pMatchmakeSession) = 0;
    virtual void AutoMatchmakeWithParam_Postpone(const AutoMatchmakeParam& rParam,
                                                 MatchmakeSession* pMatchmakeSession) = 0;
    virtual void FindMatchmakeSessionByGatheringIdDetail(const unsigned int& rGatheringID,
                                                         MatchmakeSession* pMatchmakeSession) = 0;
    virtual void BrowseMatchmakeSessionNoHolder(const MatchmakeSessionSearchCriteria& rSearchCriteria,
                                                const ResultRange& rRange,
                                                qList<MatchmakeSession>* pMatchmakeSessions) = 0;
    virtual void BrowseMatchmakeSessionWithHostUrlsNoHolder(const MatchmakeSessionSearchCriteria& rSearchCriteria,
                                                            const ResultRange& rRange,
                                                            qList<MatchmakeSession>* pMatchmakeSessions,
                                                            qList<GatheringURLs>* pGatheringURLs) = 0;
    virtual void UpdateMatchmakeSessionPart(const UpdateMatchmakeSessionParam& rParam) = 0;
    virtual void RequestMatchmaking(const AutoMatchmakeParam& rParam, unsigned long long* pRequestID) = 0;
    virtual void WithdrawMatchmaking(const unsigned long long& rRequestID) = 0;
    virtual void WithdrawMatchmakingAll() = 0;

private:
    enum
    {
        RESULT_NOT_IMPLEMENTED = static_cast<int>(0x80010002u),
        RESPONSE_METHOD_FLAG = 0x8000
    };

    static void ProtoStub_CloseParticipation(Message* pRequest, Message* pResponse,
                                             _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_OpenParticipation(Message* pRequest, Message* pResponse,
                                            _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_AutoMatchmake_Postpone(Message* pRequest, Message* pResponse,
                                                 _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_BrowseMatchmakeSession(Message* pRequest, Message* pResponse,
                                                 _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_BrowseMatchmakeSessionWithHostUrls(Message* pRequest, Message* pResponse,
                                                             _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_CreateMatchmakeSession(Message* pRequest, Message* pResponse,
                                                 _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_JoinMatchmakeSession(Message* pRequest, Message* pResponse,
                                               _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_ModifyCurrentGameAttribute(Message* pRequest, Message* pResponse,
                                                     _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_UpdateNotificationData(Message* pRequest, Message* pResponse,
                                                 _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_GetFriendNotificationData(Message* pRequest, Message* pResponse,
                                                    _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_UpdateApplicationBuffer(Message* pRequest, Message* pResponse,
                                                  _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_UpdateMatchmakeSessionAttribute(Message* pRequest, Message* pResponse,
                                                          _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_GetlstFriendNotificationData(Message* pRequest, Message* pResponse,
                                                       _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_UpdateMatchmakeSession(Message* pRequest, Message* pResponse,
                                                 _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_AutoMatchmakeWithSearchCriteria_Postpone(Message* pRequest, Message* pResponse,
                                                                   _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_GetPlayingSession(Message* pRequest, Message* pResponse,
                                            _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_CreateCommunity(Message* pRequest, Message* pResponse,
                                          _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_UpdateCommunity(Message* pRequest, Message* pResponse,
                                          _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_JoinCommunity(Message* pRequest, Message* pResponse,
                                        _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_FindCommunityByGatheringId(Message* pRequest, Message* pResponse,
                                                     _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_FindOfficialCommunity(Message* pRequest, Message* pResponse,
                                                _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_FindCommunityByParticipant(Message* pRequest, Message* pResponse,
                                                     _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_UpdatePrivacySetting(Message* pRequest, Message* pResponse,
                                               _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_GetMyBlackList(Message* pRequest, Message* pResponse,
                                         _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_AddToBlackList(Message* pRequest, Message* pResponse,
                                         _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_RemoveFromBlackList(Message* pRequest, Message* pResponse,
                                              _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_ClearMyBlackList(Message* pRequest, Message* pResponse,
                                           _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_ReportViolation(Message* pRequest, Message* pResponse,
                                          _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_IsViolationUser(Message* pRequest, Message* pResponse,
                                          _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_JoinMatchmakeSessionEx(Message* pRequest, Message* pResponse,
                                                 _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_GetSimplePlayingSession(Message* pRequest, Message* pResponse,
                                                  _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_GetSimpleCommunity(Message* pRequest, Message* pResponse,
                                             _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_AutoMatchmakeWithGatheringId_Postpone(Message* pRequest, Message* pResponse,
                                                                _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_UpdateProgressScore(Message* pRequest, Message* pResponse,
                                              _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_DebugNotifyEvent(Message* pRequest, Message* pResponse,
                                           _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_GenerateMatchmakeSessionSystemPassword(Message* pRequest, Message* pResponse,
                                                                 _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_ClearMatchmakeSessionSystemPassword(Message* pRequest, Message* pResponse,
                                                              _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_CreateMatchmakeSessionWithParam(Message* pRequest, Message* pResponse,
                                                          _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_JoinMatchmakeSessionWithParam(Message* pRequest, Message* pResponse,
                                                        _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_AutoMatchmakeWithParam_Postpone(Message* pRequest, Message* pResponse,
                                                          _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_FindMatchmakeSessionByGatheringIdDetail(Message* pRequest, Message* pResponse,
                                                                  _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_BrowseMatchmakeSessionNoHolder(Message* pRequest, Message* pResponse,
                                                         _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_BrowseMatchmakeSessionWithHostUrlsNoHolder(
        Message* pRequest, Message* pResponse, _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_UpdateMatchmakeSessionPart(Message* pRequest, Message* pResponse,
                                                     _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_RequestMatchmaking(Message* pRequest, Message* pResponse,
                                             _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_WithdrawMatchmaking(Message* pRequest, Message* pResponse,
                                              _Proto_MatchmakeExtensionProtocolServer* pProtocol);
    static void ProtoStub_WithdrawMatchmakingAll(Message* pRequest, Message* pResponse,
                                                 _Proto_MatchmakeExtensionProtocolServer* pProtocol);
};

}
}

#endif
