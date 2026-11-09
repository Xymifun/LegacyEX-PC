#ifndef SOURCE_NN_NEX_MATCHMAKINGPROTOCOLEXTCLIENT_HPP
#define SOURCE_NN_NEX_MATCHMAKINGPROTOCOLEXTCLIENT_HPP

#include "nn/nex/ClientProtocol.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class GatheringURLs;
class ParticipantDetails;

class MatchMakingProtocolExtClient : public ClientProtocol
{
public:
    explicit MatchMakingProtocolExtClient(unsigned short protocol_id = 0);

    virtual void ExtractCallSpecificResults(Message* pMessage, ProtocolCallContext* pCallContext);
    virtual Protocol* CreateResponder() const;

    bool CallEndParticipation(ProtocolCallContext* pCallContext, bool* pSuccess, const unsigned int& rGatheringID,
                              const String& rMessage);
    static bool EndParticipation_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pSuccess,
                                                const unsigned int& rGatheringID, const String& rMessage);
    bool CallGetParticipants(ProtocolCallContext* pCallContext, const unsigned int& rGatheringID,
                             qList<unsigned int>* pParticipants, const bool& rOnlyActive);
    static bool GetParticipants_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                               const unsigned int& rGatheringID, qList<unsigned int>* pParticipants,
                                               const bool& rOnlyActive);
    bool CallGetDetailedParticipants(ProtocolCallContext* pCallContext, const unsigned int& rGatheringID,
                                     qList<ParticipantDetails>* pParticipants, const bool& rOnlyActive);
    static bool GetDetailedParticipants_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                       const unsigned int& rGatheringID,
                                                       qList<ParticipantDetails>* pParticipants,
                                                       const bool& rOnlyActive);
    bool CallGetParticipantsURLs(ProtocolCallContext* pCallContext, const qList<unsigned int>& rGatheringIDs,
                                 qList<GatheringURLs>* pGatheringURLs);
    static bool GetParticipantsURLs_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                   const qList<unsigned int>& rGatheringIDs,
                                                   qList<GatheringURLs>* pGatheringURLs);
    bool CallGetGatheringRelations(ProtocolCallContext* pCallContext, String* pRelations,
                                   const unsigned int& rGatheringID, const String& rDescription);
    static bool GetGatheringRelations_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                     String* pRelations, const unsigned int& rGatheringID,
                                                     const String& rDescription);
    bool CallDeleteFromDeletions(ProtocolCallContext* pCallContext, const qList<unsigned int>& rDeletionIDs,
                                 const unsigned int& rPrincipalID);
    static bool DeleteFromDeletions_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                   const qList<unsigned int>& rDeletionIDs,
                                                   const unsigned int& rPrincipalID);

    static void ProtoReturn_EndParticipation(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_GetParticipants(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_GetDetailedParticipants(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_GetParticipantsURLs(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_GetGatheringRelations(Message* pMessage, ProtocolCallContext* pCallContext);
    static void ProtoReturn_DeleteFromDeletions(Message* pMessage, ProtocolCallContext* pCallContext);

private:
    enum
    {
        RESULT_NOT_IMPLEMENTED = static_cast<int>(0x80010002u)
    };
    enum MethodID
    {
        METHOD_END_PARTICIPATION = 1,
        METHOD_GET_PARTICIPANTS = 2,
        METHOD_GET_DETAILED_PARTICIPANTS = 3,
        METHOD_GET_PARTICIPANTS_URLS = 4,
        METHOD_GET_GATHERING_RELATIONS = 5,
        METHOD_DELETE_FROM_DELETIONS = 6
    };
};

}
}

#endif
