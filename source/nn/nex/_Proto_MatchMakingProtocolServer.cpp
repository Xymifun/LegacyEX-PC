#include "nn/nex/_Proto_MatchMakingProtocolServer.hpp"

#include <stdint.h>

#include "nn/nex/DDLClassMetaType.hpp"
#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/DeletionEntry.hpp"
#include "nn/nex/Gathering.hpp"
#include "nn/nex/GatheringStats.hpp"
#include "nn/nex/Invitation.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/ParticipantDetails.hpp"
#include "nn/nex/ResultRange.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/_Type_qlist.hpp"

namespace nn
{
namespace nex
{

inline void _Proto_MatchMakingProtocolServer::ProtoStub_RegisterGathering(Message* pRequest, Message* pResponse,
                                                                          _Proto_MatchMakingProtocolServer* pProtocol)
{
    AnyObjectHolder<Gathering, String> gathering;
    _Type_any<DDLClassMetaType<Gathering>, _Type_string>::Extract(pRequest, &gathering);
    if (pRequest->HasError())
    {
        return;
    }
    const unsigned int uiGatheringID = pProtocol->RegisterGathering(gathering);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_REGISTER_GATHERING);
    _Type_uint32::Add(pResponse, uiGatheringID);
}

inline void _Proto_MatchMakingProtocolServer::ProtoStub_UnregisterGathering(Message* pRequest, Message* pResponse,
                                                                            _Proto_MatchMakingProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    const bool bResult = pProtocol->UnregisterGathering(uiGatheringID);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_UNREGISTER_GATHERING);
    *pResponse << bResult;
}

inline void _Proto_MatchMakingProtocolServer::ProtoStub_UnregisterGatherings(
    Message* pRequest, Message* pResponse, _Proto_MatchMakingProtocolServer* pProtocol)
{
    qList<unsigned int> gatheringIDs;
    _Type_std_list<_Type_uint32>::Extract(pRequest, &gatheringIDs);
    if (pRequest->HasError())
    {
        return;
    }
    const bool bResult = pProtocol->UnregisterGatherings(gatheringIDs);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_UNREGISTER_GATHERINGS);
    *pResponse << bResult;
}

inline void _Proto_MatchMakingProtocolServer::ProtoStub_UpdateGathering(Message* pRequest, Message* pResponse,
                                                                        _Proto_MatchMakingProtocolServer* pProtocol)
{
    AnyObjectHolder<Gathering, String> gathering;
    _Type_any<DDLClassMetaType<Gathering>, _Type_string>::Extract(pRequest, &gathering);
    if (pRequest->HasError())
    {
        return;
    }
    const bool bResult = pProtocol->UpdateGathering(gathering);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_UPDATE_GATHERING);
    *pResponse << bResult;
}

inline void _Proto_MatchMakingProtocolServer::ProtoStub_Invite(Message* pRequest, Message* pResponse,
                                                               _Proto_MatchMakingProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    qList<unsigned int> principalIDs;
    _Type_std_list<_Type_uint32>::Extract(pRequest, &principalIDs);
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
    const bool bResult = pProtocol->Invite(uiGatheringID, principalIDs, message);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_INVITE);
    *pResponse << bResult;
}

inline void _Proto_MatchMakingProtocolServer::ProtoStub_AcceptInvitation(Message* pRequest, Message* pResponse,
                                                                         _Proto_MatchMakingProtocolServer* pProtocol)
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
    const bool bResult = pProtocol->AcceptInvitation(uiGatheringID, message);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_ACCEPT_INVITATION);
    *pResponse << bResult;
}

inline void _Proto_MatchMakingProtocolServer::ProtoStub_DeclineInvitation(Message* pRequest, Message* pResponse,
                                                                          _Proto_MatchMakingProtocolServer* pProtocol)
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
    const bool bResult = pProtocol->DeclineInvitation(uiGatheringID, message);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_DECLINE_INVITATION);
    *pResponse << bResult;
}

inline void _Proto_MatchMakingProtocolServer::ProtoStub_CancelInvitation(Message* pRequest, Message* pResponse,
                                                                         _Proto_MatchMakingProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    qList<unsigned int> principalIDs;
    _Type_std_list<_Type_uint32>::Extract(pRequest, &principalIDs);
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
    const bool bResult = pProtocol->CancelInvitation(uiGatheringID, principalIDs, message);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_CANCEL_INVITATION);
    *pResponse << bResult;
}

