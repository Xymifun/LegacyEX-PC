#include "nn/nex/MatchMakingProtocolClient.hpp"

#include "nn/nex/AnyGatheringAdapter.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/DDLClassMetaType.hpp"
#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/DeletionEntry.hpp"
#include "nn/nex/Gathering.hpp"
#include "nn/nex/GatheringStats.hpp"
#include "nn/nex/Invitation.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/ParticipantDetails.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/ProtocolRegistry.hpp"
#include "nn/nex/ProtocolRequestBrokerInterface.hpp"
#include "nn/nex/ResultRange.hpp"
#include "nn/nex/RVClientCore.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/StationURL.hpp"

namespace nn
{
namespace nex
{

namespace
{

typedef void (*ProtoReturnFunction)(Message* pMessage, ProtocolCallContext* pCallContext);
typedef _Type_any<DDLClassMetaType<Gathering>, _Type_string> GatheringAny;
typedef MatchMakingProtocolClient::GatheringHolder GatheringHolder;
typedef MatchMakingProtocolClient::GatheringHolderList GatheringHolderList;

const unsigned int METHOD_ID_MASK = 0x7FFF;
enum
{
    RESULT_NOT_IMPLEMENTED = static_cast<int>(0x80010002u)
};
const unsigned int METHOD_COUNT = 44;
const unsigned int METHOD_REGISTER_GATHERING = 1;
const unsigned int METHOD_UNREGISTER_GATHERING = 2;
const unsigned int METHOD_UNREGISTER_GATHERINGS = 3;
const unsigned int METHOD_UPDATE_GATHERING = 4;
const unsigned int METHOD_INVITE = 5;
const unsigned int METHOD_ACCEPT_INVITATION = 6;
const unsigned int METHOD_DECLINE_INVITATION = 7;
const unsigned int METHOD_CANCEL_INVITATION = 8;
const unsigned int METHOD_GET_INVITATIONS_SENT = 9;
const unsigned int METHOD_GET_INVITATIONS_RECEIVED = 10;
const unsigned int METHOD_PARTICIPATE = 11;
const unsigned int METHOD_CANCEL_PARTICIPATION = 12;
const unsigned int METHOD_GET_PARTICIPANTS = 13;
const unsigned int METHOD_ADD_PARTICIPANTS = 14;
const unsigned int METHOD_GET_DETAILED_PARTICIPANTS = 15;
const unsigned int METHOD_GET_PARTICIPANTSUR_LS = 16;
const unsigned int METHOD_FIND_BY_TYPE = 17;
const unsigned int METHOD_FIND_BY_DESCRIPTION = 18;
const unsigned int METHOD_FIND_BY_DESCRIPTION_REGEX = 19;
const unsigned int METHOD_FIND_BYID = 20;
const unsigned int METHOD_FIND_BY_SINGLEID = 21;
const unsigned int METHOD_FIND_BY_OWNER = 22;
const unsigned int METHOD_FIND_BY_PARTICIPANTS = 23;
const unsigned int METHOD_FIND_INVITATIONS = 24;
const unsigned int METHOD_FIND_BYSQL_QUERY = 25;
const unsigned int METHOD_LAUNCH_SESSION = 26;
const unsigned int METHOD_UPDATE_SESSION_URL = 27;
const unsigned int METHOD_GET_SESSION_URL = 28;
const unsigned int METHOD_GET_STATE = 29;
const unsigned int METHOD_SET_STATE = 30;
const unsigned int METHOD_REPORT_STATS = 31;
const unsigned int METHOD_GET_STATS = 32;
const unsigned int METHOD_DELETE_GATHERING = 33;
const unsigned int METHOD_GET_PENDING_DELETIONS = 34;
const unsigned int METHOD_DELETE_FROM_DELETIONS = 35;
const unsigned int METHOD_MIGRATE_GATHERING_OWNERSHIP_V1 = 36;
const unsigned int METHOD_FIND_BY_DESCRIPTION_LIKE = 37;
const unsigned int METHOD_REGISTER_LOCAL_URL = 38;
const unsigned int METHOD_REGISTER_LOCALUR_LS = 39;
const unsigned int METHOD_UPDATE_SESSION_HOST_V1 = 40;
const unsigned int METHOD_GET_SESSIONUR_LS = 41;
const unsigned int METHOD_UPDATE_SESSION_HOST = 42;
const unsigned int METHOD_UPDATE_GATHERING_OWNERSHIP = 43;
const unsigned int METHOD_MIGRATE_GATHERING_OWNERSHIP = 44;
const unsigned int PROTOCOL_FLAG_UNLOCK_BEFORE_SEND = 0x10;

ProtoReturnFunction s_ProtoReturn[METHOD_COUNT];

}

MatchMakingProtocolClient::MatchMakingProtocolClient(unsigned short protocol_id) : ClientProtocol(1)
{
    if (protocol_id == 0)
        return;
    SetProtocolID(protocol_id);
    RVClientCore::GetInstance()->GetProtocolRegistry()->RegisterProtocol(protocol_id, new MatchMakingProtocolClient());
}

void MatchMakingProtocolClient::ExtractCallSpecificResults(Message* pMessage, ProtocolCallContext* pCallContext)
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

Protocol* MatchMakingProtocolClient::CreateResponder() const
{
    return new MatchMakingProtocolClient();
}

bool MatchMakingProtocolClient::CallRegisterGathering(ProtocolCallContext* pCallContext, unsigned int* pGatheringID,
                                                      const GatheringHolder& rGathering)
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
    AddMethodID(&message, METHOD_REGISTER_GATHERING);
    GatheringAny::Add(&message, rGathering);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pGatheringID);
    }
    s_ProtoReturn[METHOD_REGISTER_GATHERING - 1] = ProtoReturn_RegisterGathering;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::RegisterGathering_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                 unsigned int* pGatheringID,
                                                                 const GatheringHolder& rGathering)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_REGISTER_GATHERING);
    GatheringAny::Add(pMessage, rGathering);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pGatheringID);
    }
    s_ProtoReturn[METHOD_REGISTER_GATHERING - 1] = ProtoReturn_RegisterGathering;
    return true;
}

bool MatchMakingProtocolClient::CallUnregisterGathering(ProtocolCallContext* pCallContext, bool* pSuccess,
                                                        const unsigned int& rGatheringID)
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
    AddMethodID(&message, METHOD_UNREGISTER_GATHERING);
    _Type_uint32::Add(&message, rGatheringID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_UNREGISTER_GATHERING - 1] = ProtoReturn_UnregisterGathering;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::UnregisterGathering_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                   bool* pSuccess, const unsigned int& rGatheringID)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_UNREGISTER_GATHERING);
    _Type_uint32::Add(pMessage, rGatheringID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_UNREGISTER_GATHERING - 1] = ProtoReturn_UnregisterGathering;
    return true;
}

bool MatchMakingProtocolClient::CallUnregisterGatherings(ProtocolCallContext* pCallContext, bool* pSuccess,
                                                         const qList<unsigned int>& rGatheringIDs)
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
    AddMethodID(&message, METHOD_UNREGISTER_GATHERINGS);
    _Type_std_list<_Type_uint32>::Add(&message, rGatheringIDs);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_UNREGISTER_GATHERINGS - 1] = ProtoReturn_UnregisterGatherings;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::UnregisterGatherings_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                    Message* pMessage, bool* pSuccess,
                                                                    const qList<unsigned int>& rGatheringIDs)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_UNREGISTER_GATHERINGS);
    _Type_std_list<_Type_uint32>::Add(pMessage, rGatheringIDs);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_UNREGISTER_GATHERINGS - 1] = ProtoReturn_UnregisterGatherings;
    return true;
}

