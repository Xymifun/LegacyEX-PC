#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__PROTO_MATCHMAKINGPROTOCOLSERVER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__PROTO_MATCHMAKINGPROTOCOLSERVER_HPP

#include "nn/nex/AnyObjectHolder.hpp"
#include "nn/nex/ServerProtocol.hpp"
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

class _Proto_MatchMakingProtocolServer : public ServerProtocol
{
public:
    enum _MethodID
    {
        METHOD_REGISTER_GATHERING = 1,
        METHOD_UNREGISTER_GATHERING = 2,
        METHOD_UNREGISTER_GATHERINGS = 3,
        METHOD_UPDATE_GATHERING = 4,
        METHOD_INVITE = 5,
        METHOD_ACCEPT_INVITATION = 6,
        METHOD_DECLINE_INVITATION = 7,
        METHOD_CANCEL_INVITATION = 8,
        METHOD_GET_INVITATIONS_SENT = 9,
        METHOD_GET_INVITATIONS_RECEIVED = 10,
        METHOD_PARTICIPATE = 11,
        METHOD_CANCEL_PARTICIPATION = 12,
        METHOD_GET_PARTICIPANTS = 13,
        METHOD_ADD_PARTICIPANTS = 14,
        METHOD_GET_DETAILED_PARTICIPANTS = 15,
        METHOD_GET_PARTICIPANTS_URLS = 16,
        METHOD_FIND_BY_TYPE = 17,
        METHOD_FIND_BY_DESCRIPTION = 18,
        METHOD_FIND_BY_DESCRIPTION_REGEX = 19,
        METHOD_FIND_BY_ID = 20,
        METHOD_FIND_BY_SINGLE_ID = 21,
        METHOD_FIND_BY_OWNER = 22,
        METHOD_FIND_BY_PARTICIPANTS = 23,
        METHOD_FIND_INVITATIONS = 24,
        METHOD_FIND_BY_SQL_QUERY = 25,
        METHOD_LAUNCH_SESSION = 26,
        METHOD_UPDATE_SESSION_URL = 27,
        METHOD_GET_SESSION_URL = 28,
        METHOD_GET_STATE = 29,
        METHOD_SET_STATE = 30,
        METHOD_REPORT_STATS = 31,
        METHOD_GET_STATS = 32,
        METHOD_DELETE_GATHERING = 33,
        METHOD_GET_PENDING_DELETIONS = 34,
        METHOD_DELETE_FROM_DELETIONS = 35,
        METHOD_MIGRATE_GATHERING_OWNERSHIP_V1 = 36,
        METHOD_FIND_BY_DESCRIPTION_LIKE = 37,
        METHOD_REGISTER_LOCAL_URL = 38,
        METHOD_REGISTER_LOCAL_URLS = 39,
        METHOD_UPDATE_SESSION_HOST_V1 = 40,
        METHOD_GET_SESSION_URLS = 41,
        METHOD_UPDATE_SESSION_HOST = 42,
        METHOD_UPDATE_GATHERING_OWNERSHIP = 43,
        METHOD_MIGRATE_GATHERING_OWNERSHIP = 44
    };

    explicit _Proto_MatchMakingProtocolServer(unsigned int uiFlags) : ServerProtocol(uiFlags) {}