inline void _Proto_MatchMakingProtocolServer::ProtoStub_GetInvitationsSent(Message* pRequest, Message* pResponse,
                                                                           _Proto_MatchMakingProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    qList<Invitation> invitations;
    pProtocol->GetInvitationsSent(uiGatheringID, &invitations);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GET_INVITATIONS_SENT);
    _Type_std_list<DDLClassMetaType<Invitation> >::Add(pResponse, invitations);
}

inline void _Proto_MatchMakingProtocolServer::ProtoStub_GetInvitationsReceived(
    Message*, Message* pResponse, _Proto_MatchMakingProtocolServer* pProtocol)
{
    qList<Invitation> invitations;
    pProtocol->GetInvitationsReceived(&invitations);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GET_INVITATIONS_RECEIVED);
    _Type_std_list<DDLClassMetaType<Invitation> >::Add(pResponse, invitations);
}

inline void _Proto_MatchMakingProtocolServer::ProtoStub_Participate(Message* pRequest, Message* pResponse,
                                                                    _Proto_MatchMakingProtocolServer* pProtocol)
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
    const bool bResult = pProtocol->Participate(uiGatheringID, message);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_PARTICIPATE);
    *pResponse << bResult;
}

void _Proto_MatchMakingProtocolServer::ProtoStub_CancelParticipation(Message* pRequest, Message* pResponse,
                                                                     _Proto_MatchMakingProtocolServer* pProtocol)
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
    const bool bResult = pProtocol->CancelParticipation(uiGatheringID, message);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_CANCEL_PARTICIPATION);
    *pResponse << bResult;
}

void _Proto_MatchMakingProtocolServer::ProtoStub_GetParticipants(Message* pRequest, Message* pResponse,
                                                                 _Proto_MatchMakingProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    qList<unsigned int> participantIDs;
    pProtocol->GetParticipants(uiGatheringID, &participantIDs);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GET_PARTICIPANTS);
    _Type_std_list<_Type_uint32>::Add(pResponse, participantIDs);
}

void _Proto_MatchMakingProtocolServer::ProtoStub_AddParticipants(Message* pRequest, Message* pResponse,
                                                                 _Proto_MatchMakingProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    qList<unsigned int> principalIDs;
    _Type_std_list<_Type_uint32>::Extract(pRequest, &principalIDs);
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
    const bool bResult = pProtocol->AddParticipants(uiGatheringID, principalIDs, message);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_ADD_PARTICIPANTS);
    *pResponse << bResult;
}

void _Proto_MatchMakingProtocolServer::ProtoStub_GetDetailedParticipants(Message* pRequest, Message* pResponse,
                                                                         _Proto_MatchMakingProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    qList<ParticipantDetails> participants;
    pProtocol->GetDetailedParticipants(uiGatheringID, &participants);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GET_DETAILED_PARTICIPANTS);
    _Type_std_list<DDLClassMetaType<ParticipantDetails> >::Add(pResponse, participants);
}

void _Proto_MatchMakingProtocolServer::ProtoStub_GetParticipantsURLs(Message* pRequest, Message* pResponse,
                                                                     _Proto_MatchMakingProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    qList<StationURL> urls;
    pProtocol->GetParticipantsURLs(uiGatheringID, &urls);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GET_PARTICIPANTS_URLS);
    _Type_std_list<_Type_stationurl>::Add(pResponse, urls);
}

void _Proto_MatchMakingProtocolServer::ProtoStub_FindByType(Message* pRequest, Message* pResponse,
                                                            _Proto_MatchMakingProtocolServer* pProtocol)
{
    String type;
    _Type_string::Extract(pRequest, &type);
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
    pProtocol->FindByType(type, range, &gatherings);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_FIND_BY_TYPE);
    _Type_std_list<_Type_any<DDLClassMetaType<Gathering>, _Type_string> >::Add(pResponse, gatherings);
}

void _Proto_MatchMakingProtocolServer::ProtoStub_FindByDescription(Message* pRequest, Message* pResponse,
                                                                   _Proto_MatchMakingProtocolServer* pProtocol)
{
    String description;
    _Type_string::Extract(pRequest, &description);
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
    pProtocol->FindByDescription(description, range, &gatherings);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_FIND_BY_DESCRIPTION);
    _Type_std_list<_Type_any<DDLClassMetaType<Gathering>, _Type_string> >::Add(pResponse, gatherings);
}

