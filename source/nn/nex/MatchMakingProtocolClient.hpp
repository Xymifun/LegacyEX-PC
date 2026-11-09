#ifndef SOURCE_NN_NEX_MATCHMAKINGPROTOCOLCLIENT_HPP
#define SOURCE_NN_NEX_MATCHMAKINGPROTOCOLCLIENT_HPP

#include "nn/nex/AnyObjectHolder.hpp"
#include "nn/nex/ClientProtocol.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class DeletionEntry;
class Gathering;
class GatheringStats;
class Invitation;
class ParticipantDetails;
class ResultRange;
class StationURL;

class MatchMakingProtocolClient : public ClientProtocol
{
public:
    typedef AnyObjectHolder<Gathering, String> GatheringHolder;
    typedef qList<GatheringHolder> GatheringHolderList;

    explicit MatchMakingProtocolClient(unsigned short protocol_id = 0);

    virtual void ExtractCallSpecificResults(Message* pMessage, ProtocolCallContext* pCallContext);
    virtual Protocol* CreateResponder() const;

    bool CallRegisterGathering(ProtocolCallContext* pCallContext, unsigned int* pGatheringID,
                               const GatheringHolder& rGathering);
    bool CallUnregisterGathering(ProtocolCallContext* pCallContext, bool* pSuccess, const unsigned int& rGatheringID);
    bool CallUnregisterGatherings(ProtocolCallContext* pCallContext, bool* pSuccess,
                                  const qList<unsigned int>& rGatheringIDs);
    bool CallUpdateGathering(ProtocolCallContext* pCallContext, bool* pSuccess, const GatheringHolder& rGathering);
    bool CallInvite(ProtocolCallContext* pCallContext, bool* pSuccess, const unsigned int& rGatheringID,
                    const qList<unsigned int>& rPrincipalIDs, const String& rMessage);
    bool CallAcceptInvitation(ProtocolCallContext* pCallContext, bool* pSuccess, const unsigned int& rGatheringID,
                              const String& rMessage);
    bool CallDeclineInvitation(ProtocolCallContext* pCallContext, bool* pSuccess, const unsigned int& rGatheringID,
                               const String& rMessage);
    bool CallCancelInvitation(ProtocolCallContext* pCallContext, bool* pSuccess, const unsigned int& rGatheringID,
                              const qList<unsigned int>& rPrincipalIDs, const String& rMessage);
    bool CallGetInvitationsSent(ProtocolCallContext* pCallContext, const unsigned int& rGatheringID,
                                qList<Invitation>* pInvitations);
    bool CallGetInvitationsReceived(ProtocolCallContext* pCallContext, qList<Invitation>* pInvitations);
    bool CallParticipate(ProtocolCallContext* pCallContext, bool* pSuccess, const unsigned int& rGatheringID,
                         const String& rMessage);
    bool CallCancelParticipation(ProtocolCallContext* pCallContext, bool* pSuccess, const unsigned int& rGatheringID,
                                 const String& rMessage);
    bool CallGetParticipants(ProtocolCallContext* pCallContext, const unsigned int& rGatheringID,
                             qList<unsigned int>* pParticipantIDs);
    bool CallAddParticipants(ProtocolCallContext* pCallContext, bool* pSuccess, const unsigned int& rGatheringID,
                             const qList<unsigned int>& rPrincipalIDs, const String& rMessage);
    bool CallGetDetailedParticipants(ProtocolCallContext* pCallContext, const unsigned int& rGatheringID,
                                     qList<ParticipantDetails>* pParticipants);
    bool CallGetParticipantsURLs(ProtocolCallContext* pCallContext, const unsigned int& rGatheringID,
                                 qList<StationURL>* pURLs);
    bool CallFindByType(ProtocolCallContext* pCallContext, const String& rType, const ResultRange& rRange,
                        GatheringHolderList* pGatherings);
    bool CallFindByDescription(ProtocolCallContext* pCallContext, const String& rDescription, const ResultRange& rRange,
                               GatheringHolderList* pGatherings);
    bool CallFindByDescriptionRegex(ProtocolCallContext* pCallContext, const String& rDescription,
                                    const ResultRange& rRange, GatheringHolderList* pGatherings);
    bool CallFindByID(ProtocolCallContext* pCallContext, const qList<unsigned int>& rGatheringIDs,
                      GatheringHolderList* pGatherings);
    bool CallFindBySingleID(ProtocolCallContext* pCallContext, const unsigned int& rGatheringID, bool* pSuccess,
                            GatheringHolder* pGathering);
    bool CallFindByOwner(ProtocolCallContext* pCallContext, const unsigned int& rOwnerID, const ResultRange& rRange,
                         GatheringHolderList* pGatherings);
    bool CallFindByParticipants(ProtocolCallContext* pCallContext, const qList<unsigned int>& rPrincipalIDs,
                                GatheringHolderList* pGatherings);
    bool CallFindInvitations(ProtocolCallContext* pCallContext, const ResultRange& rRange,
                             GatheringHolderList* pGatherings);
    bool CallFindBySQLQuery(ProtocolCallContext* pCallContext, const String& rQuery, const ResultRange& rRange,
                            GatheringHolderList* pGatherings);
    bool CallLaunchSession(ProtocolCallContext* pCallContext, bool* pSuccess, const unsigned int& rGatheringID,
                           const String& rSessionURL);
    bool CallUpdateSessionURL(ProtocolCallContext* pCallContext, bool* pSuccess, const unsigned int& rGatheringID,
                              const String& rSessionURL);
    bool CallGetSessionURL(ProtocolCallContext* pCallContext, bool* pSuccess, const unsigned int& rGatheringID,
                           String* pSessionURL);
    bool CallGetState(ProtocolCallContext* pCallContext, bool* pSuccess, const unsigned int& rGatheringID,
                      unsigned int* pState);
    bool CallSetState(ProtocolCallContext* pCallContext, bool* pSuccess, const unsigned int& rGatheringID,
                      const unsigned int& rState);
    bool CallReportStats(ProtocolCallContext* pCallContext, bool* pSuccess, const unsigned int& rGatheringID,
                         const qList<GatheringStats>& rStats);
    bool CallGetStats(ProtocolCallContext* pCallContext, bool* pSuccess, const unsigned int& rGatheringID,
                      const qList<unsigned int>& rIDs, const qList<unsigned char>& rTypes,
                      qList<GatheringStats>* pStats);
    bool CallDeleteGathering(ProtocolCallContext* pCallContext, bool* pSuccess, const unsigned int& rGatheringID);
    bool CallGetPendingDeletions(ProtocolCallContext* pCallContext, bool* pSuccess, const unsigned int& rGatheringID,
                                 const ResultRange& rRange, qList<DeletionEntry>* pDeletions);
    bool CallDeleteFromDeletions(ProtocolCallContext* pCallContext, bool* pSuccess,
                                 const qList<unsigned int>& rGatheringIDs);
    bool CallMigrateGatheringOwnershipV1(ProtocolCallContext* pCallContext, bool* pSuccess,
                                         const unsigned int& rGatheringID, const qList<unsigned int>& rCandidateIDs);
    bool CallFindByDescriptionLike(ProtocolCallContext* pCallContext, const String& rDescription,
                                   const ResultRange& rRange, GatheringHolderList* pGatherings);
    bool CallRegisterLocalURL(ProtocolCallContext* pCallContext, const unsigned int& rGatheringID,
                              const String& rSessionURL);
    bool CallRegisterLocalURLs(ProtocolCallContext* pCallContext, const unsigned int& rGatheringID,
                               const qList<StationURL>& rSessionURLs);
    bool CallUpdateSessionHostV1(ProtocolCallContext* pCallContext, const unsigned int& rGatheringID);
    bool CallGetSessionURLs(ProtocolCallContext* pCallContext, const unsigned int& rGatheringID,
                            qList<StationURL>* pURLs);
    bool CallUpdateSessionHost(ProtocolCallContext* pCallContext, const unsigned int& rGatheringID, const bool& rFlag);
    bool CallUpdateGatheringOwnership(ProtocolCallContext* pCallContext, const unsigned int& rGatheringID,
                                      const bool& rFlag);
    bool CallMigrateGatheringOwnership(ProtocolCallContext* pCallContext, const unsigned int& rGatheringID,
                                       const qList<unsigned int>& rCandidateIDs, const bool& rFlag);
    static bool RegisterGathering_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                 unsigned int* pGatheringID, const GatheringHolder& rGathering);
    static bool UnregisterGathering_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pSuccess,
                                                   const unsigned int& rGatheringID);
    static bool UnregisterGatherings_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                    bool* pSuccess, const qList<unsigned int>& rGatheringIDs);
    static bool UpdateGathering_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pSuccess,
                                               const GatheringHolder& rGathering);
    static bool Invite_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pSuccess,
                                      const unsigned int& rGatheringID, const qList<unsigned int>& rPrincipalIDs,
                                      const String& rMessage);
    static bool AcceptInvitation_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pSuccess,
                                                const unsigned int& rGatheringID, const String& rMessage);
    static bool DeclineInvitation_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pSuccess,
                                                 const unsigned int& rGatheringID, const String& rMessage);
    static bool CancelInvitation_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pSuccess,
                                                const unsigned int& rGatheringID,
                                                const qList<unsigned int>& rPrincipalIDs, const String& rMessage);
    static bool GetInvitationsSent_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                  const unsigned int& rGatheringID, qList<Invitation>* pInvitations);
    static bool GetInvitationsReceived_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                      qList<Invitation>* pInvitations);
    static bool Participate_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pSuccess,
                                           const unsigned int& rGatheringID, const String& rMessage);
    static bool CancelParticipation_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pSuccess,
                                                   const unsigned int& rGatheringID, const String& rMessage);
    static bool GetParticipants_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                               const unsigned int& rGatheringID, qList<unsigned int>* pParticipantIDs);
    static bool AddParticipants_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pSuccess,
                                               const unsigned int& rGatheringID,
                                               const qList<unsigned int>& rPrincipalIDs, const String& rMessage);
    static bool GetDetailedParticipants_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                       const unsigned int& rGatheringID,
                                                       qList<ParticipantDetails>* pParticipants);
    static bool GetParticipantsURLs_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                   const unsigned int& rGatheringID, qList<StationURL>* pURLs);
    static bool FindByType_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, const String& rType,
                                          const ResultRange& rRange, GatheringHolderList* pGatherings);
    static bool FindByDescription_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                 const String& rDescription, const ResultRange& rRange,
                                                 GatheringHolderList* pGatherings);
    static bool FindByDescriptionRegex_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                      const String& rDescription, const ResultRange& rRange,
                                                      GatheringHolderList* pGatherings);
    static bool FindByID_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                        const qList<unsigned int>& rGatheringIDs, GatheringHolderList* pGatherings);
    static bool FindBySingleID_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                              const unsigned int& rGatheringID, bool* pSuccess,
                                              GatheringHolder* pGathering);
    static bool FindByOwner_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                           const unsigned int& rOwnerID, const ResultRange& rRange,
                                           GatheringHolderList* pGatherings);
    static bool FindByParticipants_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                  const qList<unsigned int>& rPrincipalIDs,
                                                  GatheringHolderList* pGatherings);
    static bool FindInvitations_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                               const ResultRange& rRange, GatheringHolderList* pGatherings);
    static bool FindBySQLQuery_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                              const String& rQuery, const ResultRange& rRange,
                                              GatheringHolderList* pGatherings);
    static bool LaunchSession_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pSuccess,
                                             const unsigned int& rGatheringID, const String& rSessionURL);
    static bool UpdateSessionURL_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pSuccess,
                                                const unsigned int& rGatheringID, const String& rSessionURL);
    static bool GetSessionURL_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pSuccess,
                                             const unsigned int& rGatheringID, String* pSessionURL);
    static bool GetState_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pSuccess,
                                        const unsigned int& rGatheringID, unsigned int* pState);
    static bool SetState_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pSuccess,
                                        const unsigned int& rGatheringID, const unsigned int& rState);
    static bool ReportStats_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pSuccess,
                                           const unsigned int& rGatheringID, const qList<GatheringStats>& rStats);
    static bool GetStats_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pSuccess,
                                        const unsigned int& rGatheringID, const qList<unsigned int>& rIDs,
                                        const qList<unsigned char>& rTypes, qList<GatheringStats>* pStats);
    static bool DeleteGathering_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pSuccess,
                                               const unsigned int& rGatheringID);
    static bool GetPendingDeletions_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pSuccess,
                                                   const unsigned int& rGatheringID, const ResultRange& rRange,
                                                   qList<DeletionEntry>* pDeletions);
    static bool DeleteFromDeletions_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pSuccess,
                                                   const qList<unsigned int>& rGatheringIDs);
    static bool MigrateGatheringOwnershipV1_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                           bool* pSuccess, const unsigned int& rGatheringID,
                                                           const qList<unsigned int>& rCandidateIDs);
    static bool FindByDescriptionLike_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                     const String& rDescription, const ResultRange& rRange,
                                                     GatheringHolderList* pGatherings);
    static bool RegisterLocalURL_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                const unsigned int& rGatheringID, const String& rSessionURL);
    static bool RegisterLocalURLs_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                 const unsigned int& rGatheringID,
                                                 const qList<StationURL>& rSessionURLs);
    static bool UpdateSessionHostV1_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                   const unsigned int& rGatheringID);
    static bool GetSessionURLs_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                              const unsigned int& rGatheringID, qList<StationURL>* pURLs);
    static bool UpdateSessionHost_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                 const unsigned int& rGatheringID, const bool& rFlag);
    static bool UpdateGatheringOwnership_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                        const unsigned int& rGatheringID, const bool& rFlag);
    static bool MigrateGatheringOwnership_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                         const unsigned int& rGatheringID,
                                                         const qList<unsigned int>& rCandidateIDs, const bool& rFlag);
    static void ProtoReturn_RegisterGathering(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_UnregisterGathering(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_UnregisterGatherings(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_UpdateGathering(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_Invite(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_AcceptInvitation(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_DeclineInvitation(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_CancelInvitation(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_GetInvitationsSent(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_GetInvitationsReceived(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_Participate(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_CancelParticipation(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_GetParticipants(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_AddParticipants(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_GetDetailedParticipants(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_GetParticipantsURLs(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_FindByType(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_FindByDescription(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_FindByDescriptionRegex(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_FindByID(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_FindBySingleID(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_FindByOwner(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_FindByParticipants(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_FindInvitations(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_FindBySQLQuery(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_LaunchSession(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_UpdateSessionURL(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_GetSessionURL(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_GetState(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_SetState(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_ReportStats(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_GetStats(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_DeleteGathering(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_GetPendingDeletions(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_DeleteFromDeletions(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_MigrateGatheringOwnershipV1(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_FindByDescriptionLike(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_RegisterLocalURL(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_RegisterLocalURLs(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_UpdateSessionHostV1(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_GetSessionURLs(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_UpdateSessionHost(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_UpdateGatheringOwnership(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_MigrateGatheringOwnership(Message* pMessage, ProtocolCallContext* pCallContext);
};

}
}

#endif