bool MatchMakingProtocolClient::CallUpdateGathering(ProtocolCallContext* pCallContext, bool* pSuccess,
                                                    const GatheringHolder& rGathering)
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
    AddMethodID(&message, METHOD_UPDATE_GATHERING);
    GatheringAny::Add(&message, rGathering);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_UPDATE_GATHERING - 1] = ProtoReturn_UpdateGathering;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::UpdateGathering_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                               bool* pSuccess, const GatheringHolder& rGathering)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_UPDATE_GATHERING);
    GatheringAny::Add(pMessage, rGathering);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_UPDATE_GATHERING - 1] = ProtoReturn_UpdateGathering;
    return true;
}

bool MatchMakingProtocolClient::CallInvite(ProtocolCallContext* pCallContext, bool* pSuccess,
                                           const unsigned int& rGatheringID, const qList<unsigned int>& rPrincipalIDs,
                                           const String& rMessage)
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
    AddMethodID(&message, METHOD_INVITE);
    _Type_uint32::Add(&message, rGatheringID);
    _Type_std_list<_Type_uint32>::Add(&message, rPrincipalIDs);
    _Type_string::Add(&message, rMessage);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_INVITE - 1] = ProtoReturn_Invite;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::Invite_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                      bool* pSuccess, const unsigned int& rGatheringID,
                                                      const qList<unsigned int>& rPrincipalIDs, const String& rMessage)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_INVITE);
    _Type_uint32::Add(pMessage, rGatheringID);
    _Type_std_list<_Type_uint32>::Add(pMessage, rPrincipalIDs);
    _Type_string::Add(pMessage, rMessage);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_INVITE - 1] = ProtoReturn_Invite;
    return true;
}

bool MatchMakingProtocolClient::CallAcceptInvitation(ProtocolCallContext* pCallContext, bool* pSuccess,
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
    AddMethodID(&message, METHOD_ACCEPT_INVITATION);
    _Type_uint32::Add(&message, rGatheringID);
    _Type_string::Add(&message, rMessage);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_ACCEPT_INVITATION - 1] = ProtoReturn_AcceptInvitation;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::AcceptInvitation_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                bool* pSuccess, const unsigned int& rGatheringID,
                                                                const String& rMessage)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_ACCEPT_INVITATION);
    _Type_uint32::Add(pMessage, rGatheringID);
    _Type_string::Add(pMessage, rMessage);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_ACCEPT_INVITATION - 1] = ProtoReturn_AcceptInvitation;
    return true;
}

bool MatchMakingProtocolClient::CallDeclineInvitation(ProtocolCallContext* pCallContext, bool* pSuccess,
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
    AddMethodID(&message, METHOD_DECLINE_INVITATION);
    _Type_uint32::Add(&message, rGatheringID);
    _Type_string::Add(&message, rMessage);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_DECLINE_INVITATION - 1] = ProtoReturn_DeclineInvitation;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::DeclineInvitation_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                 bool* pSuccess, const unsigned int& rGatheringID,
                                                                 const String& rMessage)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_DECLINE_INVITATION);
    _Type_uint32::Add(pMessage, rGatheringID);
    _Type_string::Add(pMessage, rMessage);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_DECLINE_INVITATION - 1] = ProtoReturn_DeclineInvitation;
    return true;
}

bool MatchMakingProtocolClient::CallCancelInvitation(ProtocolCallContext* pCallContext, bool* pSuccess,
                                                     const unsigned int& rGatheringID,
                                                     const qList<unsigned int>& rPrincipalIDs, const String& rMessage)
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
    AddMethodID(&message, METHOD_CANCEL_INVITATION);
    _Type_uint32::Add(&message, rGatheringID);
    _Type_std_list<_Type_uint32>::Add(&message, rPrincipalIDs);
    _Type_string::Add(&message, rMessage);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_CANCEL_INVITATION - 1] = ProtoReturn_CancelInvitation;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::CancelInvitation_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                bool* pSuccess, const unsigned int& rGatheringID,
                                                                const qList<unsigned int>& rPrincipalIDs,
                                                                const String& rMessage)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_CANCEL_INVITATION);
    _Type_uint32::Add(pMessage, rGatheringID);
    _Type_std_list<_Type_uint32>::Add(pMessage, rPrincipalIDs);
    _Type_string::Add(pMessage, rMessage);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_CANCEL_INVITATION - 1] = ProtoReturn_CancelInvitation;
    return true;
}

bool MatchMakingProtocolClient::CallGetInvitationsSent(ProtocolCallContext* pCallContext,
                                                       const unsigned int& rGatheringID,
                                                       qList<Invitation>* pInvitations)
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
    AddMethodID(&message, METHOD_GET_INVITATIONS_SENT);
    _Type_uint32::Add(&message, rGatheringID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pInvitations);
    }
    s_ProtoReturn[METHOD_GET_INVITATIONS_SENT - 1] = ProtoReturn_GetInvitationsSent;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::GetInvitationsSent_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                  const unsigned int& rGatheringID,
                                                                  qList<Invitation>* pInvitations)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_GET_INVITATIONS_SENT);
    _Type_uint32::Add(pMessage, rGatheringID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pInvitations);
    }
    s_ProtoReturn[METHOD_GET_INVITATIONS_SENT - 1] = ProtoReturn_GetInvitationsSent;
    return true;
}

bool MatchMakingProtocolClient::CallGetInvitationsReceived(ProtocolCallContext* pCallContext,
                                                           qList<Invitation>* pInvitations)
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
    AddMethodID(&message, METHOD_GET_INVITATIONS_RECEIVED);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pInvitations);
    }
    s_ProtoReturn[METHOD_GET_INVITATIONS_RECEIVED - 1] = ProtoReturn_GetInvitationsReceived;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::GetInvitationsReceived_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                      Message* pMessage,
                                                                      qList<Invitation>* pInvitations)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_GET_INVITATIONS_RECEIVED);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pInvitations);
    }
    s_ProtoReturn[METHOD_GET_INVITATIONS_RECEIVED - 1] = ProtoReturn_GetInvitationsReceived;
    return true;
}

bool MatchMakingProtocolClient::CallParticipate(ProtocolCallContext* pCallContext, bool* pSuccess,
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
    AddMethodID(&message, METHOD_PARTICIPATE);
    _Type_uint32::Add(&message, rGatheringID);
    _Type_string::Add(&message, rMessage);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_PARTICIPATE - 1] = ProtoReturn_Participate;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::Participate_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                           bool* pSuccess, const unsigned int& rGatheringID,
                                                           const String& rMessage)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_PARTICIPATE);
    _Type_uint32::Add(pMessage, rGatheringID);
    _Type_string::Add(pMessage, rMessage);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_PARTICIPATE - 1] = ProtoReturn_Participate;
    return true;
}

bool MatchMakingProtocolClient::CallCancelParticipation(ProtocolCallContext* pCallContext, bool* pSuccess,
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
    AddMethodID(&message, METHOD_CANCEL_PARTICIPATION);
    _Type_uint32::Add(&message, rGatheringID);
    _Type_string::Add(&message, rMessage);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_CANCEL_PARTICIPATION - 1] = ProtoReturn_CancelParticipation;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::CancelParticipation_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                   bool* pSuccess, const unsigned int& rGatheringID,
                                                                   const String& rMessage)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_CANCEL_PARTICIPATION);
    _Type_uint32::Add(pMessage, rGatheringID);
    _Type_string::Add(pMessage, rMessage);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_CANCEL_PARTICIPATION - 1] = ProtoReturn_CancelParticipation;
    return true;
}

