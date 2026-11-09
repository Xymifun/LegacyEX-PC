#include "nn/nex/MatchMakingProtocolExtClient.hpp"

#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/DDLClassMetaType.hpp"
#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/GatheringURLs.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/ParticipantDetails.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/ProtocolRegistry.hpp"
#include "nn/nex/ProtocolRequestBrokerInterface.hpp"
#include "nn/nex/RVClientCore.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

namespace
{

typedef void (*ProtoReturnFunction)(Message* pMessage, ProtocolCallContext* pCallContext);

const unsigned int METHOD_ID_MASK = 0x7FFF;
const unsigned int METHOD_COUNT = 6;
const unsigned int PROTOCOL_FLAG_UNLOCK_BEFORE_SEND = 0x10;

ProtoReturnFunction s_ProtoReturn[METHOD_COUNT];

}

MatchMakingProtocolExtClient::MatchMakingProtocolExtClient(unsigned short protocol_id) : ClientProtocol(1)
{
    if (protocol_id == 0)
        return;
    SetProtocolID(protocol_id);
    RVClientCore::GetInstance()->GetProtocolRegistry()->RegisterProtocol(protocol_id,
                                                                         new MatchMakingProtocolExtClient());
}

void MatchMakingProtocolExtClient::ExtractCallSpecificResults(Message* pMessage, ProtocolCallContext* pCallContext)
{
    const unsigned int uiMethodID = Protocol::ExtractMethodID(pMessage) & METHOD_ID_MASK;
    if (uiMethodID - 1 >= METHOD_COUNT)
    {
        ClientProtocol::SetCallError(qResult(RESULT_NOT_IMPLEMENTED));
        return;
    }
    if (s_ProtoReturn[uiMethodID - 1] != 0)
        s_ProtoReturn[uiMethodID - 1](pMessage, pCallContext);
}

Protocol* MatchMakingProtocolExtClient::CreateResponder() const
{
    return new MatchMakingProtocolExtClient();
}

bool MatchMakingProtocolExtClient::CallEndParticipation(ProtocolCallContext* pCallContext, bool* pSuccess,
                                                        const unsigned int& rGatheringID, const String& rMessage)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_END_PARTICIPATION);
    _Type_uint32::Add(&message, rGatheringID);
    _Type_string::Add(&message, rMessage);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_END_PARTICIPATION - 1] = ProtoReturn_EndParticipation;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolExtClient::EndParticipation_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                   bool* pSuccess, const unsigned int& rGatheringID,
                                                                   const String& rMessage)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_END_PARTICIPATION);
    _Type_uint32::Add(pMessage, rGatheringID);
    _Type_string::Add(pMessage, rMessage);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_END_PARTICIPATION - 1] = ProtoReturn_EndParticipation;
    return true;
}

bool MatchMakingProtocolExtClient::CallGetParticipants(ProtocolCallContext* pCallContext,
                                                       const unsigned int& rGatheringID,
                                                       qList<unsigned int>* pParticipants, const bool& rOnlyActive)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_PARTICIPANTS);
    _Type_uint32::Add(&message, rGatheringID);
    message << rOnlyActive;
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pParticipants);
    }
    s_ProtoReturn[METHOD_GET_PARTICIPANTS - 1] = ProtoReturn_GetParticipants;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolExtClient::GetParticipants_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                  const unsigned int& rGatheringID,
                                                                  qList<unsigned int>* pParticipants,
                                                                  const bool& rOnlyActive)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_GET_PARTICIPANTS);
    _Type_uint32::Add(pMessage, rGatheringID);
    *pMessage << rOnlyActive;
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pParticipants);
    }
    s_ProtoReturn[METHOD_GET_PARTICIPANTS - 1] = ProtoReturn_GetParticipants;
    return true;
}

bool MatchMakingProtocolExtClient::CallGetDetailedParticipants(ProtocolCallContext* pCallContext,
                                                               const unsigned int& rGatheringID,
                                                               qList<ParticipantDetails>* pParticipants,
                                                               const bool& rOnlyActive)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_DETAILED_PARTICIPANTS);
    _Type_uint32::Add(&message, rGatheringID);
    message << rOnlyActive;
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pParticipants);
    }
    s_ProtoReturn[METHOD_GET_DETAILED_PARTICIPANTS - 1] = ProtoReturn_GetDetailedParticipants;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolExtClient::GetDetailedParticipants_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                          Message* pMessage,
                                                                          const unsigned int& rGatheringID,
                                                                          qList<ParticipantDetails>* pParticipants,
                                                                          const bool& rOnlyActive)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_GET_DETAILED_PARTICIPANTS);
    _Type_uint32::Add(pMessage, rGatheringID);
    *pMessage << rOnlyActive;
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pParticipants);
    }
    s_ProtoReturn[METHOD_GET_DETAILED_PARTICIPANTS - 1] = ProtoReturn_GetDetailedParticipants;
    return true;
}

bool MatchMakingProtocolExtClient::CallGetParticipantsURLs(ProtocolCallContext* pCallContext,
                                                           const qList<unsigned int>& rGatheringIDs,
                                                           qList<GatheringURLs>* pGatheringURLs)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_PARTICIPANTS_URLS);
    _Type_std_list<_Type_uint32>::Add(&message, rGatheringIDs);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pGatheringURLs);
    }
    s_ProtoReturn[METHOD_GET_PARTICIPANTS_URLS - 1] = ProtoReturn_GetParticipantsURLs;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolExtClient::GetParticipantsURLs_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                      Message* pMessage,
                                                                      const qList<unsigned int>& rGatheringIDs,
                                                                      qList<GatheringURLs>* pGatheringURLs)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_GET_PARTICIPANTS_URLS);
    _Type_std_list<_Type_uint32>::Add(pMessage, rGatheringIDs);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pGatheringURLs);
    }
    s_ProtoReturn[METHOD_GET_PARTICIPANTS_URLS - 1] = ProtoReturn_GetParticipantsURLs;
    return true;
}