void _Proto_MatchMakingProtocolServer::ProtoStub_FindByDescriptionRegex(Message* pRequest, Message* pResponse,
                                                                        _Proto_MatchMakingProtocolServer* pProtocol)
{
    String description;
    _Type_string::Extract(pRequest, &description);
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
    pProtocol->FindByDescriptionRegex(description, range, &gatherings);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_FIND_BY_DESCRIPTION_REGEX);
    _Type_std_list<_Type_any<DDLClassMetaType<Gathering>, _Type_string> >::Add(pResponse, gatherings);
}

void _Proto_MatchMakingProtocolServer::ProtoStub_FindByID(Message* pRequest, Message* pResponse,
                                                          _Proto_MatchMakingProtocolServer* pProtocol)
{
    qList<unsigned int> gatheringIDs;
    _Type_std_list<_Type_uint32>::Extract(pRequest, &gatheringIDs);
    if (pRequest->HasError())
    {
        return;
    }
    qList<AnyObjectHolder<Gathering, String> > gatherings;
    pProtocol->FindByID(gatheringIDs, &gatherings);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_FIND_BY_ID);
    _Type_std_list<_Type_any<DDLClassMetaType<Gathering>, _Type_string> >::Add(pResponse, gatherings);
}

inline void _Proto_MatchMakingProtocolServer::ProtoStub_FindBySingleID(Message* pRequest, Message* pResponse,
                                                                       _Proto_MatchMakingProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    bool bSuccess;
    AnyObjectHolder<Gathering, String> gathering;
    pProtocol->FindBySingleID(uiGatheringID, &bSuccess, &gathering);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_FIND_BY_SINGLE_ID);
    *pResponse << bSuccess;
    _Type_any<DDLClassMetaType<Gathering>, _Type_string>::Add(pResponse, gathering);
}

void _Proto_MatchMakingProtocolServer::ProtoStub_FindByOwner(Message* pRequest, Message* pResponse,
                                                             _Proto_MatchMakingProtocolServer* pProtocol)
{
    unsigned int uiOwnerID;
    _Type_uint32::Extract(pRequest, &uiOwnerID);
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
    pProtocol->FindByOwner(uiOwnerID, range, &gatherings);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_FIND_BY_OWNER);
    _Type_std_list<_Type_any<DDLClassMetaType<Gathering>, _Type_string> >::Add(pResponse, gatherings);
}

void _Proto_MatchMakingProtocolServer::ProtoStub_FindByParticipants(Message* pRequest, Message* pResponse,
                                                                    _Proto_MatchMakingProtocolServer* pProtocol)
{
    qList<unsigned int> principalIDs;
    _Type_std_list<_Type_uint32>::Extract(pRequest, &principalIDs);
    if (pRequest->HasError())
    {
        return;
    }
    qList<AnyObjectHolder<Gathering, String> > gatherings;
    pProtocol->FindByParticipants(principalIDs, &gatherings);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_FIND_BY_PARTICIPANTS);
    _Type_std_list<_Type_any<DDLClassMetaType<Gathering>, _Type_string> >::Add(pResponse, gatherings);
}

void _Proto_MatchMakingProtocolServer::ProtoStub_FindInvitations(Message* pRequest, Message* pResponse,
                                                                 _Proto_MatchMakingProtocolServer* pProtocol)
{
    ResultRange range;
    DDLClassMetaType<ResultRange>::Extract(pRequest, &range);
    if (pRequest->HasError())
    {
        return;
    }
    qList<AnyObjectHolder<Gathering, String> > gatherings;
    pProtocol->FindInvitations(range, &gatherings);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_FIND_INVITATIONS);
    _Type_std_list<_Type_any<DDLClassMetaType<Gathering>, _Type_string> >::Add(pResponse, gatherings);
}

void _Proto_MatchMakingProtocolServer::ProtoStub_FindBySQLQuery(Message* pRequest, Message* pResponse,
                                                                _Proto_MatchMakingProtocolServer* pProtocol)
{
    String query;
    _Type_string::Extract(pRequest, &query);
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
    pProtocol->FindBySQLQuery(query, range, &gatherings);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_FIND_BY_SQL_QUERY);
    _Type_std_list<_Type_any<DDLClassMetaType<Gathering>, _Type_string> >::Add(pResponse, gatherings);
}