bool MatchMakingProtocolClient::CallGetParticipants(ProtocolCallContext* pCallContext, const unsigned int& rGatheringID,
                                                    qList<unsigned int>* pParticipantIDs)
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
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pParticipantIDs);
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

bool MatchMakingProtocolClient::GetParticipants_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                               const unsigned int& rGatheringID,
                                                               qList<unsigned int>* pParticipantIDs)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_GET_PARTICIPANTS);
    _Type_uint32::Add(pMessage, rGatheringID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pParticipantIDs);
    }
    s_ProtoReturn[METHOD_GET_PARTICIPANTS - 1] = ProtoReturn_GetParticipants;
    return true;
}

bool MatchMakingProtocolClient::CallAddParticipants(ProtocolCallContext* pCallContext, bool* pSuccess,
                                                    const unsigned int& rGatheringID,
                                                    const qList<unsigned int>& rPrincipalIDs, const String& rMessage)
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
    AddMethodID(&message, METHOD_ADD_PARTICIPANTS);
    _Type_uint32::Add(&message, rGatheringID);
    _Type_std_list<_Type_uint32>::Add(&message, rPrincipalIDs);
    _Type_string::Add(&message, rMessage);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_ADD_PARTICIPANTS - 1] = ProtoReturn_AddParticipants;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::AddParticipants_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                               bool* pSuccess, const unsigned int& rGatheringID,
                                                               const qList<unsigned int>& rPrincipalIDs,
                                                               const String& rMessage)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_ADD_PARTICIPANTS);
    _Type_uint32::Add(pMessage, rGatheringID);
    _Type_std_list<_Type_uint32>::Add(pMessage, rPrincipalIDs);
    _Type_string::Add(pMessage, rMessage);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_ADD_PARTICIPANTS - 1] = ProtoReturn_AddParticipants;
    return true;
}

bool MatchMakingProtocolClient::CallGetDetailedParticipants(ProtocolCallContext* pCallContext,
                                                            const unsigned int& rGatheringID,
                                                            qList<ParticipantDetails>* pParticipants)
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

bool MatchMakingProtocolClient::GetDetailedParticipants_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                       Message* pMessage,
                                                                       const unsigned int& rGatheringID,
                                                                       qList<ParticipantDetails>* pParticipants)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_GET_DETAILED_PARTICIPANTS);
    _Type_uint32::Add(pMessage, rGatheringID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pParticipants);
    }
    s_ProtoReturn[METHOD_GET_DETAILED_PARTICIPANTS - 1] = ProtoReturn_GetDetailedParticipants;
    return true;
}

bool MatchMakingProtocolClient::CallGetParticipantsURLs(ProtocolCallContext* pCallContext,
                                                        const unsigned int& rGatheringID, qList<StationURL>* pURLs)
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
    AddMethodID(&message, METHOD_GET_PARTICIPANTSUR_LS);
    _Type_uint32::Add(&message, rGatheringID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pURLs);
    }
    s_ProtoReturn[METHOD_GET_PARTICIPANTSUR_LS - 1] = ProtoReturn_GetParticipantsURLs;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::GetParticipantsURLs_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                   const unsigned int& rGatheringID,
                                                                   qList<StationURL>* pURLs)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_GET_PARTICIPANTSUR_LS);
    _Type_uint32::Add(pMessage, rGatheringID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pURLs);
    }
    s_ProtoReturn[METHOD_GET_PARTICIPANTSUR_LS - 1] = ProtoReturn_GetParticipantsURLs;
    return true;
}

bool MatchMakingProtocolClient::CallFindByType(ProtocolCallContext* pCallContext, const String& rType,
                                               const ResultRange& rRange, GatheringHolderList* pGatherings)
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
    AddMethodID(&message, METHOD_FIND_BY_TYPE);
    _Type_string::Add(&message, rType);
    _DDL_ResultRange::Add(&message, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pGatherings);
    }
    s_ProtoReturn[METHOD_FIND_BY_TYPE - 1] = ProtoReturn_FindByType;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::FindByType_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                          const String& rType, const ResultRange& rRange,
                                                          GatheringHolderList* pGatherings)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_FIND_BY_TYPE);
    _Type_string::Add(pMessage, rType);
    _DDL_ResultRange::Add(pMessage, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pGatherings);
    }
    s_ProtoReturn[METHOD_FIND_BY_TYPE - 1] = ProtoReturn_FindByType;
    return true;
}

bool MatchMakingProtocolClient::CallFindByDescription(ProtocolCallContext* pCallContext, const String& rDescription,
                                                      const ResultRange& rRange, GatheringHolderList* pGatherings)
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
    AddMethodID(&message, METHOD_FIND_BY_DESCRIPTION);
    _Type_string::Add(&message, rDescription);
    _DDL_ResultRange::Add(&message, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pGatherings);
    }
    s_ProtoReturn[METHOD_FIND_BY_DESCRIPTION - 1] = ProtoReturn_FindByDescription;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::FindByDescription_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                 const String& rDescription, const ResultRange& rRange,
                                                                 GatheringHolderList* pGatherings)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_FIND_BY_DESCRIPTION);
    _Type_string::Add(pMessage, rDescription);
    _DDL_ResultRange::Add(pMessage, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pGatherings);
    }
    s_ProtoReturn[METHOD_FIND_BY_DESCRIPTION - 1] = ProtoReturn_FindByDescription;
    return true;
}

bool MatchMakingProtocolClient::CallFindByDescriptionRegex(ProtocolCallContext* pCallContext,
                                                           const String& rDescription, const ResultRange& rRange,
                                                           GatheringHolderList* pGatherings)
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
    AddMethodID(&message, METHOD_FIND_BY_DESCRIPTION_REGEX);
    _Type_string::Add(&message, rDescription);
    _DDL_ResultRange::Add(&message, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pGatherings);
    }
    s_ProtoReturn[METHOD_FIND_BY_DESCRIPTION_REGEX - 1] = ProtoReturn_FindByDescriptionRegex;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::FindByDescriptionRegex_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                      Message* pMessage, const String& rDescription,
                                                                      const ResultRange& rRange,
                                                                      GatheringHolderList* pGatherings)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_FIND_BY_DESCRIPTION_REGEX);
    _Type_string::Add(pMessage, rDescription);
    _DDL_ResultRange::Add(pMessage, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pGatherings);
    }
    s_ProtoReturn[METHOD_FIND_BY_DESCRIPTION_REGEX - 1] = ProtoReturn_FindByDescriptionRegex;
    return true;
}

bool MatchMakingProtocolClient::CallFindByID(ProtocolCallContext* pCallContext,
                                             const qList<unsigned int>& rGatheringIDs, GatheringHolderList* pGatherings)
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
    AddMethodID(&message, METHOD_FIND_BYID);
    _Type_std_list<_Type_uint32>::Add(&message, rGatheringIDs);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pGatherings);
    }
    s_ProtoReturn[METHOD_FIND_BYID - 1] = ProtoReturn_FindByID;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::FindByID_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                        const qList<unsigned int>& rGatheringIDs,
                                                        GatheringHolderList* pGatherings)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_FIND_BYID);
    _Type_std_list<_Type_uint32>::Add(pMessage, rGatheringIDs);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pGatherings);
    }
    s_ProtoReturn[METHOD_FIND_BYID - 1] = ProtoReturn_FindByID;
    return true;
}