bool MatchMakingProtocolExtClient::CallGetGatheringRelations(ProtocolCallContext* pCallContext, String* pRelations,
                                                             const unsigned int& rGatheringID,
                                                             const String& rDescription)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_GET_GATHERING_RELATIONS);
    _Type_uint32::Add(&message, rGatheringID);
    _Type_string::Add(&message, rDescription);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pRelations);
    }
    s_ProtoReturn[METHOD_GET_GATHERING_RELATIONS - 1] = ProtoReturn_GetGatheringRelations;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolExtClient::GetGatheringRelations_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                        Message* pMessage, String* pRelations,
                                                                        const unsigned int& rGatheringID,
                                                                        const String& rDescription)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_GET_GATHERING_RELATIONS);
    _Type_uint32::Add(pMessage, rGatheringID);
    _Type_string::Add(pMessage, rDescription);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pRelations);
    }
    s_ProtoReturn[METHOD_GET_GATHERING_RELATIONS - 1] = ProtoReturn_GetGatheringRelations;
    return true;
}

bool MatchMakingProtocolExtClient::CallDeleteFromDeletions(ProtocolCallContext* pCallContext,
                                                           const qList<unsigned int>& rDeletionIDs,
                                                           const unsigned int& rPrincipalID)
{
    Message message;
    CriticalSection* pLock = &Scheduler::GetInstance()->GetCriticalSection();
    pLock->Enter();
    ProtocolRequestBrokerInterface::InitMessage(&message, m_uiProtocolID, COMMAND_REQUEST);
    if (!RegisterCallContext(&message, pCallContext))
    {
        pLock->Leave();
        return false;
    }
    AddMethodID(&message, METHOD_DELETE_FROM_DELETIONS);
    _Type_std_list<_Type_uint32>::Add(&message, rDeletionIDs);
    _Type_uint32::Add(&message, rPrincipalID);
    s_ProtoReturn[METHOD_DELETE_FROM_DELETIONS - 1] = ProtoReturn_DeleteFromDeletions;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolExtClient::DeleteFromDeletions_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                      Message* pMessage,
                                                                      const qList<unsigned int>& rDeletionIDs,
                                                                      const unsigned int& rPrincipalID)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_DELETE_FROM_DELETIONS);
    _Type_std_list<_Type_uint32>::Add(pMessage, rDeletionIDs);
    _Type_uint32::Add(pMessage, rPrincipalID);
    s_ProtoReturn[METHOD_DELETE_FROM_DELETIONS - 1] = ProtoReturn_DeleteFromDeletions;
    return true;
}

void MatchMakingProtocolExtClient::ProtoReturn_EndParticipation(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
}


void MatchMakingProtocolExtClient::ProtoReturn_GetParticipants(Message* pMessage, ProtocolCallContext* pCallContext)
{
    void* pList = pCallContext->GetReturnValuePtr(0);
    if (pList != 0)
    {
        _Type_std_list<_Type_uint32>::Extract(pMessage, static_cast<qList<unsigned int>*>(pList));
        return;
    }
    qList<unsigned int> discarded[1];
    _Type_std_list<_Type_uint32>::Extract(pMessage, &discarded[0]);
}

void MatchMakingProtocolExtClient::ProtoReturn_GetDetailedParticipants(Message* pMessage,
                                                                       ProtocolCallContext* pCallContext)
{
    void* pList = pCallContext->GetReturnValuePtr(0);
    if (pList != 0)
    {
        _Type_std_list<DDLClassMetaType<ParticipantDetails> >::Extract(pMessage,
                                                                       static_cast<qList<ParticipantDetails>*>(pList));
        return;
    }
    qList<ParticipantDetails> discarded[1];
    _Type_std_list<DDLClassMetaType<ParticipantDetails> >::Extract(pMessage, &discarded[0]);
}

void MatchMakingProtocolExtClient::ProtoReturn_GetParticipantsURLs(Message* pMessage, ProtocolCallContext* pCallContext)
{
    void* pList = pCallContext->GetReturnValuePtr(0);
    if (pList != 0)
    {
        _Type_std_list<DDLClassMetaType<GatheringURLs> >::Extract(pMessage, static_cast<qList<GatheringURLs>*>(pList));
        return;
    }
    qList<GatheringURLs> discarded[1];
    _Type_std_list<DDLClassMetaType<GatheringURLs> >::Extract(pMessage, &discarded[0]);
}

void MatchMakingProtocolExtClient::ProtoReturn_GetGatheringRelations(Message* pMessage,
                                                                     ProtocolCallContext* pCallContext)
{
    void* pRelations = pCallContext->GetReturnValuePtr(0);
    if (pRelations != 0)
    {
        _Type_string::Extract(pMessage, static_cast<String*>(pRelations));
        return;
    }
    String discarded[1];
    _Type_string::Extract(pMessage, &discarded[0]);
}

void MatchMakingProtocolExtClient::ProtoReturn_DeleteFromDeletions(Message*, ProtocolCallContext*)
{
}

}
}