void _Proto_MatchMakingProtocolServer::ProtoStub_LaunchSession(Message* pRequest, Message* pResponse,
                                                               _Proto_MatchMakingProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    String sessionURL;
    _Type_string::Extract(pRequest, &sessionURL);
    if (pRequest->HasError())
    {
        return;
    }
    const bool bResult = pProtocol->LaunchSession(uiGatheringID, sessionURL);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_LAUNCH_SESSION);
    *pResponse << bResult;
}

void _Proto_MatchMakingProtocolServer::ProtoStub_UpdateSessionURL(Message* pRequest, Message* pResponse,
                                                                  _Proto_MatchMakingProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    String sessionURL;
    _Type_string::Extract(pRequest, &sessionURL);
    if (pRequest->HasError())
    {
        return;
    }
    const bool bResult = pProtocol->UpdateSessionURL(uiGatheringID, sessionURL);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_UPDATE_SESSION_URL);
    *pResponse << bResult;
}

inline void _Proto_MatchMakingProtocolServer::ProtoStub_GetSessionURL(Message* pRequest, Message* pResponse,
                                                                      _Proto_MatchMakingProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    String sessionURL;
    const bool bResult = pProtocol->GetSessionURL(uiGatheringID, &sessionURL);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GET_SESSION_URL);
    *pResponse << bResult;
    _Type_string::Add(pResponse, sessionURL);
}

inline void _Proto_MatchMakingProtocolServer::ProtoStub_GetState(Message* pRequest, Message* pResponse,
                                                                 _Proto_MatchMakingProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    unsigned int uiState;
    const bool bResult = pProtocol->GetState(uiGatheringID, &uiState);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GET_STATE);
    *pResponse << bResult;
    _Type_uint32::Add(pResponse, uiState);
}

void _Proto_MatchMakingProtocolServer::ProtoStub_SetState(Message* pRequest, Message* pResponse,
                                                          _Proto_MatchMakingProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    unsigned int uiState;
    _Type_uint32::Extract(pRequest, &uiState);
    if (pRequest->HasError())
    {
        return;
    }
    const bool bResult = pProtocol->SetState(uiGatheringID, uiState);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_SET_STATE);
    *pResponse << bResult;
}

void _Proto_MatchMakingProtocolServer::ProtoStub_ReportStats(Message* pRequest, Message* pResponse,
                                                             _Proto_MatchMakingProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    qList<GatheringStats> stats;
    _Type_std_list<DDLClassMetaType<GatheringStats> >::Extract(pRequest, &stats);
    if (pRequest->HasError())
    {
        return;
    }
    const bool bResult = pProtocol->ReportStats(uiGatheringID, stats);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_REPORT_STATS);
    *pResponse << bResult;
}

void _Proto_MatchMakingProtocolServer::ProtoStub_GetStats(Message* pRequest, Message* pResponse,
                                                          _Proto_MatchMakingProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    qList<unsigned int> ids;
    _Type_std_list<_Type_uint32>::Extract(pRequest, &ids);
    if (pRequest->HasError())
    {
        return;
    }
    qList<unsigned char> types;
    _Type_std_list<_Type_byte>::Extract(pRequest, &types);
    if (pRequest->HasError())
    {
        return;
    }
    qList<GatheringStats> stats;
    const bool bResult = pProtocol->GetStats(uiGatheringID, ids, types, &stats);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GET_STATS);
    *pResponse << bResult;
    _Type_std_list<DDLClassMetaType<GatheringStats> >::Add(pResponse, stats);
}

inline void _Proto_MatchMakingProtocolServer::ProtoStub_DeleteGathering(Message* pRequest, Message* pResponse,
                                                                        _Proto_MatchMakingProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    const bool bResult = pProtocol->DeleteGathering(uiGatheringID);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_DELETE_GATHERING);
    *pResponse << bResult;
}

void _Proto_MatchMakingProtocolServer::ProtoStub_GetPendingDeletions(Message* pRequest, Message* pResponse,
                                                                     _Proto_MatchMakingProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
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
    qList<DeletionEntry> deletions;
    const bool bResult = pProtocol->GetPendingDeletions(uiGatheringID, range, &deletions);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GET_PENDING_DELETIONS);
    *pResponse << bResult;
    _Type_std_list<DDLClassMetaType<DeletionEntry> >::Add(pResponse, deletions);
}