bool MatchMakingProtocolClient::CallFindBySingleID(ProtocolCallContext* pCallContext, const unsigned int& rGatheringID,
                                                   bool* pSuccess, GatheringHolder* pGathering)
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
    AddMethodID(&message, METHOD_FIND_BY_SINGLEID);
    _Type_uint32::Add(&message, rGatheringID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
        pCallContext->AddReturnValuePtr(pGathering);
    }
    s_ProtoReturn[METHOD_FIND_BY_SINGLEID - 1] = ProtoReturn_FindBySingleID;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::FindBySingleID_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                              const unsigned int& rGatheringID, bool* pSuccess,
                                                              GatheringHolder* pGathering)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_FIND_BY_SINGLEID);
    _Type_uint32::Add(pMessage, rGatheringID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
        pCallContext->AddReturnValuePtr(pGathering);
    }
    s_ProtoReturn[METHOD_FIND_BY_SINGLEID - 1] = ProtoReturn_FindBySingleID;
    return true;
}

bool MatchMakingProtocolClient::CallFindByOwner(ProtocolCallContext* pCallContext, const unsigned int& rOwnerID,
                                                const ResultRange& rRange, GatheringHolderList* pGatherings)
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
    AddMethodID(&message, METHOD_FIND_BY_OWNER);
    _Type_uint32::Add(&message, rOwnerID);
    _DDL_ResultRange::Add(&message, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pGatherings);
    }
    s_ProtoReturn[METHOD_FIND_BY_OWNER - 1] = ProtoReturn_FindByOwner;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::FindByOwner_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                           const unsigned int& rOwnerID, const ResultRange& rRange,
                                                           GatheringHolderList* pGatherings)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_FIND_BY_OWNER);
    _Type_uint32::Add(pMessage, rOwnerID);
    _DDL_ResultRange::Add(pMessage, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pGatherings);
    }
    s_ProtoReturn[METHOD_FIND_BY_OWNER - 1] = ProtoReturn_FindByOwner;
    return true;
}

bool MatchMakingProtocolClient::CallFindByParticipants(ProtocolCallContext* pCallContext,
                                                       const qList<unsigned int>& rPrincipalIDs,
                                                       GatheringHolderList* pGatherings)
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
    AddMethodID(&message, METHOD_FIND_BY_PARTICIPANTS);
    _Type_std_list<_Type_uint32>::Add(&message, rPrincipalIDs);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pGatherings);
    }
    s_ProtoReturn[METHOD_FIND_BY_PARTICIPANTS - 1] = ProtoReturn_FindByParticipants;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::FindByParticipants_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                  const qList<unsigned int>& rPrincipalIDs,
                                                                  GatheringHolderList* pGatherings)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_FIND_BY_PARTICIPANTS);
    _Type_std_list<_Type_uint32>::Add(pMessage, rPrincipalIDs);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pGatherings);
    }
    s_ProtoReturn[METHOD_FIND_BY_PARTICIPANTS - 1] = ProtoReturn_FindByParticipants;
    return true;
}

bool MatchMakingProtocolClient::CallFindInvitations(ProtocolCallContext* pCallContext, const ResultRange& rRange,
                                                    GatheringHolderList* pGatherings)
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
    AddMethodID(&message, METHOD_FIND_INVITATIONS);
    _DDL_ResultRange::Add(&message, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pGatherings);
    }
    s_ProtoReturn[METHOD_FIND_INVITATIONS - 1] = ProtoReturn_FindInvitations;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::FindInvitations_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                               const ResultRange& rRange,
                                                               GatheringHolderList* pGatherings)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_FIND_INVITATIONS);
    _DDL_ResultRange::Add(pMessage, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pGatherings);
    }
    s_ProtoReturn[METHOD_FIND_INVITATIONS - 1] = ProtoReturn_FindInvitations;
    return true;
}

bool MatchMakingProtocolClient::CallFindBySQLQuery(ProtocolCallContext* pCallContext, const String& rQuery,
                                                   const ResultRange& rRange, GatheringHolderList* pGatherings)
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
    AddMethodID(&message, METHOD_FIND_BYSQL_QUERY);
    _Type_string::Add(&message, rQuery);
    _DDL_ResultRange::Add(&message, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pGatherings);
    }
    s_ProtoReturn[METHOD_FIND_BYSQL_QUERY - 1] = ProtoReturn_FindBySQLQuery;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::FindBySQLQuery_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                              const String& rQuery, const ResultRange& rRange,
                                                              GatheringHolderList* pGatherings)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_FIND_BYSQL_QUERY);
    _Type_string::Add(pMessage, rQuery);
    _DDL_ResultRange::Add(pMessage, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pGatherings);
    }
    s_ProtoReturn[METHOD_FIND_BYSQL_QUERY - 1] = ProtoReturn_FindBySQLQuery;
    return true;
}

bool MatchMakingProtocolClient::CallLaunchSession(ProtocolCallContext* pCallContext, bool* pSuccess,
                                                  const unsigned int& rGatheringID, const String& rSessionURL)
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
    AddMethodID(&message, METHOD_LAUNCH_SESSION);
    _Type_uint32::Add(&message, rGatheringID);
    _Type_string::Add(&message, rSessionURL);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_LAUNCH_SESSION - 1] = ProtoReturn_LaunchSession;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::LaunchSession_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                             bool* pSuccess, const unsigned int& rGatheringID,
                                                             const String& rSessionURL)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_LAUNCH_SESSION);
    _Type_uint32::Add(pMessage, rGatheringID);
    _Type_string::Add(pMessage, rSessionURL);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_LAUNCH_SESSION - 1] = ProtoReturn_LaunchSession;
    return true;
}

bool MatchMakingProtocolClient::CallUpdateSessionURL(ProtocolCallContext* pCallContext, bool* pSuccess,
                                                     const unsigned int& rGatheringID, const String& rSessionURL)
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
    AddMethodID(&message, METHOD_UPDATE_SESSION_URL);
    _Type_uint32::Add(&message, rGatheringID);
    _Type_string::Add(&message, rSessionURL);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_UPDATE_SESSION_URL - 1] = ProtoReturn_UpdateSessionURL;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::UpdateSessionURL_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                bool* pSuccess, const unsigned int& rGatheringID,
                                                                const String& rSessionURL)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_UPDATE_SESSION_URL);
    _Type_uint32::Add(pMessage, rGatheringID);
    _Type_string::Add(pMessage, rSessionURL);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_UPDATE_SESSION_URL - 1] = ProtoReturn_UpdateSessionURL;
    return true;
}

bool MatchMakingProtocolClient::CallGetSessionURL(ProtocolCallContext* pCallContext, bool* pSuccess,
                                                  const unsigned int& rGatheringID, String* pSessionURL)
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
    AddMethodID(&message, METHOD_GET_SESSION_URL);
    _Type_uint32::Add(&message, rGatheringID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
        pCallContext->AddReturnValuePtr(pSessionURL);
    }
    s_ProtoReturn[METHOD_GET_SESSION_URL - 1] = ProtoReturn_GetSessionURL;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::GetSessionURL_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                             bool* pSuccess, const unsigned int& rGatheringID,
                                                             String* pSessionURL)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_GET_SESSION_URL);
    _Type_uint32::Add(pMessage, rGatheringID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
        pCallContext->AddReturnValuePtr(pSessionURL);
    }
    s_ProtoReturn[METHOD_GET_SESSION_URL - 1] = ProtoReturn_GetSessionURL;
    return true;
}

