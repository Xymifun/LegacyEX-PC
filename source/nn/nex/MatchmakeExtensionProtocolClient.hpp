#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_MATCHMAKEEXTENSIONPROTOCOLCLIENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_MATCHMAKEEXTENSIONPROTOCOLCLIENT_HPP

#include "nn/nex/AnyObjectHolder.hpp"
#include "nn/nex/ClientProtocol.hpp"
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
class Message;
class NotificationEvent;
class PersistentGathering;
class PlayingSession;
class ProtocolCallContext;
class ResultRange;
class SimpleCommunity;
class SimplePlayingSession;
class UpdateMatchmakeSessionParam;

class MatchmakeExtensionProtocolClient : public ClientProtocol
{
public:
    typedef AnyObjectHolder<Gathering, String> GatheringHolder;
    typedef qList<GatheringHolder> GatheringHolderList;

    explicit MatchmakeExtensionProtocolClient(unsigned short uiProtocolID = 0);

    virtual void ExtractCallSpecificResults(Message* pMessage, ProtocolCallContext* pCallContext);
    virtual Protocol* CreateResponder() const;

    bool CallCloseParticipation(ProtocolCallContext* pCallContext, const unsigned int& rGatheringID);
    bool CallOpenParticipation(ProtocolCallContext* pCallContext, const unsigned int& rGatheringID);
    bool CallAutoMatchmake_Postpone(ProtocolCallContext* pCallContext, const GatheringHolder& rGathering,
                                    GatheringHolder* pJoinedGathering, const String& rMessage);
    bool CallBrowseMatchmakeSession(ProtocolCallContext* pCallContext,
                                    const MatchmakeSessionSearchCriteria& rSearchCriteria, const ResultRange& rRange,
                                    GatheringHolderList* pGatherings);
    bool CallBrowseMatchmakeSessionWithHostUrls(ProtocolCallContext* pCallContext,
                                                const MatchmakeSessionSearchCriteria& rSearchCriteria,
                                                const ResultRange& rRange, GatheringHolderList* pGatherings,
                                                qList<GatheringURLs>* pGatheringURLs);
    bool CallCreateMatchmakeSession(ProtocolCallContext* pCallContext, const GatheringHolder& rGathering,
                                    unsigned int* pGatheringID, const String& rMessage,
                                    qVector<unsigned char>* pSessionKey, const unsigned short& rParticipationCount);
    bool CallJoinMatchmakeSession(ProtocolCallContext* pCallContext, const unsigned int& rGatheringID,
                                  const String& rMessage, qVector<unsigned char>* pSessionKey);
    bool CallModifyCurrentGameAttribute(ProtocolCallContext* pCallContext, const unsigned int& rGatheringID,
                                        const unsigned int& rAttributeIndex, const unsigned int& rNewValue);
    bool CallUpdateNotificationData(ProtocolCallContext* pCallContext, const unsigned int& rType,
                                    const unsigned int& rParam1, const unsigned int& rParam2, const String& rParam3);
    bool CallGetFriendNotificationData(ProtocolCallContext* pCallContext, qList<NotificationEvent>* pNotificationEvents,
                                       const int& rType);
    bool CallUpdateApplicationBuffer(ProtocolCallContext* pCallContext, const unsigned int& rGatheringID,
                                     const qVector<unsigned char>& rApplicationBuffer);
    bool CallUpdateMatchmakeSessionAttribute(ProtocolCallContext* pCallContext, const unsigned int& rGatheringID,
                                             const qVector<unsigned int>& rAttributes);
    bool CallGetlstFriendNotificationData(ProtocolCallContext* pCallContext,
                                          qList<NotificationEvent>* pNotificationEvents,
                                          const qList<unsigned int>& rTypes);
    bool CallUpdateMatchmakeSession(ProtocolCallContext* pCallContext, const GatheringHolder& rGathering);
    bool CallAutoMatchmakeWithSearchCriteria_Postpone(ProtocolCallContext* pCallContext,
                                                      const qList<MatchmakeSessionSearchCriteria>& rSearchCriteria,
                                                      const GatheringHolder& rGathering,
                                                      GatheringHolder* pJoinedGathering, const String& rMessage);
    bool CallGetPlayingSession(ProtocolCallContext* pCallContext, const qList<unsigned int>& rPrincipalIDs,
                               qList<PlayingSession>* pPlayingSessions);
    bool CallCreateCommunity(ProtocolCallContext* pCallContext, const PersistentGathering& rCommunity,
                             unsigned int* pGatheringID, const String& rMessage);
    bool CallUpdateCommunity(ProtocolCallContext* pCallContext, const PersistentGathering& rCommunity);
    bool CallJoinCommunity(ProtocolCallContext* pCallContext, const unsigned int& rGatheringID, const String& rMessage,
                           const String& rPassword);
    bool CallFindCommunityByGatheringId(ProtocolCallContext* pCallContext, const qList<unsigned int>& rGatheringIDs,
                                        qList<PersistentGathering>* pCommunities);
    bool CallFindOfficialCommunity(ProtocolCallContext* pCallContext, qList<PersistentGathering>* pCommunities,
                                   const bool& rAvailableOnly, const ResultRange& rRange);
    bool CallFindCommunityByParticipant(ProtocolCallContext* pCallContext, qList<PersistentGathering>* pCommunities,
                                        const unsigned int& rPrincipalID, const ResultRange& rRange);
    bool CallUpdatePrivacySetting(ProtocolCallContext* pCallContext, const bool& rPublic,
                                  const bool& rPublicParticipation);
    bool CallGetMyBlackList(ProtocolCallContext* pCallContext, qList<unsigned int>* pPrincipalIDs);
    bool CallAddToBlackList(ProtocolCallContext* pCallContext, const qList<unsigned int>& rPrincipalIDs);
    bool CallRemoveFromBlackList(ProtocolCallContext* pCallContext, const qList<unsigned int>& rPrincipalIDs);
    bool CallClearMyBlackList(ProtocolCallContext* pCallContext);
    bool CallReportViolation(ProtocolCallContext* pCallContext, const unsigned int& rPrincipalID,
                             const String& rUserName, const unsigned int& rViolationCode);
    bool CallIsViolationUser(ProtocolCallContext* pCallContext, bool* pViolationUser, unsigned int* pScore);
    bool CallJoinMatchmakeSessionEx(ProtocolCallContext* pCallContext, const unsigned int& rGatheringID,
                                    const String& rMessage, const bool& rIgnoreBlockList,
                                    qVector<unsigned char>* pSessionKey, const unsigned short& rParticipationCount);
    bool CallGetSimplePlayingSession(ProtocolCallContext* pCallContext, const qList<unsigned int>& rPrincipalIDs,
                                     const bool& rIncludeLoginUser,
                                     qList<SimplePlayingSession>* pSimplePlayingSessions);
    bool CallGetSimpleCommunity(ProtocolCallContext* pCallContext, const qList<unsigned int>& rGatheringIDs,
                                qList<SimpleCommunity>* pSimpleCommunities);
    bool CallAutoMatchmakeWithGatheringId_Postpone(ProtocolCallContext* pCallContext,
                                                   const qList<unsigned int>& rGatheringIDs,
                                                   const GatheringHolder& rGathering, GatheringHolder* pJoinedGathering,
                                                   const String& rMessage);
    bool CallUpdateProgressScore(ProtocolCallContext* pCallContext, const unsigned int& rGatheringID,
                                 const unsigned char& rProgressScore);
    bool CallDebugNotifyEvent(ProtocolCallContext* pCallContext, const unsigned int& rPrincipalID,
                              const unsigned int& rMainType, const unsigned int& rSubType, const unsigned int& rParam1,
                              const unsigned int& rParam2, const String& rParam3);
    bool CallGenerateMatchmakeSessionSystemPassword(ProtocolCallContext* pCallContext, const unsigned int& rGatheringID,
                                                    String* pPassword);
    bool CallClearMatchmakeSessionSystemPassword(ProtocolCallContext* pCallContext, const unsigned int& rGatheringID);
    bool CallCreateMatchmakeSessionWithParam(ProtocolCallContext* pCallContext,
                                             const CreateMatchmakeSessionParam& rParam,
                                             MatchmakeSession* pMatchmakeSession);
    bool CallJoinMatchmakeSessionWithParam(ProtocolCallContext* pCallContext, const JoinMatchmakeSessionParam& rParam,
                                           MatchmakeSession* pMatchmakeSession);
    bool CallAutoMatchmakeWithParam_Postpone(ProtocolCallContext* pCallContext, const AutoMatchmakeParam& rParam,
                                             MatchmakeSession* pMatchmakeSession);
    bool CallFindMatchmakeSessionByGatheringIdDetail(ProtocolCallContext* pCallContext,
                                                     const unsigned int& rGatheringID,
                                                     MatchmakeSession* pMatchmakeSession);
    bool CallBrowseMatchmakeSessionNoHolder(ProtocolCallContext* pCallContext,
                                            const MatchmakeSessionSearchCriteria& rSearchCriteria,
                                            const ResultRange& rRange, qList<MatchmakeSession>* pMatchmakeSessions);
    bool CallBrowseMatchmakeSessionWithHostUrlsNoHolder(ProtocolCallContext* pCallContext,
                                                        const MatchmakeSessionSearchCriteria& rSearchCriteria,
                                                        const ResultRange& rRange,
                                                        qList<MatchmakeSession>* pMatchmakeSessions,
                                                        qList<GatheringURLs>* pGatheringURLs);
    bool CallUpdateMatchmakeSessionPart(ProtocolCallContext* pCallContext, const UpdateMatchmakeSessionParam& rParam);
    bool CallRequestMatchmaking(ProtocolCallContext* pCallContext, const AutoMatchmakeParam& rParam,
                                unsigned long long* pRequestID);
    bool CallWithdrawMatchmaking(ProtocolCallContext* pCallContext, const unsigned long long& rRequestID);
    bool CallWithdrawMatchmakingAll(ProtocolCallContext* pCallContext);
    static bool CloseParticipation_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                  const unsigned int& rGatheringID);
    static bool OpenParticipation_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                 const unsigned int& rGatheringID);
    static bool AutoMatchmake_Postpone_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                      const GatheringHolder& rGathering,
                                                      GatheringHolder* pJoinedGathering, const String& rMessage);
    static bool BrowseMatchmakeSession_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                      const MatchmakeSessionSearchCriteria& rSearchCriteria,
                                                      const ResultRange& rRange, GatheringHolderList* pGatherings);
    static bool BrowseMatchmakeSessionWithHostUrls_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                  const MatchmakeSessionSearchCriteria& rSearchCriteria,
                                                                  const ResultRange& rRange,
                                                                  GatheringHolderList* pGatherings,
                                                                  qList<GatheringURLs>* pGatheringURLs);
    static bool CreateMatchmakeSession_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                      const GatheringHolder& rGathering, unsigned int* pGatheringID,
                                                      const String& rMessage, qVector<unsigned char>* pSessionKey,
                                                      const unsigned short& rParticipationCount);
    static bool JoinMatchmakeSession_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                    const unsigned int& rGatheringID, const String& rMessage,
                                                    qVector<unsigned char>* pSessionKey);
    static bool ModifyCurrentGameAttribute_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                          const unsigned int& rGatheringID,
                                                          const unsigned int& rAttributeIndex,
                                                          const unsigned int& rNewValue);
    static bool UpdateNotificationData_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                      const unsigned int& rType, const unsigned int& rParam1,
                                                      const unsigned int& rParam2, const String& rParam3);
    static bool GetFriendNotificationData_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                         qList<NotificationEvent>* pNotificationEvents,
                                                         const int& rType);
    static bool UpdateApplicationBuffer_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                       const unsigned int& rGatheringID,
                                                       const qVector<unsigned char>& rApplicationBuffer);
    static bool UpdateMatchmakeSessionAttribute_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                               const unsigned int& rGatheringID,
                                                               const qVector<unsigned int>& rAttributes);
    static bool GetlstFriendNotificationData_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                            qList<NotificationEvent>* pNotificationEvents,
                                                            const qList<unsigned int>& rTypes);
    static bool UpdateMatchmakeSession_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                      const GatheringHolder& rGathering);
    static bool AutoMatchmakeWithSearchCriteria_Postpone_PrepareRequest(
        ProtocolCallContext* pCallContext, Message* pMessage,
        const qList<MatchmakeSessionSearchCriteria>& rSearchCriteria, const GatheringHolder& rGathering,
        GatheringHolder* pJoinedGathering, const String& rMessage);
    static bool GetPlayingSession_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                 const qList<unsigned int>& rPrincipalIDs,
                                                 qList<PlayingSession>* pPlayingSessions);
    static bool CreateCommunity_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                               const PersistentGathering& rCommunity, unsigned int* pGatheringID,
                                               const String& rMessage);
    static bool UpdateCommunity_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                               const PersistentGathering& rCommunity);
    static bool JoinCommunity_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                             const unsigned int& rGatheringID, const String& rMessage,
                                             const String& rPassword);
    static bool FindCommunityByGatheringId_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                          const qList<unsigned int>& rGatheringIDs,
                                                          qList<PersistentGathering>* pCommunities);
    static bool FindOfficialCommunity_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                     qList<PersistentGathering>* pCommunities,
                                                     const bool& rAvailableOnly, const ResultRange& rRange);
    static bool FindCommunityByParticipant_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                          qList<PersistentGathering>* pCommunities,
                                                          const unsigned int& rPrincipalID, const ResultRange& rRange);
    static bool UpdatePrivacySetting_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                    const bool& rPublic, const bool& rPublicParticipation);
    static bool GetMyBlackList_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                              qList<unsigned int>* pPrincipalIDs);
    static bool AddToBlackList_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                              const qList<unsigned int>& rPrincipalIDs);
    static bool RemoveFromBlackList_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                   const qList<unsigned int>& rPrincipalIDs);
    static bool ClearMyBlackList_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage);
    static bool ReportViolation_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                               const unsigned int& rPrincipalID, const String& rUserName,
                                               const unsigned int& rViolationCode);
    static bool IsViolationUser_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                               bool* pViolationUser, unsigned int* pScore);
    static bool JoinMatchmakeSessionEx_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                      const unsigned int& rGatheringID, const String& rMessage,
                                                      const bool& rIgnoreBlockList, qVector<unsigned char>* pSessionKey,
                                                      const unsigned short& rParticipationCount);
    static bool GetSimplePlayingSession_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                       const qList<unsigned int>& rPrincipalIDs,
                                                       const bool& rIncludeLoginUser,
                                                       qList<SimplePlayingSession>* pSimplePlayingSessions);
    static bool GetSimpleCommunity_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                  const qList<unsigned int>& rGatheringIDs,
                                                  qList<SimpleCommunity>* pSimpleCommunities);
    static bool AutoMatchmakeWithGatheringId_Postpone_PrepareRequest(
        ProtocolCallContext* pCallContext, Message* pMessage, const qList<unsigned int>& rGatheringIDs,
        const GatheringHolder& rGathering, GatheringHolder* pJoinedGathering, const String& rMessage);
    static bool UpdateProgressScore_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                   const unsigned int& rGatheringID,
                                                   const unsigned char& rProgressScore);
    static bool DebugNotifyEvent_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                const unsigned int& rPrincipalID, const unsigned int& rMainType,
                                                const unsigned int& rSubType, const unsigned int& rParam1,
                                                const unsigned int& rParam2, const String& rParam3);
    static bool GenerateMatchmakeSessionSystemPassword_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                      Message* pMessage,
                                                                      const unsigned int& rGatheringID,
                                                                      String* pPassword);
    static bool ClearMatchmakeSessionSystemPassword_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                   const unsigned int& rGatheringID);
    static bool CreateMatchmakeSessionWithParam_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                               const CreateMatchmakeSessionParam& rParam,
                                                               MatchmakeSession* pMatchmakeSession);
    static bool JoinMatchmakeSessionWithParam_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                             const JoinMatchmakeSessionParam& rParam,
                                                             MatchmakeSession* pMatchmakeSession);
    static bool AutoMatchmakeWithParam_Postpone_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                               const AutoMatchmakeParam& rParam,
                                                               MatchmakeSession* pMatchmakeSession);
    static bool FindMatchmakeSessionByGatheringIdDetail_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                       Message* pMessage,
                                                                       const unsigned int& rGatheringID,
                                                                       MatchmakeSession* pMatchmakeSession);
    static bool BrowseMatchmakeSessionNoHolder_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                              const MatchmakeSessionSearchCriteria& rSearchCriteria,
                                                              const ResultRange& rRange,
                                                              qList<MatchmakeSession>* pMatchmakeSessions);
    static bool BrowseMatchmakeSessionWithHostUrlsNoHolder_PrepareRequest(
        ProtocolCallContext* pCallContext, Message* pMessage, const MatchmakeSessionSearchCriteria& rSearchCriteria,
        const ResultRange& rRange, qList<MatchmakeSession>* pMatchmakeSessions, qList<GatheringURLs>* pGatheringURLs);
    static bool UpdateMatchmakeSessionPart_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                          const UpdateMatchmakeSessionParam& rParam);
    static bool RequestMatchmaking_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                  const AutoMatchmakeParam& rParam, unsigned long long* pRequestID);
    static bool WithdrawMatchmaking_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                   const unsigned long long& rRequestID);
    static bool WithdrawMatchmakingAll_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage);
    static void ProtoReturn_CloseParticipation(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_OpenParticipation(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_AutoMatchmake_Postpone(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_BrowseMatchmakeSession(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_BrowseMatchmakeSessionWithHostUrls(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_CreateMatchmakeSession(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_JoinMatchmakeSession(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_ModifyCurrentGameAttribute(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_UpdateNotificationData(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_GetFriendNotificationData(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_UpdateApplicationBuffer(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_UpdateMatchmakeSessionAttribute(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_GetlstFriendNotificationData(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_UpdateMatchmakeSession(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_AutoMatchmakeWithSearchCriteria_Postpone(Message* pMessage,
                                                                     ProtocolCallContext* pCallContext);
    static void ProtoReturn_GetPlayingSession(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_CreateCommunity(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_UpdateCommunity(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_JoinCommunity(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_FindCommunityByGatheringId(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_FindOfficialCommunity(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_FindCommunityByParticipant(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_UpdatePrivacySetting(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_GetMyBlackList(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_AddToBlackList(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_RemoveFromBlackList(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_ClearMyBlackList(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_ReportViolation(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_IsViolationUser(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_JoinMatchmakeSessionEx(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_GetSimplePlayingSession(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_GetSimpleCommunity(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_AutoMatchmakeWithGatheringId_Postpone(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_UpdateProgressScore(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_DebugNotifyEvent(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_GenerateMatchmakeSessionSystemPassword(Message* pMessage,
                                                                   ProtocolCallContext* pCallContext);
    static void ProtoReturn_ClearMatchmakeSessionSystemPassword(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_CreateMatchmakeSessionWithParam(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_JoinMatchmakeSessionWithParam(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_AutoMatchmakeWithParam_Postpone(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_FindMatchmakeSessionByGatheringIdDetail(Message* pMessage,
                                                                    ProtocolCallContext* pCallContext);
    static void ProtoReturn_BrowseMatchmakeSessionNoHolder(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_BrowseMatchmakeSessionWithHostUrlsNoHolder(Message* pMessage,
                                                                       ProtocolCallContext* pCallContext);
    static void ProtoReturn_UpdateMatchmakeSessionPart(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_RequestMatchmaking(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_WithdrawMatchmaking(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_WithdrawMatchmakingAll(Message* pMessage, ProtocolCallContext* pCallContext);
};

}
}

#endif
