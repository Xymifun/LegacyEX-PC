#include "nn/nex/_Proto_MatchMakingProtocolExtServer.hpp"

#include <stdint.h>

#include "nn/nex/DDLClassMetaType.hpp"
#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/GatheringURLs.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/ParticipantDetails.hpp"

namespace nn
{
namespace nex
{

inline void _Proto_MatchMakingProtocolExtServer::ProtoStub_EndParticipation(
    Message* pRequest, Message* pResponse, _Proto_MatchMakingProtocolExtServer* pProtocol)
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
    const bool bResult = pProtocol->EndParticipation(uiGatheringID, message);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_END_PARTICIPATION);
    *pResponse << bResult;
}

inline void _Proto_MatchMakingProtocolExtServer::ProtoStub_GetParticipants(
    Message* pRequest, Message* pResponse, _Proto_MatchMakingProtocolExtServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    qList<unsigned int> participantIDs;
    bool bOnlyActive;
    *pRequest >> bOnlyActive;
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->GetParticipants(uiGatheringID, &participantIDs, bOnlyActive);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GET_PARTICIPANTS);
    _Type_std_list<_Type_uint32>::Add(pResponse, participantIDs);
}

inline void _Proto_MatchMakingProtocolExtServer::ProtoStub_GetDetailedParticipants(
    Message* pRequest, Message* pResponse, _Proto_MatchMakingProtocolExtServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    qList<ParticipantDetails> participants;
    bool bOnlyActive;
    *pRequest >> bOnlyActive;
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->GetDetailedParticipants(uiGatheringID, &participants, bOnlyActive);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GET_DETAILED_PARTICIPANTS);
    _Type_std_list<DDLClassMetaType<ParticipantDetails> >::Add(pResponse, participants);
}

inline void _Proto_MatchMakingProtocolExtServer::ProtoStub_GetParticipantsURLs(
    Message* pRequest, Message* pResponse, _Proto_MatchMakingProtocolExtServer* pProtocol)
{
    qList<unsigned int> gatheringIDs;
    _Type_std_list<_Type_uint32>::Extract(pRequest, &gatheringIDs);
    if (pRequest->HasError())
    {
        return;
    }
    qList<GatheringURLs> gatheringURLs;
    pProtocol->GetParticipantsURLs(gatheringIDs, &gatheringURLs);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GET_PARTICIPANTS_URLS);
    _Type_std_list<DDLClassMetaType<GatheringURLs> >::Add(pResponse, gatheringURLs);
}

inline void _Proto_MatchMakingProtocolExtServer::ProtoStub_GetGatheringRelations(
    Message* pRequest, Message* pResponse, _Proto_MatchMakingProtocolExtServer* pProtocol)
{
    unsigned int uiGatheringID;
    _Type_uint32::Extract(pRequest, &uiGatheringID);
    if (pRequest->HasError())
    {
        return;
    }
    String description;
    _Type_string::Extract(pRequest, &description);
    if (pRequest->HasError())
    {
        return;
    }
    const String relations = pProtocol->GetGatheringRelations(uiGatheringID, description);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GET_GATHERING_RELATIONS);
    _Type_string::Add(pResponse, relations);
}

inline void _Proto_MatchMakingProtocolExtServer::ProtoStub_DeleteFromDeletions(
    Message* pRequest, Message* pResponse, _Proto_MatchMakingProtocolExtServer* pProtocol)
{
    qList<unsigned int> deletionIDs;
    _Type_std_list<_Type_uint32>::Extract(pRequest, &deletionIDs);
    if (pRequest->HasError())
    {
        return;
    }
    unsigned int uiPrincipalID;
    _Type_uint32::Extract(pRequest, &uiPrincipalID);
    if (pRequest->HasError())
    {
        return;
    }
    pProtocol->DeleteFromDeletions(deletionIDs, uiPrincipalID);
    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_DELETE_FROM_DELETIONS);
}

void _Proto_MatchMakingProtocolExtServer::DispatchProtocolMessage(Message* pRequest, Message* pResponse,
                                                                  bool* pResponseRequired, EndPoint*)
{
    switch (static_cast<uint16_t>(ExtractMethodID(pRequest)))
    {
    case METHOD_END_PARTICIPATION:
        ProtoStub_EndParticipation(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_GET_PARTICIPANTS:
        ProtoStub_GetParticipants(pRequest, pResponse, this);
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
    case METHOD_GET_GATHERING_RELATIONS:
        ProtoStub_GetGatheringRelations(pRequest, pResponse, this);
        *pResponseRequired = true;
        return;
    case METHOD_DELETE_FROM_DELETIONS:
        ProtoStub_DeleteFromDeletions(pRequest, pResponse, this);
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