bool MatchMakingProtocolClient::CallGetState(ProtocolCallContext* pCallContext, bool* pSuccess,
                                             const unsigned int& rGatheringID, unsigned int* pState)
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
    AddMethodID(&message, METHOD_GET_STATE);
    _Type_uint32::Add(&message, rGatheringID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
        pCallContext->AddReturnValuePtr(pState);
    }
    s_ProtoReturn[METHOD_GET_STATE - 1] = ProtoReturn_GetState;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::GetState_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                        bool* pSuccess, const unsigned int& rGatheringID,
                                                        unsigned int* pState)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_GET_STATE);
    _Type_uint32::Add(pMessage, rGatheringID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
        pCallContext->AddReturnValuePtr(pState);
    }
    s_ProtoReturn[METHOD_GET_STATE - 1] = ProtoReturn_GetState;
    return true;
}

bool MatchMakingProtocolClient::CallSetState(ProtocolCallContext* pCallContext, bool* pSuccess,
                                             const unsigned int& rGatheringID, const unsigned int& rState)
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
    AddMethodID(&message, METHOD_SET_STATE);
    _Type_uint32::Add(&message, rGatheringID);
    _Type_uint32::Add(&message, rState);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_SET_STATE - 1] = ProtoReturn_SetState;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::SetState_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                        bool* pSuccess, const unsigned int& rGatheringID,
                                                        const unsigned int& rState)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_SET_STATE);
    _Type_uint32::Add(pMessage, rGatheringID);
    _Type_uint32::Add(pMessage, rState);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_SET_STATE - 1] = ProtoReturn_SetState;
    return true;
}

bool MatchMakingProtocolClient::CallReportStats(ProtocolCallContext* pCallContext, bool* pSuccess,
                                                const unsigned int& rGatheringID, const qList<GatheringStats>& rStats)
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
    AddMethodID(&message, METHOD_REPORT_STATS);
    _Type_uint32::Add(&message, rGatheringID);
    _Type_std_list<DDLClassMetaType<GatheringStats> >::Add(&message, rStats);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_REPORT_STATS - 1] = ProtoReturn_ReportStats;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::ReportStats_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                           bool* pSuccess, const unsigned int& rGatheringID,
                                                           const qList<GatheringStats>& rStats)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_REPORT_STATS);
    _Type_uint32::Add(pMessage, rGatheringID);
    _Type_std_list<DDLClassMetaType<GatheringStats> >::Add(pMessage, rStats);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_REPORT_STATS - 1] = ProtoReturn_ReportStats;
    return true;
}

bool MatchMakingProtocolClient::CallGetStats(ProtocolCallContext* pCallContext, bool* pSuccess,
                                             const unsigned int& rGatheringID, const qList<unsigned int>& rIDs,
                                             const qList<unsigned char>& rTypes, qList<GatheringStats>* pStats)
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
    AddMethodID(&message, METHOD_GET_STATS);
    _Type_uint32::Add(&message, rGatheringID);
    _Type_std_list<_Type_uint32>::Add(&message, rIDs);
    _Type_std_list<_Type_byte>::Add(&message, rTypes);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
        pCallContext->AddReturnValuePtr(pStats);
    }
    s_ProtoReturn[METHOD_GET_STATS - 1] = ProtoReturn_GetStats;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::GetStats_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                        bool* pSuccess, const unsigned int& rGatheringID,
                                                        const qList<unsigned int>& rIDs,
                                                        const qList<unsigned char>& rTypes,
                                                        qList<GatheringStats>* pStats)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_GET_STATS);
    _Type_uint32::Add(pMessage, rGatheringID);
    _Type_std_list<_Type_uint32>::Add(pMessage, rIDs);
    _Type_std_list<_Type_byte>::Add(pMessage, rTypes);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
        pCallContext->AddReturnValuePtr(pStats);
    }
    s_ProtoReturn[METHOD_GET_STATS - 1] = ProtoReturn_GetStats;
    return true;
}

bool MatchMakingProtocolClient::CallDeleteGathering(ProtocolCallContext* pCallContext, bool* pSuccess,
                                                    const unsigned int& rGatheringID)
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
    AddMethodID(&message, METHOD_DELETE_GATHERING);
    _Type_uint32::Add(&message, rGatheringID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_DELETE_GATHERING - 1] = ProtoReturn_DeleteGathering;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::DeleteGathering_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                               bool* pSuccess, const unsigned int& rGatheringID)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_DELETE_GATHERING);
    _Type_uint32::Add(pMessage, rGatheringID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_DELETE_GATHERING - 1] = ProtoReturn_DeleteGathering;
    return true;
}

bool MatchMakingProtocolClient::CallGetPendingDeletions(ProtocolCallContext* pCallContext, bool* pSuccess,
                                                        const unsigned int& rGatheringID, const ResultRange& rRange,
                                                        qList<DeletionEntry>* pDeletions)
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
    AddMethodID(&message, METHOD_GET_PENDING_DELETIONS);
    _Type_uint32::Add(&message, rGatheringID);
    _DDL_ResultRange::Add(&message, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
        pCallContext->AddReturnValuePtr(pDeletions);
    }
    s_ProtoReturn[METHOD_GET_PENDING_DELETIONS - 1] = ProtoReturn_GetPendingDeletions;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::GetPendingDeletions_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                   bool* pSuccess, const unsigned int& rGatheringID,
                                                                   const ResultRange& rRange,
                                                                   qList<DeletionEntry>* pDeletions)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_GET_PENDING_DELETIONS);
    _Type_uint32::Add(pMessage, rGatheringID);
    _DDL_ResultRange::Add(pMessage, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
        pCallContext->AddReturnValuePtr(pDeletions);
    }
    s_ProtoReturn[METHOD_GET_PENDING_DELETIONS - 1] = ProtoReturn_GetPendingDeletions;
    return true;
}

bool MatchMakingProtocolClient::CallDeleteFromDeletions(ProtocolCallContext* pCallContext, bool* pSuccess,
                                                        const qList<unsigned int>& rGatheringIDs)
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
    _Type_std_list<_Type_uint32>::Add(&message, rGatheringIDs);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
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

bool MatchMakingProtocolClient::DeleteFromDeletions_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                   bool* pSuccess,
                                                                   const qList<unsigned int>& rGatheringIDs)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_DELETE_FROM_DELETIONS);
    _Type_std_list<_Type_uint32>::Add(pMessage, rGatheringIDs);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_DELETE_FROM_DELETIONS - 1] = ProtoReturn_DeleteFromDeletions;
    return true;
}

bool MatchMakingProtocolClient::CallMigrateGatheringOwnershipV1(ProtocolCallContext* pCallContext, bool* pSuccess,
                                                                const unsigned int& rGatheringID,
                                                                const qList<unsigned int>& rCandidateIDs)
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
    AddMethodID(&message, METHOD_MIGRATE_GATHERING_OWNERSHIP_V1);
    _Type_uint32::Add(&message, rGatheringID);
    _Type_std_list<_Type_uint32>::Add(&message, rCandidateIDs);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_MIGRATE_GATHERING_OWNERSHIP_V1 - 1] = ProtoReturn_MigrateGatheringOwnershipV1;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::MigrateGatheringOwnershipV1_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                           Message* pMessage, bool* pSuccess,
                                                                           const unsigned int& rGatheringID,
                                                                           const qList<unsigned int>& rCandidateIDs)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_MIGRATE_GATHERING_OWNERSHIP_V1);
    _Type_uint32::Add(pMessage, rGatheringID);
    _Type_std_list<_Type_uint32>::Add(pMessage, rCandidateIDs);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pSuccess);
    }
    s_ProtoReturn[METHOD_MIGRATE_GATHERING_OWNERSHIP_V1 - 1] = ProtoReturn_MigrateGatheringOwnershipV1;
    return true;
}

