#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHMAKESESSION_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXMATCHMAKESESSION_HPP

#include <list>

#include "nn/nex/AnyObjectHolder.hpp"
#include "nn/nex/AutoMatchmakeParam.hpp"
#include "nn/nex/CreateMatchmakeSessionParam.hpp"
#include "nn/nex/Gathering.hpp"
#include "nn/nex/InetAddress.hpp"
#include "nn/nex/JoinMatchmakeSessionParam.hpp"
#include "nn/nex/MatchmakeParam.hpp"
#include "nn/nex/MatchmakeSelectionMethod.hpp"
#include "nn/nex/MatchmakeSession.hpp"
#include "nn/nex/MatchmakeSessionSearchCriteria.hpp"
#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/PersistentGathering.hpp"
#include "nn/nex/PlayingSession.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/ResultRange.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/UpdateMatchmakeSessionParam.hpp"
#include "nn/nex/qList.hpp"
#include "nn/nex/qVector.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/inet/NexSessionInfo.hpp"
#include "nn/pia/inet/NexSessionSearchCriteria.hpp"
#include "nn/pia/inet/SessionType.hpp"
#include "nn/pia/session/CommonMatchmakeSession.hpp"
#include "nn/pia/session/ISessionInfoList.hpp"

namespace nn
{
namespace nex
{

class MatchmakeExtensionClient;
class NgsBridgeInterface;

}
}

namespace nn
{
namespace pia
{
namespace session
{

class CreateSessionSetting;
class JoinSessionSetting;
class JoinRandomSessionSetting;
class UpdateSessionSetting;

}
}
}

namespace nn
{
namespace pia
{
namespace transport
{

class StationConnectionInfo;

}
}
}

namespace nn
{
namespace pia
{
namespace inet
{

class NexCommunityInfo;
class NexCreateCommunitySetting;
class NexJoinCommunitySetting;
class NexUpdateCommunitySetting;
class NexSessionSearchCriteriaOwner;
class NexSessionSearchCriteriaParticipant;
class NexSessionSearchCriteriaSessionId;

class NexMatchmakeSession : public session::CommonMatchmakeSession
{
public:
    NexMatchmakeSession();
    virtual ~NexMatchmakeSession();
    virtual void Cleanup();
    virtual Result RandomMatchmakeSession(nn::pia::common::CallContext* callContext);
    virtual bool WaitRandomMatchmakeSession(unsigned int* gatheringId, bool* found);
    virtual Result BrowseMatchmakeSession(nn::pia::common::CallContext* callContext);
    virtual bool WaitBrowseMatchmakeSession();
    virtual session::ISessionInfoList* GetBrowseMatchmakeSessionResult();
    virtual Result CreateMatchmakeSession(nn::pia::common::CallContext* callContext);
    virtual bool WaitCreateMatchmakeSession(unsigned int* gatheringId);
    virtual Result JoinMatchmakeSession(nn::pia::common::CallContext* callContext, unsigned int gatheringId);
    virtual bool WaitJoinMatchmakeSession();
    virtual Result LeaveMatchmakeSession(nn::pia::common::CallContext* callContext, unsigned int gatheringId);
    virtual bool WaitLeaveMatchmakeSession();
    virtual Result UnregisterMatchmakeSession(nn::pia::common::CallContext* callContext, unsigned int gatheringId);
    virtual bool WaitUnregisterMatchmakeSession();
    virtual Result OpenParticipate(nn::pia::common::CallContext* callContext, unsigned int gatheringId);
    virtual bool WaitOpenParticipate();
    virtual Result CloseParticipate(nn::pia::common::CallContext* callContext, unsigned int gatheringId);
    virtual bool WaitCloseParticipate();
    virtual Result GetStationConnectionInfo(nn::pia::common::CallContext* callContext, unsigned int stationId);
    virtual bool WaitGetStationConnectionInfo(nn::pia::transport::StationConnectionInfo* info);
    virtual Result RequestSessionInfo(nn::pia::common::CallContext* callContext, unsigned int gatheringId);
    virtual bool WaitRequestSessionInfo();
    virtual unsigned long long ConvertGatheringIdToPersistentGatheringCode(unsigned int gatheringId);
    virtual unsigned int ConvertPersistentGatheringCodeToGatheringId(unsigned long long code);
    virtual session::ICommunityInfoList* GetBrowsedCommunityResult();
    virtual Result UpdateMatchmakeSessionHost(unsigned int stationId);
    virtual void SetMatchmakeOwnerUniqueId(unsigned long long ownerId);
    virtual Result UpdateSelectionPriority(unsigned int gatheringId, unsigned char priority);
    virtual unsigned int IsUpdatableSelectionPriority(unsigned char priority);
    virtual const session::ISessionInfo* GetSessionInfo() const;
    virtual session::ISessionInfo* GetSessionInfo();
    virtual Result RandomMatchmakeSessionWithTeam(nn::pia::common::CallContext* callContext,
                                                  unsigned long long* teamOwnerIds, unsigned short teamCount,
                                                  unsigned int gatheringId);
    virtual bool WaitRandomMatchmakeSessionWithTeam(unsigned int* gatheringId, bool* found);
    virtual Result FindSessionByOwner(nn::pia::common::CallContext* callContext,
                                      const NexSessionSearchCriteriaOwner& criteria);
    virtual bool WaitFindSessionByOwner();
    virtual Result FindSessionByParticipant(nn::pia::common::CallContext* callContext,
                                            const NexSessionSearchCriteriaParticipant& criteria);
    virtual bool WaitFindSessionByParticipant();
    virtual Result FindSessionBySessionId(nn::pia::common::CallContext* callContext,
                                          const NexSessionSearchCriteriaSessionId& criteria);
    virtual bool WaitFindSessionBySessionId();
    virtual Result CreateMatchmakeSessionWithTeam(nn::pia::common::CallContext* callContext,
                                                  unsigned long long* teamOwnerIds, unsigned short teamCount,
                                                  unsigned int gatheringId);
    virtual bool WaitCreateMatchmakeSessionWithTeam(unsigned int* gatheringId);
    virtual Result JoinMatchmakeSessionWithTeam(nn::pia::common::CallContext* callContext, unsigned int gatheringId,
                                                unsigned long long* teamOwnerIds, unsigned short teamCount,
                                                unsigned int teamIndex);
    virtual bool WaitJoinMatchmakeSessionWithTeam();
    virtual Result UpdateSessionSetting(nn::pia::common::CallContext* callContext, unsigned int gatheringId);
    virtual bool WaitUpdateSessionSetting();
    virtual void SetMatchmakeCurrentParticipantNum(unsigned short participantNum);