void _Proto_MatchMakingProtocolServer::ProtoStub_DeleteFromDeletions(Message* pRequest, Message* pResponse,
                                                                     _Proto_MatchMakingProtocolServer* pProtocol)
{
    qList<unsigned int> gatheringIDs;
    _Type_std_list<_Type_uint32>::Extract(pRequest, &gatheringIDs);
    if (pRequest->HasError())
    {
        return;
    }
    const bool bResult = pProtocol->DeleteFromDeletions(gatheringIDs);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_DELETE_FROM_DELETIONS);
    *pResponse << bResult;
}

inline void _Proto_MatchMakingProtocolServer::ProtoStub_MigrateGatheringOwnershipV1(
    Message* pRequest, Message* pResponse, _Proto_MatchMakingProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (!pRequest->HasError())
    {
        qList<unsigned int> candidateIDs;
        _Type_std_list<_Type_uint32>::Extract(pRequest, &candidateIDs);
        if (!pRequest->HasError())
        {
            const bool bResult = pProtocol->MigrateGatheringOwnershipV1(uiGatheringID, candidateIDs);
            AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_MIGRATE_GATHERING_OWNERSHIP_V1);
            *pResponse << bResult;
        }
    }
}

void _Proto_MatchMakingProtocolServer::ProtoStub_FindByDescriptionLike(Message* pRequest, Message* pResponse,
                                                                       _Proto_MatchMakingProtocolServer* pProtocol)
{
    String description;
    _Type_string::Extract(pRequest, &description);
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
    pProtocol->FindByDescriptionLike(description, range, &gatherings);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_FIND_BY_DESCRIPTION_LIKE);
    _Type_std_list<_Type_any<DDLClassMetaType<Gathering>, _Type_string> >::Add(pResponse, gatherings);
}

inline void _Proto_MatchMakingProtocolServer::ProtoStub_RegisterLocalURL(Message* pRequest, Message* pResponse,
                                                                         _Proto_MatchMakingProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    String sessionURL;
    _Type_string::Extract(pRequest, &sessionURL);
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->RegisterLocalURL(uiGatheringID, sessionURL);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_REGISTER_LOCAL_URL);
}

void _Proto_MatchMakingProtocolServer::ProtoStub_RegisterLocalURLs(Message* pRequest, Message* pResponse,
                                                                   _Proto_MatchMakingProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    qList<StationURL> sessionURLs;
    _Type_std_list<_Type_stationurl>::Extract(pRequest, &sessionURLs);
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->RegisterLocalURLs(uiGatheringID, sessionURLs);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_REGISTER_LOCAL_URLS);
}

void _Proto_MatchMakingProtocolServer::ProtoStub_UpdateSessionHostV1(Message* pRequest, Message* pResponse,
                                                                     _Proto_MatchMakingProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->UpdateSessionHostV1(uiGatheringID);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_UPDATE_SESSION_HOST_V1);
}

void _Proto_MatchMakingProtocolServer::ProtoStub_GetSessionURLs(Message* pRequest, Message* pResponse,
                                                                _Proto_MatchMakingProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    qList<StationURL> urls;
    pProtocol->GetSessionURLs(uiGatheringID, &urls);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GET_SESSION_URLS);
    _Type_std_list<_Type_stationurl>::Add(pResponse, urls);
}

inline void _Proto_MatchMakingProtocolServer::ProtoStub_UpdateSessionHost(Message* pRequest, Message* pResponse,
                                                                          _Proto_MatchMakingProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    bool bFlag;
    *pRequest >> bFlag;
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->UpdateSessionHost(uiGatheringID, bFlag);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_UPDATE_SESSION_HOST);
}

inline void _Proto_MatchMakingProtocolServer::ProtoStub_UpdateGatheringOwnership(
    Message* pRequest, Message* pResponse, _Proto_MatchMakingProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    bool bFlag;
    *pRequest >> bFlag;
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->UpdateGatheringOwnership(uiGatheringID, bFlag);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_UPDATE_GATHERING_OWNERSHIP);
}