bool MatchMakingProtocolClient::CallFindByDescriptionLike(ProtocolCallContext* pCallContext, const String& rDescription,
                                                          const ResultRange& rRange, GatheringHolderList* pGatherings)
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
    AddMethodID(&message, METHOD_FIND_BY_DESCRIPTION_LIKE);
    _Type_string::Add(&message, rDescription);
    _DDL_ResultRange::Add(&message, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pGatherings);
    }
    s_ProtoReturn[METHOD_FIND_BY_DESCRIPTION_LIKE - 1] = ProtoReturn_FindByDescriptionLike;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::FindByDescriptionLike_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                     Message* pMessage, const String& rDescription,
                                                                     const ResultRange& rRange,
                                                                     GatheringHolderList* pGatherings)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_FIND_BY_DESCRIPTION_LIKE);
    _Type_string::Add(pMessage, rDescription);
    _DDL_ResultRange::Add(pMessage, rRange);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pGatherings);
    }
    s_ProtoReturn[METHOD_FIND_BY_DESCRIPTION_LIKE - 1] = ProtoReturn_FindByDescriptionLike;
    return true;
}

bool MatchMakingProtocolClient::CallRegisterLocalURL(ProtocolCallContext* pCallContext,
                                                     const unsigned int& rGatheringID, const String& rSessionURL)
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
    AddMethodID(&message, METHOD_REGISTER_LOCAL_URL);
    _Type_uint32::Add(&message, rGatheringID);
    _Type_string::Add(&message, rSessionURL);
    s_ProtoReturn[METHOD_REGISTER_LOCAL_URL - 1] = ProtoReturn_RegisterLocalURL;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::RegisterLocalURL_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                const unsigned int& rGatheringID,
                                                                const String& rSessionURL)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_REGISTER_LOCAL_URL);
    _Type_uint32::Add(pMessage, rGatheringID);
    _Type_string::Add(pMessage, rSessionURL);
    s_ProtoReturn[METHOD_REGISTER_LOCAL_URL - 1] = ProtoReturn_RegisterLocalURL;
    return true;
}

bool MatchMakingProtocolClient::CallRegisterLocalURLs(ProtocolCallContext* pCallContext,
                                                      const unsigned int& rGatheringID,
                                                      const qList<StationURL>& rSessionURLs)
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
    AddMethodID(&message, METHOD_REGISTER_LOCALUR_LS);
    _Type_uint32::Add(&message, rGatheringID);
    _Type_std_list<_Type_stationurl>::Add(&message, rSessionURLs);
    s_ProtoReturn[METHOD_REGISTER_LOCALUR_LS - 1] = ProtoReturn_RegisterLocalURLs;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::RegisterLocalURLs_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                 const unsigned int& rGatheringID,
                                                                 const qList<StationURL>& rSessionURLs)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_REGISTER_LOCALUR_LS);
    _Type_uint32::Add(pMessage, rGatheringID);
    _Type_std_list<_Type_stationurl>::Add(pMessage, rSessionURLs);
    s_ProtoReturn[METHOD_REGISTER_LOCALUR_LS - 1] = ProtoReturn_RegisterLocalURLs;
    return true;
}

bool MatchMakingProtocolClient::CallUpdateSessionHostV1(ProtocolCallContext* pCallContext,
                                                        const unsigned int& rGatheringID)
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
    AddMethodID(&message, METHOD_UPDATE_SESSION_HOST_V1);
    _Type_uint32::Add(&message, rGatheringID);
    s_ProtoReturn[METHOD_UPDATE_SESSION_HOST_V1 - 1] = ProtoReturn_UpdateSessionHostV1;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::UpdateSessionHostV1_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                   const unsigned int& rGatheringID)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_UPDATE_SESSION_HOST_V1);
    _Type_uint32::Add(pMessage, rGatheringID);
    s_ProtoReturn[METHOD_UPDATE_SESSION_HOST_V1 - 1] = ProtoReturn_UpdateSessionHostV1;
    return true;
}

bool MatchMakingProtocolClient::CallGetSessionURLs(ProtocolCallContext* pCallContext, const unsigned int& rGatheringID,
                                                   qList<StationURL>* pURLs)
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
    AddMethodID(&message, METHOD_GET_SESSIONUR_LS);
    _Type_uint32::Add(&message, rGatheringID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pURLs);
    }
    s_ProtoReturn[METHOD_GET_SESSIONUR_LS - 1] = ProtoReturn_GetSessionURLs;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::GetSessionURLs_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                              const unsigned int& rGatheringID,
                                                              qList<StationURL>* pURLs)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_GET_SESSIONUR_LS);
    _Type_uint32::Add(pMessage, rGatheringID);
    if (pCallContext != 0)
    {
        pCallContext->AddReturnValuePtr(pURLs);
    }
    s_ProtoReturn[METHOD_GET_SESSIONUR_LS - 1] = ProtoReturn_GetSessionURLs;
    return true;
}

bool MatchMakingProtocolClient::CallUpdateSessionHost(ProtocolCallContext* pCallContext,
                                                      const unsigned int& rGatheringID, const bool& rFlag)
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
    AddMethodID(&message, METHOD_UPDATE_SESSION_HOST);
    _Type_uint32::Add(&message, rGatheringID);
    message << rFlag;
    s_ProtoReturn[METHOD_UPDATE_SESSION_HOST - 1] = ProtoReturn_UpdateSessionHost;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::UpdateSessionHost_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                                 const unsigned int& rGatheringID, const bool& rFlag)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_UPDATE_SESSION_HOST);
    _Type_uint32::Add(pMessage, rGatheringID);
    *pMessage << rFlag;
    s_ProtoReturn[METHOD_UPDATE_SESSION_HOST - 1] = ProtoReturn_UpdateSessionHost;
    return true;
}

bool MatchMakingProtocolClient::CallUpdateGatheringOwnership(ProtocolCallContext* pCallContext,
                                                             const unsigned int& rGatheringID, const bool& rFlag)
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
    AddMethodID(&message, METHOD_UPDATE_GATHERING_OWNERSHIP);
    _Type_uint32::Add(&message, rGatheringID);
    message << rFlag;
    s_ProtoReturn[METHOD_UPDATE_GATHERING_OWNERSHIP - 1] = ProtoReturn_UpdateGatheringOwnership;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::UpdateGatheringOwnership_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                        Message* pMessage,
                                                                        const unsigned int& rGatheringID,
                                                                        const bool& rFlag)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_UPDATE_GATHERING_OWNERSHIP);
    _Type_uint32::Add(pMessage, rGatheringID);
    *pMessage << rFlag;
    s_ProtoReturn[METHOD_UPDATE_GATHERING_OWNERSHIP - 1] = ProtoReturn_UpdateGatheringOwnership;
    return true;
}

bool MatchMakingProtocolClient::CallMigrateGatheringOwnership(ProtocolCallContext* pCallContext,
                                                              const unsigned int& rGatheringID,
                                                              const qList<unsigned int>& rCandidateIDs,
                                                              const bool& rFlag)
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
    AddMethodID(&message, METHOD_MIGRATE_GATHERING_OWNERSHIP);
    _Type_uint32::Add(&message, rGatheringID);
    _Type_std_list<_Type_uint32>::Add(&message, rCandidateIDs);
    message << rFlag;
    s_ProtoReturn[METHOD_MIGRATE_GATHERING_OWNERSHIP - 1] = ProtoReturn_MigrateGatheringOwnership;
    if (m_bLocalLoopback && FlagIsSet(PROTOCOL_FLAG_UNLOCK_BEFORE_SEND))
    {
        pLock->Leave();
        return SendRMCMessage(pCallContext, &message);
    }
    const bool bSent = SendRMCMessage(pCallContext, &message);
    pLock->Leave();
    return bSent;
}

