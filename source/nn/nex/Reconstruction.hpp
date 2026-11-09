#ifndef SOURCE_NN_NEX_RECONSTRUCTION_HPP
#define SOURCE_NN_NEX_RECONSTRUCTION_HPP

#include <stdint.h>

#include "nn/nex/BackEndServices.hpp"
#include "nn/nex/ConnectivityManager.hpp"
#include "nn/nex/ProviderSupport.hpp"
#include "nn/nex/RendezVous.hpp"
#include "nn/nex/SecureConnectionClient.hpp"
#include "nn/nex/ServiceClient.hpp"
#include "nn/nex/GlobalVariables.hpp"
#include "nn/nex/MatchmakeExtensionProtocolClient.hpp"
#include "nn/nex/NotificationEvent.hpp"
#include "nn/nex/NotificationEventManager.hpp"
#include "nn/nex/NgsFacade.hpp"
#include "nn/nex/NotificationEvents.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qList.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{
class AutoMatchmakeParam;
class CallContext;
class CreateMatchmakeSessionParam;
class Credentials;
class Gathering;
class JoinMatchmakeSessionParam;
class MatchmakeSession;
class MatchmakeSessionSearchCriteria;
class PersistentGathering;
class PlayingSession;
class ResultRange;
class UpdateMatchmakeSessionParam;

class MatchMakingClient : public ServiceClient
{
public:
    MatchMakingClient();
    virtual ~MatchMakingClient();

    virtual void UpdateProtocolsDefaultCredentials(Credentials* pCredentials);

    Gathering* FindPrototype(const String& rType) const;
    bool RegisterGathering(ProtocolCallContext* pCallContext, const AnyObjectHolder<Gathering, String>& rGathering);
    bool UnregisterGathering(ProtocolCallContext* pCallContext, unsigned int uiGatheringID);
    bool FindByID(ProtocolCallContext* pCallContext, unsigned int uiGatheringID,
                  AnyObjectHolder<Gathering, String>* pGathering);
    bool FindByOwner(ProtocolCallContext* pCallContext, unsigned int uiOwnerID, const ResultRange& rRange,
                     qList<AnyObjectHolder<Gathering, String> >* pGatherings);
    bool MigrateGatheringOwnership(ProtocolCallContext* pCallContext, unsigned int uiGatheringID,
                                   const qList<unsigned int>& rCandidateIDs, bool bParticipantsOnly);
    bool MigrateGatheringOwnership(ProtocolCallContext* pCallContext, unsigned int uiGatheringID,
                                   const qList<unsigned int>& rCandidateIDs);
    bool GetSessionURLs(ProtocolCallContext* pCallContext, unsigned int uiGatheringID, qList<StationURL>* pURLs);
    bool EndParticipation(ProtocolCallContext* pCallContext, unsigned int uiGatheringID, bool* pSuccess,
                          const String& strMessage);
    bool EndParticipation(ProtocolCallContext* pCallContext, unsigned int uiGatheringID, const String& strMessage);
    bool UpdateSessionHost(ProtocolCallContext* pCallContext, unsigned int uiGatheringID, bool bIsMigrateOwner);
    bool RegisterPrototype(Gathering* pPrototype);

protected:
    MatchMakingProtocolClient m_matchMakingProtocolClient;
    MatchMakingProtocolExtClient m_matchMakingProtocolExtClient;
};

class MatchmakeExtensionClient : public MatchMakingClient
{
public:
    MatchmakeExtensionClient();
    virtual ~MatchmakeExtensionClient();