    Result ClearSystemPassword(nn::pia::common::CallContext* callContext, unsigned int gatheringId);
    static SessionType ConvertMatchmakeSystemTypeToSessionType(nn::nex::MatchmakeSystemType systemType);
    static nn::nex::MatchmakeSelectionMethod ConvertSelectionMethodToMatchmakeSelectionMethod(SelectionMethod method);
    static nn::nex::MatchmakeSystemType ConvertSessionTypeToMatchmakeSystemType(SessionType sessionType);
    void CopyFoundPersistentGatheringToBrowsedCommunityInfoList(const nn::nex::PersistentGathering* gathering,
                                                                NexCommunityInfo* communityInfo);
    void CopyMatchmakeSessionToSessionInfo(const nn::nex::MatchmakeSession* session, NexSessionInfo* sessionInfo);
    Result CreateCommunity(nn::pia::common::CallContext* callContext);
    Result Finalize();
    Result FindCommunityByGatheringId(nn::pia::common::CallContext* callContext);
    Result FindCommunityByOwner(nn::pia::common::CallContext* callContext, unsigned long long ownerId,
                                unsigned int offset, unsigned int size);
    Result FindCommunityByParticipant(nn::pia::common::CallContext* callContext, unsigned long long participantId,
                                      unsigned int offset, unsigned int size);
    Result FindOfficialCommunity(nn::pia::common::CallContext* callContext, bool isAvailableOnly, unsigned int offset,
                                 unsigned int size);
    Result GenerateSystemPassword(nn::pia::common::CallContext* callContext, unsigned int gatheringId);
    unsigned int GetRandomMatchmakeScore() const;
    Result Initialize();
    unsigned char IsMatchmakeHost() const;
    unsigned char IsProcessingJoinMatchmakeSession() const;
    unsigned char IsRandomMatchmakeScoreAvailable() const;
    Result JoinCommunity(nn::pia::common::CallContext* callContext);
    Result MigrateMatchmakeSessionOwner(nn::pia::common::CallContext* callContext, unsigned int gatheringId,
                                        unsigned long long* candidateIds, unsigned short candidateCount);
    Result ModifyAttribute(nn::pia::common::CallContext* callContext, unsigned int gatheringId, unsigned int index,
                           unsigned int value);
    Result ReflectCommunityIdList(const unsigned int* idList, unsigned int count);
    Result ReflectCreateCommunitySetting(const NexCreateCommunitySetting* setting);
    Result ReflectCreateSetting(const nn::pia::session::CreateSessionSetting* setting, bool hostMigrationEnabled);
    Result ReflectJoinCommunitySetting(const NexJoinCommunitySetting* setting);
    Result ReflectJoinRandomSetting(const nn::pia::session::JoinRandomSessionSetting* setting, bool isTeam);
    Result ReflectJoinSetting(const nn::pia::session::JoinSessionSetting* setting);
    Result ReflectSettingToMatchmakeSession(const nn::pia::session::CreateSessionSetting* setting,
                                            bool hostMigrationEnabled);
    Result ReflectUpdateCommunitySetting(const NexUpdateCommunitySetting* setting);
    Result ReflectUpdateSetting(const nn::pia::session::UpdateSessionSetting* setting, bool isJointSession);
    void RegisterFindCommunityResult();
    void RegisterFindSessionResult(nn::pia::session::SessionSearchCriteria::SearchCriteriaKey keyType);
    Result RegisterSearchCriteria(const nn::pia::session::SessionSearchCriteria* criteria, unsigned char criteriaNum);
    Result RequestSessionInfoWithoutPermission(nn::pia::common::CallContext* callContext, unsigned int gatheringId);
    Result SetMatchmakeSessionKey(const unsigned char* key, unsigned int keySize);
    Result SetMatchmakeSessionKey(const nn::nex::qVector<unsigned char>& key);
    void SetTemporaryOwnerChangeEvent(unsigned int gatheringId, unsigned long long ownerId);
    void UnregisterSearchCriteria();
    Result UpdateApplicationBuffer(nn::pia::common::CallContext* callContext, unsigned int gatheringId,
                                   unsigned char* buffer, unsigned int size);
    Result UpdateAttributes(nn::pia::common::CallContext* callContext, unsigned int gatheringId,
                            unsigned int* attributes);
    Result UpdateCommunity(nn::pia::common::CallContext* callContext);
    bool WaitClearSystemPassword();
    bool WaitCreateCommunity();
    bool WaitFindCommunityByGatheringId();
    bool WaitFindCommunityByOwner();
    bool WaitFindCommunityByParticipant();
    bool WaitFindOfficialCommunity();
    bool WaitGenerateSystemPassword();
    bool WaitJoinCommunity();
    bool WaitLeaveMatchmakeSessionCompanion();
    bool WaitMigrateMatchmakeSessionOwner();
    bool WaitModifyAttribute();
    bool WaitRequestSessionInfoWithoutPermission();
    bool WaitUpdateApplicationBuffer();
    bool WaitUpdateAttributes();
    bool WaitUpdateCommunity();

private:
    friend class NexMatchJointSessionJob;
    friend class NexMatchMeshLayerController;