bool MatchMakingProtocolClient::MigrateGatheringOwnership_PrepareRequest(ProtocolCallContext* pCallContext,
                                                                         Message* pMessage,
                                                                         const unsigned int& rGatheringID,
                                                                         const qList<unsigned int>& rCandidateIDs,
                                                                         const bool& rFlag)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (!RegisterCallContext(pMessage, pCallContext))
        return false;
    AddMethodID(pMessage, METHOD_MIGRATE_GATHERING_OWNERSHIP);
    _Type_uint32::Add(pMessage, rGatheringID);
    _Type_std_list<_Type_uint32>::Add(pMessage, rCandidateIDs);
    *pMessage << rFlag;
    s_ProtoReturn[METHOD_MIGRATE_GATHERING_OWNERSHIP - 1] = ProtoReturn_MigrateGatheringOwnership;
    return true;
}

void MatchMakingProtocolClient::ProtoReturn_RegisterGathering(Message* pMessage, ProtocolCallContext* pCallContext)
{
    unsigned int uiDiscardedGatheringID;
    unsigned int* pGatheringID = static_cast<unsigned int*>(pCallContext->GetReturnValuePtr(0));
    unsigned int* pDestination = &uiDiscardedGatheringID;
    if (pGatheringID != 0)
        pDestination = pGatheringID;
    _Type_uint32::Extract(pMessage, pDestination);
}

void MatchMakingProtocolClient::ProtoReturn_UnregisterGathering(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
}

void MatchMakingProtocolClient::ProtoReturn_UnregisterGatherings(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
}

void MatchMakingProtocolClient::ProtoReturn_UpdateGathering(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
}

void MatchMakingProtocolClient::ProtoReturn_Invite(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
}

void MatchMakingProtocolClient::ProtoReturn_AcceptInvitation(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
}

void MatchMakingProtocolClient::ProtoReturn_DeclineInvitation(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
}

void MatchMakingProtocolClient::ProtoReturn_CancelInvitation(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
}

void MatchMakingProtocolClient::ProtoReturn_GetInvitationsSent(Message* pMessage, ProtocolCallContext* pCallContext)
{
    void* pReturn = pCallContext->GetReturnValuePtr(0);
    if (pReturn != 0)
    {
        _Type_std_list<DDLClassMetaType<Invitation> >::Extract(pMessage, static_cast<qList<Invitation>*>(pReturn));
        return;
    }
    qList<Invitation> discarded[1];
    _Type_std_list<DDLClassMetaType<Invitation> >::Extract(pMessage, &discarded[0]);
}

void MatchMakingProtocolClient::ProtoReturn_GetInvitationsReceived(Message* pMessage, ProtocolCallContext* pCallContext)
{
    void* pReturn = pCallContext->GetReturnValuePtr(0);
    if (pReturn != 0)
    {
        _Type_std_list<DDLClassMetaType<Invitation> >::Extract(pMessage, static_cast<qList<Invitation>*>(pReturn));
        return;
    }
    qList<Invitation> discarded[1];
    _Type_std_list<DDLClassMetaType<Invitation> >::Extract(pMessage, &discarded[0]);
}

void MatchMakingProtocolClient::ProtoReturn_Participate(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
}

void MatchMakingProtocolClient::ProtoReturn_CancelParticipation(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
}

void MatchMakingProtocolClient::ProtoReturn_GetParticipants(Message* pMessage, ProtocolCallContext* pCallContext)
{
    qList<unsigned int>* pList = static_cast<qList<unsigned int>*>(pCallContext->GetReturnValuePtr(0));
    if (pList != 0)
    {
        _Type_std_list<_Type_uint32>::Extract(pMessage, pList);
        return;
    }
    qList<unsigned int> discarded[1];
    _Type_std_list<_Type_uint32>::Extract(pMessage, &discarded[0]);
}

void MatchMakingProtocolClient::ProtoReturn_AddParticipants(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
}

void MatchMakingProtocolClient::ProtoReturn_GetDetailedParticipants(Message* pMessage,
                                                                    ProtocolCallContext* pCallContext)
{
    qList<ParticipantDetails>* pList = static_cast<qList<ParticipantDetails>*>(pCallContext->GetReturnValuePtr(0));
    if (pList != 0)
    {
        _Type_std_list<DDLClassMetaType<ParticipantDetails> >::Extract(pMessage, pList);
    }
    else
    {
        qList<ParticipantDetails> discarded[1];
        _Type_std_list<DDLClassMetaType<ParticipantDetails> >::Extract(pMessage, &discarded[0]);
    }
}

void MatchMakingProtocolClient::ProtoReturn_GetParticipantsURLs(Message* pMessage, ProtocolCallContext* pCallContext)
{
    qList<StationURL>* pList = static_cast<qList<StationURL>*>(pCallContext->GetReturnValuePtr(0));
    if (pList != 0)
    {
        _Type_std_list<_Type_stationurl>::Extract(pMessage, pList);
    }
    else
    {
        qList<StationURL> discarded[1];
        _Type_std_list<_Type_stationurl>::Extract(pMessage, &discarded[0]);
    }
}

void MatchMakingProtocolClient::ProtoReturn_FindByType(Message* pMessage, ProtocolCallContext* pCallContext)
{
    void* pReturn = pCallContext->GetReturnValuePtr(0);
    if (pReturn != 0)
    {
        _Type_std_list<GatheringAny>::Extract(pMessage, static_cast<GatheringHolderList*>(pReturn));
        return;
    }
    GatheringHolderList discarded[1];
    _Type_std_list<GatheringAny>::Extract(pMessage, &discarded[0]);
}

void MatchMakingProtocolClient::ProtoReturn_FindByDescription(Message* pMessage, ProtocolCallContext* pCallContext)
{
    void* pReturn = pCallContext->GetReturnValuePtr(0);
    if (pReturn != 0)
    {
        _Type_std_list<GatheringAny>::Extract(pMessage, static_cast<GatheringHolderList*>(pReturn));
        return;
    }
    GatheringHolderList discarded[1];
    _Type_std_list<GatheringAny>::Extract(pMessage, &discarded[0]);
}

void MatchMakingProtocolClient::ProtoReturn_FindByDescriptionRegex(Message* pMessage, ProtocolCallContext* pCallContext)
{
    void* pReturn = pCallContext->GetReturnValuePtr(0);
    if (pReturn != 0)
    {
        _Type_std_list<GatheringAny>::Extract(pMessage, static_cast<GatheringHolderList*>(pReturn));
        return;
    }
    GatheringHolderList discarded[1];
    _Type_std_list<GatheringAny>::Extract(pMessage, &discarded[0]);
}

void MatchMakingProtocolClient::ProtoReturn_FindByID(Message* pMessage, ProtocolCallContext* pCallContext)
{
    void* pReturn = pCallContext->GetReturnValuePtr(0);
    if (pReturn != 0)
    {
        _Type_std_list<GatheringAny>::Extract(pMessage, static_cast<GatheringHolderList*>(pReturn));
        return;
    }
    GatheringHolderList discarded[1];
    _Type_std_list<GatheringAny>::Extract(pMessage, &discarded[0]);
}

void MatchMakingProtocolClient::ProtoReturn_FindBySingleID(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
    if (pMessage->HasError())
        return;
    GatheringHolder* pGathering = static_cast<GatheringHolder*>(pCallContext->GetReturnValuePtr(1));
    if (pGathering != 0)
    {
        GatheringAny::Extract(pMessage, pGathering);
    }
    else
    {
        GatheringHolder discarded[1];
        GatheringAny::Extract(pMessage, &discarded[0]);
    }
}