    virtual bool Bind(Credentials* pCredentials);
    virtual bool Unbind();
    bool AutoMatchmake(ProtocolCallContext* pCallContext, const AutoMatchmakeParam& rParam,
                       MatchmakeSession* pJoinedSession);
    bool RequestMatchmaking(ProtocolCallContext* pCallContext, const AutoMatchmakeParam& rParam,
                            unsigned long long* pRequestID);
    bool DoMatchmaking(ProtocolCallContext* pCallContext, const AutoMatchmakeParam& rParam,
                       MatchmakeSession* pJoinedSession);
    bool BrowseMatchmakeSession(ProtocolCallContext* pCallContext,
                                const MatchmakeSessionSearchCriteria& rSearchCriteria, const ResultRange& rRange,
                                qList<AnyObjectHolder<Gathering, String> >* pGatherings);
    bool CreateMatchmakeSession(ProtocolCallContext* pCallContext, const CreateMatchmakeSessionParam& rParam,
                                MatchmakeSession* pCreatedSession);
    bool JoinMatchmakeSession(ProtocolCallContext* pCallContext, const JoinMatchmakeSessionParam& rParam,
                              MatchmakeSession* pJoinedSession);
    bool CloseParticipation(ProtocolCallContext* pCallContext, unsigned int uiGatheringID);
    bool OpenParticipation(ProtocolCallContext* pCallContext, unsigned int uiGatheringID);
    bool ModifyMatchmakeSessionAttribute(ProtocolCallContext* pCallContext, unsigned int uiGatheringID,
                                         unsigned int uiAttributeIndex, unsigned int uiNewValue);
    bool UpdateMatchmakeSessionAttribute(ProtocolCallContext* pCallContext, unsigned int uiGatheringID,
                                         unsigned int* pAttributes);
    bool UpdateProgressScore(ProtocolCallContext* pCallContext, unsigned int uiGatheringID,
                             unsigned char uiProgressScore);
    bool UpdateProgressScore(unsigned int uiGatheringID, unsigned char uiProgressScore);
    bool UpdateApplicationBuffer(ProtocolCallContext* pCallContext, unsigned int uiGatheringID,
                                 qVector<unsigned char> applicationBuffer);
    bool UpdateMatchmakeSession(ProtocolCallContext* pCallContext, const UpdateMatchmakeSessionParam& rParam);
    bool UpdateNotificationData(ProtocolCallContext* pCallContext, unsigned int uiType, unsigned int uiParam1,
                                unsigned int uiParam2, String strText);
    static void SetPersistentGatheringCodeKey(const String& rKey);
    static void ClearPersistentGatheringCodeKey();
    static unsigned long long GatheringIdToPersistentGatheringCode(unsigned int uiGatheringID);
    static unsigned int PersistentGatheringCodeToGatheringId(unsigned long long uiPersistentGatheringCode);
    static bool IsValidPersistentGatheringCode(unsigned long long uiPersistentGatheringCode);
    bool GetFriendNotificationData(ProtocolCallContext* pCallContext, qList<NotificationEvent>* pEvents,
                                   const qList<unsigned int>& rTypes);
    bool GetPlayingSession(ProtocolCallContext* pCallContext, const qList<unsigned int>& rPrincipalIDs,
                           qList<PlayingSession>* pPlayingSessions);
    bool CreatePersistentGathering(ProtocolCallContext* pCallContext, const PersistentGathering& rGathering,
                                   unsigned int* pGatheringID, const String& strMessage = L"");
    bool UpdatePersistentGathering(ProtocolCallContext* pCallContext, const PersistentGathering& rGathering);
    bool JoinPersistentGathering(ProtocolCallContext* pCallContext, unsigned int uiGatheringID,
                                 const String& strMessage, const String& strPassword);
    bool FindPersistentGatheringByGatheringId(ProtocolCallContext* pCallContext,
                                              const qList<unsigned int>& rGatheringIDs,
                                              qList<PersistentGathering>* pGatherings);
    bool FindOfficialPersistentGathering(ProtocolCallContext* pCallContext, bool bAvailableOnly,
                                         const ResultRange& rRange, qList<PersistentGathering>* pGatherings);
    bool FindPersistentGatheringByParticipant(ProtocolCallContext* pCallContext, unsigned int uiPrincipalID,
                                              const ResultRange& rRange, qList<PersistentGathering>* pGatherings);
    bool FindMatchmakeSessionByGatheringIdDetail(ProtocolCallContext* pCallContext, unsigned int uiGatheringID,
                                                 MatchmakeSession* pMatchmakeSession);
    bool GenerateMatchmakeSessionSystemPassword(ProtocolCallContext* pCallContext, unsigned int uiGatheringID,
                                                String* pPassword);
    bool ClearMatchmakeSessionSystemPassword(ProtocolCallContext* pCallContext, unsigned int uiGatheringID);

private:
    bool m_protocolRegistered;
    MatchmakeExtensionProtocolClient m_matchmakeExtensionProtocolClient;
    unsigned long long m_uiLastProgressScoreTime;
};

}
}

#endif