    typedef nn::nex::qList<nn::nex::MatchmakeSessionSearchCriteria> SearchCriteriaList;
    typedef std::list<nn::nex::ResultRange, nn::nex::MemAllocator<nn::nex::ResultRange> > ResultRangeList;
    typedef nn::nex::qList<unsigned int> IdList;

    unsigned char* m_matchmakeExtensionClientMemory;
    nn::nex::MatchmakeExtensionClient* m_ownMatchmakeExtensionClient;
    nn::nex::MatchmakeExtensionClient* m_matchmakeExtensionClient;
    nn::nex::NgsBridgeInterface* m_ngsBridge;
    nn::nex::MatchmakeSession* m_requestSession;
    SearchCriteriaList* m_searchCriteriaList;
    ResultRangeList* m_resultRangeList;
    nn::nex::qList<nn::nex::AnyObjectHolder<nn::nex::Gathering, nn::nex::String> >* m_gatheringList;
    session::ISessionInfoList* m_browseMatchmakeSessionResult;
    nn::nex::AnyObjectHolder<nn::nex::Gathering, nn::nex::String> m_gatheringHolder;
    nn::nex::MatchmakeSession* m_foundSession;
    nn::nex::MatchmakeParam* m_matchmakeParam;
    nn::nex::AutoMatchmakeParam* m_autoMatchmakeParam;
    nn::nex::CreateMatchmakeSessionParam* m_createMatchmakeSessionParam;
    nn::nex::JoinMatchmakeSessionParam* m_joinMatchmakeSessionParam;
    nn::nex::UpdateMatchmakeSessionParam* m_updateMatchmakeSessionParam;
    nn::nex::PersistentGathering* m_persistentGathering;
    nn::nex::qList<nn::nex::PersistentGathering>* m_persistentGatheringList;
    session::ICommunityInfoList* m_browsedCommunityInfoList;
    IdList* m_idList68;
    nn::nex::ResultRange* m_resultRange;
    nn::nex::qList<nn::nex::PlayingSession>* m_playingSessionList;
    nn::nex::qList<nn::nex::StationURL>* m_stationUrlList;
    nn::nex::String* m_string78;
    IdList* m_idList7c;
    nn::nex::InetAddress* m_inetAddress;
    unsigned int m_gatheringId;
    nn::nex::ProtocolCallContext m_protocolCallContext;
    nn::nex::ProtocolCallContext m_protocolCallContextB;
    nn::pia::common::CallContext* m_callContext;
    bool m_matchmakeHost;
    unsigned int m_field180;
    unsigned int m_field184;
    nn::pia::common::Time m_selectionPriorityUpdateTime;
    nn::pia::common::Time m_requestDeadline;
    NexSessionInfo m_sessionInfo;
    unsigned int m_randomMatchmakeScore;
    bool m_randomMatchmakeScoreAvailable;
    bool m_processingJoinMatchmakeSession;
    unsigned int m_temporaryOwnerChangeGatheringId;
    unsigned long long m_temporaryOwnerChangeOwnerId;
    unsigned char m_matchmakeSessionKey[32];
};

}
}
}

#endif