void MatchMakingProtocolClient::ProtoReturn_FindByOwner(Message* pMessage, ProtocolCallContext* pCallContext)
{
    void* pReturn = pCallContext->GetReturnValuePtr(0);
    if (pReturn != 0)
    {
        _Type_std_list<GatheringAny>::Extract(pMessage, static_cast<GatheringHolderList*>(pReturn));
        return;
    }
    GatheringHolderList discarded[1];
    _Type_std_list<GatheringAny>::Extract(pMessage, &discarded[0]);
}

void MatchMakingProtocolClient::ProtoReturn_FindByParticipants(Message* pMessage, ProtocolCallContext* pCallContext)
{
    void* pReturn = pCallContext->GetReturnValuePtr(0);
    if (pReturn != 0)
    {
        _Type_std_list<GatheringAny>::Extract(pMessage, static_cast<GatheringHolderList*>(pReturn));
        return;
    }
    GatheringHolderList discarded[1];
    _Type_std_list<GatheringAny>::Extract(pMessage, &discarded[0]);
}

void MatchMakingProtocolClient::ProtoReturn_FindInvitations(Message* pMessage, ProtocolCallContext* pCallContext)
{
    void* pReturn = pCallContext->GetReturnValuePtr(0);
    if (pReturn != 0)
    {
        _Type_std_list<GatheringAny>::Extract(pMessage, static_cast<GatheringHolderList*>(pReturn));
        return;
    }
    GatheringHolderList discarded[1];
    _Type_std_list<GatheringAny>::Extract(pMessage, &discarded[0]);
}

void MatchMakingProtocolClient::ProtoReturn_FindBySQLQuery(Message* pMessage, ProtocolCallContext* pCallContext)
{
    void* pReturn = pCallContext->GetReturnValuePtr(0);
    if (pReturn != 0)
    {
        _Type_std_list<GatheringAny>::Extract(pMessage, static_cast<GatheringHolderList*>(pReturn));
        return;
    }
    GatheringHolderList discarded[1];
    _Type_std_list<GatheringAny>::Extract(pMessage, &discarded[0]);
}

void MatchMakingProtocolClient::ProtoReturn_LaunchSession(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
}

void MatchMakingProtocolClient::ProtoReturn_UpdateSessionURL(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
}

void MatchMakingProtocolClient::ProtoReturn_GetSessionURL(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
    if (pMessage->HasError())
        return;
    String* pSessionURL = static_cast<String*>(pCallContext->GetReturnValuePtr(1));
    if (pSessionURL != 0)
    {
        _Type_string::Extract(pMessage, pSessionURL);
        return;
    }
    String discarded[1];
    _Type_string::Extract(pMessage, &discarded[0]);
}

void MatchMakingProtocolClient::ProtoReturn_GetState(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    unsigned int uiDiscardedState;
    void* pReturn = pCallContext->GetReturnValuePtr(0);
    bool* pSuccess = &bDiscardedSuccess;
    if (pReturn != 0)
        pSuccess = static_cast<bool*>(pReturn);
    *pMessage >> *pSuccess;
    if (pMessage->HasError())
        return;
    pReturn = pCallContext->GetReturnValuePtr(1);
    unsigned int* pState = &uiDiscardedState;
    if (pReturn != 0)
        pState = static_cast<unsigned int*>(pReturn);
    _Type_uint32::Extract(pMessage, pState);
}

void MatchMakingProtocolClient::ProtoReturn_SetState(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
}

void MatchMakingProtocolClient::ProtoReturn_ReportStats(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
}

void MatchMakingProtocolClient::ProtoReturn_GetStats(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
    if (pMessage->HasError())
        return;
    qList<GatheringStats>* pList = static_cast<qList<GatheringStats>*>(pCallContext->GetReturnValuePtr(1));
    if (pList != 0)
    {
        _Type_std_list<DDLClassMetaType<GatheringStats> >::Extract(pMessage, pList);
        return;
    }
    qList<GatheringStats> discarded[1];
    _Type_std_list<DDLClassMetaType<GatheringStats> >::Extract(pMessage, &discarded[0]);
}

void MatchMakingProtocolClient::ProtoReturn_DeleteGathering(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
}

void MatchMakingProtocolClient::ProtoReturn_GetPendingDeletions(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
    if (pMessage->HasError())
        return;
    void* pReturn = pCallContext->GetReturnValuePtr(1);
    if (pReturn != 0)
    {
        _Type_std_list<DDLClassMetaType<DeletionEntry> >::Extract(pMessage,
                                                                  static_cast<qList<DeletionEntry>*>(pReturn));
        return;
    }
    qList<DeletionEntry> discarded[1];
    _Type_std_list<DDLClassMetaType<DeletionEntry> >::Extract(pMessage, &discarded[0]);
}

void MatchMakingProtocolClient::ProtoReturn_DeleteFromDeletions(Message* pMessage, ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
}

void MatchMakingProtocolClient::ProtoReturn_MigrateGatheringOwnershipV1(Message* pMessage,
                                                                        ProtocolCallContext* pCallContext)
{
    bool bDiscardedSuccess;
    bool* pSuccess = static_cast<bool*>(pCallContext->GetReturnValuePtr(0));
    bool* pDestination = &bDiscardedSuccess;
    if (pSuccess != 0)
        pDestination = pSuccess;
    *pMessage >> *pDestination;
}

void MatchMakingProtocolClient::ProtoReturn_FindByDescriptionLike(Message* pMessage, ProtocolCallContext* pCallContext)
{
    void* pReturn = pCallContext->GetReturnValuePtr(0);
    if (pReturn != 0)
    {
        _Type_std_list<GatheringAny>::Extract(pMessage, static_cast<GatheringHolderList*>(pReturn));
        return;
    }
    GatheringHolderList discarded[1];
    _Type_std_list<GatheringAny>::Extract(pMessage, &discarded[0]);
}

void MatchMakingProtocolClient::ProtoReturn_RegisterLocalURL(Message* pMessage, ProtocolCallContext* pCallContext)
{
}

void MatchMakingProtocolClient::ProtoReturn_RegisterLocalURLs(Message* pMessage, ProtocolCallContext* pCallContext)
{
}

void MatchMakingProtocolClient::ProtoReturn_UpdateSessionHostV1(Message* pMessage, ProtocolCallContext* pCallContext)
{
}

void MatchMakingProtocolClient::ProtoReturn_GetSessionURLs(Message* pMessage, ProtocolCallContext* pCallContext)
{
    qList<StationURL>* pList = static_cast<qList<StationURL>*>(pCallContext->GetReturnValuePtr(0));
    if (pList != 0)
    {
        _Type_std_list<_Type_stationurl>::Extract(pMessage, pList);
    }
    else
    {
        qList<StationURL> discarded[1];
        _Type_std_list<_Type_stationurl>::Extract(pMessage, &discarded[0]);
    }
}

void MatchMakingProtocolClient::ProtoReturn_UpdateSessionHost(Message* pMessage, ProtocolCallContext* pCallContext)
{
}

void MatchMakingProtocolClient::ProtoReturn_UpdateGatheringOwnership(Message* pMessage,
                                                                     ProtocolCallContext* pCallContext)
{
}

void MatchMakingProtocolClient::ProtoReturn_MigrateGatheringOwnership(Message* pMessage,
                                                                      ProtocolCallContext* pCallContext)
{
}

}
}