    virtual void DispatchProtocolMessage(Message* pRequest, Message* pResponse, bool* pResponseRequired,
                                         EndPoint* pEndPoint);
    virtual unsigned int RegisterGathering(const AnyObjectHolder<Gathering, String>& rGathering) = 0;
    virtual bool UnregisterGathering(const unsigned int& rGatheringID) = 0;
    virtual bool UnregisterGatherings(const qList<unsigned int>& rGatheringIDs) = 0;
    virtual bool UpdateGathering(const AnyObjectHolder<Gathering, String>& rGathering) = 0;
    virtual bool Invite(const unsigned int& rGatheringID, const qList<unsigned int>& rPrincipalIDs,
                        const String& rMessage) = 0;
    virtual bool AcceptInvitation(const unsigned int& rGatheringID, const String& rMessage) = 0;
    virtual bool DeclineInvitation(const unsigned int& rGatheringID, const String& rMessage) = 0;
    virtual bool CancelInvitation(const unsigned int& rGatheringID, const qList<unsigned int>& rPrincipalIDs,
                                  const String& rMessage) = 0;
    virtual void GetInvitationsSent(const unsigned int& rGatheringID, qList<Invitation>* pInvitations) = 0;
    virtual void GetInvitationsReceived(qList<Invitation>* pInvitations) = 0;
    virtual bool Participate(const unsigned int& rGatheringID, const String& rMessage) = 0;
    virtual bool CancelParticipation(const unsigned int& rGatheringID, const String& rMessage) = 0;
    virtual void GetParticipants(const unsigned int& rGatheringID, qList<unsigned int>* pParticipantIDs) = 0;
    virtual bool AddParticipants(const unsigned int& rGatheringID, const qList<unsigned int>& rPrincipalIDs,
                                 const String& rMessage) = 0;
    virtual void GetDetailedParticipants(const unsigned int& rGatheringID,
                                         qList<ParticipantDetails>* pParticipants) = 0;
    virtual void GetParticipantsURLs(const unsigned int& rGatheringID, qList<StationURL>* pURLs) = 0;
    virtual void FindByType(const String& rType, const ResultRange& rRange,
                            qList<AnyObjectHolder<Gathering, String> >* pGatherings) = 0;
    virtual void FindByDescription(const String& rDescription, const ResultRange& rRange,
                                   qList<AnyObjectHolder<Gathering, String> >* pGatherings) = 0;
    virtual void FindByDescriptionRegex(const String& rDescription, const ResultRange& rRange,
                                        qList<AnyObjectHolder<Gathering, String> >* pGatherings) = 0;
    virtual void FindByID(const qList<unsigned int>& rGatheringIDs,
                          qList<AnyObjectHolder<Gathering, String> >* pGatherings) = 0;
    virtual void FindBySingleID(const unsigned int& rGatheringID, bool* pSuccess,
                                AnyObjectHolder<Gathering, String>* pGathering) = 0;
    virtual void FindByOwner(const unsigned int& rOwnerID, const ResultRange& rRange,
                             qList<AnyObjectHolder<Gathering, String> >* pGatherings) = 0;
    virtual void FindByParticipants(const qList<unsigned int>& rPrincipalIDs,
                                    qList<AnyObjectHolder<Gathering, String> >* pGatherings) = 0;
    virtual void FindInvitations(const ResultRange& rRange,
                                 qList<AnyObjectHolder<Gathering, String> >* pGatherings) = 0;
    virtual void FindBySQLQuery(const String& rQuery, const ResultRange& rRange,
                                qList<AnyObjectHolder<Gathering, String> >* pGatherings) = 0;
    virtual bool LaunchSession(const unsigned int& rGatheringID, const String& rSessionURL) = 0;
    virtual bool UpdateSessionURL(const unsigned int& rGatheringID, const String& rSessionURL) = 0;
    virtual bool GetSessionURL(const unsigned int& rGatheringID, String* pSessionURL) = 0;
    virtual bool GetState(const unsigned int& rGatheringID, unsigned int* pState) = 0;
    virtual bool SetState(const unsigned int& rGatheringID, const unsigned int& rState) = 0;
    virtual bool ReportStats(const unsigned int& rGatheringID, const qList<GatheringStats>& rStats) = 0;
    virtual bool GetStats(const unsigned int& rGatheringID, const qList<unsigned int>& rIDs,
                          const qList<unsigned char>& rTypes, qList<GatheringStats>* pStats) = 0;
    virtual bool DeleteGathering(const unsigned int& rGatheringID) = 0;
    virtual bool GetPendingDeletions(const unsigned int& rGatheringID, const ResultRange& rRange,
                                     qList<DeletionEntry>* pDeletions) = 0;
    virtual bool DeleteFromDeletions(const qList<unsigned int>& rGatheringIDs) = 0;
    virtual bool MigrateGatheringOwnershipV1(const unsigned int& rGatheringID,
                                             const qList<unsigned int>& rCandidateIDs) = 0;
    virtual void FindByDescriptionLike(const String& rDescription, const ResultRange& rRange,
                                       qList<AnyObjectHolder<Gathering, String> >* pGatherings) = 0;
    virtual void RegisterLocalURL(const unsigned int& rGatheringID, const String& rSessionURL) = 0;
    virtual void RegisterLocalURLs(const unsigned int& rGatheringID, const qList<StationURL>& rSessionURLs) = 0;
    virtual void UpdateSessionHostV1(const unsigned int& rGatheringID) = 0;
    virtual void GetSessionURLs(const unsigned int& rGatheringID, qList<StationURL>* pURLs) = 0;
    virtual void UpdateSessionHost(const unsigned int& rGatheringID, const bool& rFlag) = 0;
    virtual void UpdateGatheringOwnership(const unsigned int& rGatheringID, const bool& rFlag) = 0;
    virtual void MigrateGatheringOwnership(const unsigned int& rGatheringID, const qList<unsigned int>& rCandidateIDs,
                                           const bool& rFlag) = 0;

private:
    enum
    {
        RESULT_NOT_IMPLEMENTED = static_cast<int>(0x80010002u),
        RESPONSE_METHOD_FLAG = 0x8000
    };