void _Proto_MatchMakingProtocolServer::ProtoStub_MigrateGatheringOwnership(Message* pRequest, Message* pResponse,
                                                                           _Proto_MatchMakingProtocolServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    qList<unsigned int> candidateIDs;
    _Type_qlist<_Type_uint32>::Extract(pRequest, &candidateIDs);
    if (pRequest->HasError())
    {
        return;
    }
    bool bFlag;
    *pRequest >> bFlag;
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->MigrateGatheringOwnership(uiGatheringID, candidateIDs, bFlag);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_MIGRATE_GATHERING_OWNERSHIP);
}

void _Proto_MatchMakingProtocolServer::DispatchProtocolMessage(Message* pRequest, Message* pResponse,
                                                               bool* pResponseRequired, EndPoint*)
{
    switch (static_cast<uint16_t>(ExtractMethodID(pRequest)))
    {
    case METHOD_REGISTER_GATHERING:
        ProtoStub_RegisterGathering(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_UNREGISTER_GATHERING:
        ProtoStub_UnregisterGathering(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_UNREGISTER_GATHERINGS:
        ProtoStub_UnregisterGatherings(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_UPDATE_GATHERING:
        ProtoStub_UpdateGathering(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_INVITE:
        ProtoStub_Invite(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_ACCEPT_INVITATION:
        ProtoStub_AcceptInvitation(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_DECLINE_INVITATION:
        ProtoStub_DeclineInvitation(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_CANCEL_INVITATION:
        ProtoStub_CancelInvitation(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_GET_INVITATIONS_SENT:
        ProtoStub_GetInvitationsSent(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_GET_INVITATIONS_RECEIVED:
        ProtoStub_GetInvitationsReceived(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_PARTICIPATE:
        ProtoStub_Participate(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_CANCEL_PARTICIPATION:
        ProtoStub_CancelParticipation(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_GET_PARTICIPANTS:
        ProtoStub_GetParticipants(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_ADD_PARTICIPANTS:
        ProtoStub_AddParticipants(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_GET_DETAILED_PARTICIPANTS:
        ProtoStub_GetDetailedParticipants(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_GET_PARTICIPANTS_URLS:
        ProtoStub_GetParticipantsURLs(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_FIND_BY_TYPE:
        ProtoStub_FindByType(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_FIND_BY_DESCRIPTION:
        ProtoStub_FindByDescription(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_FIND_BY_DESCRIPTION_REGEX:
        ProtoStub_FindByDescriptionRegex(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_FIND_BY_ID:
        ProtoStub_FindByID(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_FIND_BY_SINGLE_ID:
        ProtoStub_FindBySingleID(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_FIND_BY_OWNER:
        ProtoStub_FindByOwner(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_FIND_BY_PARTICIPANTS:
        ProtoStub_FindByParticipants(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_FIND_INVITATIONS:
        ProtoStub_FindInvitations(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_FIND_BY_SQL_QUERY:
        ProtoStub_FindBySQLQuery(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_LAUNCH_SESSION:
        ProtoStub_LaunchSession(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_UPDATE_SESSION_URL:
        ProtoStub_UpdateSessionURL(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_GET_SESSION_URL:
        ProtoStub_GetSessionURL(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_GET_STATE:
        ProtoStub_GetState(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_SET_STATE:
        ProtoStub_SetState(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_REPORT_STATS:
        ProtoStub_ReportStats(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_GET_STATS:
        ProtoStub_GetStats(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_DELETE_GATHERING:
        ProtoStub_DeleteGathering(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_GET_PENDING_DELETIONS:
        ProtoStub_GetPendingDeletions(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_DELETE_FROM_DELETIONS:
        ProtoStub_DeleteFromDeletions(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_MIGRATE_GATHERING_OWNERSHIP_V1:
        ProtoStub_MigrateGatheringOwnershipV1(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_FIND_BY_DESCRIPTION_LIKE:
        ProtoStub_FindByDescriptionLike(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_REGISTER_LOCAL_URL:
        ProtoStub_RegisterLocalURL(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_REGISTER_LOCAL_URLS:
        ProtoStub_RegisterLocalURLs(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_UPDATE_SESSION_HOST_V1:
        ProtoStub_UpdateSessionHostV1(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_GET_SESSION_URLS:
        ProtoStub_GetSessionURLs(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_UPDATE_SESSION_HOST:
        ProtoStub_UpdateSessionHost(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_UPDATE_GATHERING_OWNERSHIP:
        ProtoStub_UpdateGatheringOwnership(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_MIGRATE_GATHERING_OWNERSHIP:
        ProtoStub_MigrateGatheringOwnership(pRequest, pResponse, this);
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
