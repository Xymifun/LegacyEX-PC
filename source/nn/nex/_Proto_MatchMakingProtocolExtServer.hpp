#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__PROTO_MATCHMAKINGPROTOCOLEXTSERVER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__PROTO_MATCHMAKINGPROTOCOLEXTSERVER_HPP

#include "nn/nex/ServerProtocol.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class GatheringURLs;
class ParticipantDetails;

class _Proto_MatchMakingProtocolExtServer : public ServerProtocol
{
public:
    enum _MethodID
    {
        METHOD_END_PARTICIPATION = 1,
        METHOD_GET_PARTICIPANTS = 2,
        METHOD_GET_DETAILED_PARTICIPANTS = 3,
        METHOD_GET_PARTICIPANTS_URLS = 4,
        METHOD_GET_GATHERING_RELATIONS = 5,
        METHOD_DELETE_FROM_DELETIONS = 6
    };

    explicit _Proto_MatchMakingProtocolExtServer(unsigned int uiFlags) : ServerProtocol(uiFlags) {}

    virtual void DispatchProtocolMessage(Message* pRequest, Message* pResponse, bool* pResponseRequired,
                                         EndPoint* pEndPoint);
    virtual bool EndParticipation(const unsigned int& rGatheringID, const String& rMessage) = 0;
    virtual void GetParticipants(const unsigned int& rGatheringID, qList<unsigned int>* pParticipantIDs,
                                 const bool& rOnlyActive) = 0;
    virtual void GetDetailedParticipants(const unsigned int& rGatheringID, qList<ParticipantDetails>* pParticipants,
                                         const bool& rOnlyActive) = 0;
    virtual void GetParticipantsURLs(const qList<unsigned int>& rGatheringIDs,
                                     qList<GatheringURLs>* pGatheringURLs) = 0;
    virtual String GetGatheringRelations(const unsigned int& rGatheringID, const String& rDescription) = 0;
    virtual void DeleteFromDeletions(const qList<unsigned int>& rDeletionIDs, const unsigned int& rPrincipalID) = 0;

private:
    enum
    {
        RESULT_NOT_IMPLEMENTED = static_cast<int>(0x80010002u),
        RESPONSE_METHOD_FLAG = 0x8000
    };

    static void ProtoStub_EndParticipation(Message* pRequest, Message* pResponse,
                                           _Proto_MatchMakingProtocolExtServer* pProtocol);
    static void ProtoStub_GetParticipants(Message* pRequest, Message* pResponse,
                                          _Proto_MatchMakingProtocolExtServer* pProtocol);
    static void ProtoStub_GetDetailedParticipants(Message* pRequest, Message* pResponse,
                                                  _Proto_MatchMakingProtocolExtServer* pProtocol);
    static void ProtoStub_GetParticipantsURLs(Message* pRequest, Message* pResponse,
                                              _Proto_MatchMakingProtocolExtServer* pProtocol);
    static void ProtoStub_GetGatheringRelations(Message* pRequest, Message* pResponse,
                                                _Proto_MatchMakingProtocolExtServer* pProtocol);
    static void ProtoStub_DeleteFromDeletions(Message* pRequest, Message* pResponse,
                                              _Proto_MatchMakingProtocolExtServer* pProtocol);
};

}
}

#endif