    static void ProtoStub_RegisterGathering(Message* pRequest, Message* pResponse,
                                            _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_UnregisterGathering(Message* pRequest, Message* pResponse,
                                              _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_UnregisterGatherings(Message* pRequest, Message* pResponse,
                                               _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_UpdateGathering(Message* pRequest, Message* pResponse,
                                          _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_Invite(Message* pRequest, Message* pResponse, _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_AcceptInvitation(Message* pRequest, Message* pResponse,
                                           _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_DeclineInvitation(Message* pRequest, Message* pResponse,
                                            _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_CancelInvitation(Message* pRequest, Message* pResponse,
                                           _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_GetInvitationsSent(Message* pRequest, Message* pResponse,
                                             _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_GetInvitationsReceived(Message* pRequest, Message* pResponse,
                                                 _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_Participate(Message* pRequest, Message* pResponse,
                                      _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_CancelParticipation(Message* pRequest, Message* pResponse,
                                              _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_GetParticipants(Message* pRequest, Message* pResponse,
                                          _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_AddParticipants(Message* pRequest, Message* pResponse,
                                          _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_GetDetailedParticipants(Message* pRequest, Message* pResponse,
                                                  _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_GetParticipantsURLs(Message* pRequest, Message* pResponse,
                                              _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_FindByType(Message* pRequest, Message* pResponse,
                                     _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_FindByDescription(Message* pRequest, Message* pResponse,
                                            _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_FindByDescriptionRegex(Message* pRequest, Message* pResponse,
                                                 _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_FindByID(Message* pRequest, Message* pResponse, _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_FindBySingleID(Message* pRequest, Message* pResponse,
                                         _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_FindByOwner(Message* pRequest, Message* pResponse,
                                      _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_FindByParticipants(Message* pRequest, Message* pResponse,
                                             _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_FindInvitations(Message* pRequest, Message* pResponse,
                                          _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_FindBySQLQuery(Message* pRequest, Message* pResponse,
                                         _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_LaunchSession(Message* pRequest, Message* pResponse,
                                        _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_UpdateSessionURL(Message* pRequest, Message* pResponse,
                                           _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_GetSessionURL(Message* pRequest, Message* pResponse,
                                        _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_GetState(Message* pRequest, Message* pResponse, _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_SetState(Message* pRequest, Message* pResponse, _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_ReportStats(Message* pRequest, Message* pResponse,
                                      _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_GetStats(Message* pRequest, Message* pResponse, _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_DeleteGathering(Message* pRequest, Message* pResponse,
                                          _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_GetPendingDeletions(Message* pRequest, Message* pResponse,
                                              _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_DeleteFromDeletions(Message* pRequest, Message* pResponse,
                                              _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_MigrateGatheringOwnershipV1(Message* pRequest, Message* pResponse,
                                                      _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_FindByDescriptionLike(Message* pRequest, Message* pResponse,
                                                _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_RegisterLocalURL(Message* pRequest, Message* pResponse,
                                           _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_RegisterLocalURLs(Message* pRequest, Message* pResponse,
                                            _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_UpdateSessionHostV1(Message* pRequest, Message* pResponse,
                                              _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_GetSessionURLs(Message* pRequest, Message* pResponse,
                                         _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_UpdateSessionHost(Message* pRequest, Message* pResponse,
                                            _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_UpdateGatheringOwnership(Message* pRequest, Message* pResponse,
                                                   _Proto_MatchMakingProtocolServer* pProtocol);
    static void ProtoStub_MigrateGatheringOwnership(Message* pRequest, Message* pResponse,
                                                    _Proto_MatchMakingProtocolServer* pProtocol);
};

}
}

#endif
