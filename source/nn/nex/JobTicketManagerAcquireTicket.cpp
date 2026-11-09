#include "nn/nex/JobTicketManagerAcquireTicket.hpp"

#include "nn/nex/AuthenticationClient.hpp"
#include "nn/nex/BitStream.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/Credentials.hpp"
#include "nn/nex/DateTime.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/EndPoint.hpp"
#include "nn/nex/KerberosEncryption.hpp"
#include "nn/nex/Key.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/Protocol.hpp"
#include "nn/nex/ProtocolRequestBrokerInterface.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/Ticket.hpp"
#include "nn/nex/TicketGrantingProtocolClient.hpp"
#include "nn/nex/TicketManager.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned short TICKET_GRANTING_PROTOCOL_ID = 10;
const unsigned short AUTHENTICATION_CONNECTION_TYPE = 10;
const unsigned int TICKET_KEY_LENGTH = 0x20;
const unsigned int TICKET_KEY_BITS = 0x100;

enum AcquireTicketResult
{
    ACQUIRE_TICKET_SUCCEEDED_RESULT = 0x10001
};

}

JobTicketManagerAcquireTicket::JobTicketManagerAcquireTicket(unsigned int uiCallId, TicketManager* pTicketManager,
                                                             unsigned int uiSourcePrincipalID,
                                                             unsigned int uiTargetPrincipalID, Ticket** ppTicket)
    : StepSequenceJob(JOB_TYPE_2, DebugString(L"")),
      m_acquireContext(0),
      m_ticketData(),
      m_result()
{
    m_uiCallId = uiCallId;
    m_uiSourcePrincipalID = uiSourcePrincipalID;
    m_uiTargetPrincipalID = uiTargetPrincipalID;
    *ppTicket = 0;
    m_pTicketManager = pTicketManager;
    m_ppTicket = ppTicket;
    SetStep(Step(static_cast<Step::Function>(&JobTicketManagerAcquireTicket::PrepareCall), L""));
}

JobTicketManagerAcquireTicket::~JobTicketManagerAcquireTicket()
{
}

void JobTicketManagerAcquireTicket::CompleteJob(CallContext::State)
{
    SetToComplete();
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext != 0)
        pCallContext->SetStateImpl(CallContext::CallSucceeded, qResult(ACQUIRE_TICKET_SUCCEEDED_RESULT), true);
}

void JobTicketManagerAcquireTicket::AcquireTicketCompletionCallback(CallContext*, const UserContext* pUserContext)
{
    static_cast<JobTicketManagerAcquireTicket*>(pUserContext->GetPointer())->SetToReady();
}

void JobTicketManagerAcquireTicket::PrepareCall()
{
    TicketManager* pTicketManager = m_pTicketManager;
    Ticket* pTicket = pTicketManager->FindTicket(m_uiTargetPrincipalID);
    if (pTicket != 0)
    {
        *m_ppTicket = pTicket;
        pTicket->AddRef();
        CompleteJob(CallContext::CallSucceeded);
        return;
    }
    if (pTicketManager->m_pAuthenticationClient->m_pCredentials == 0)
    {
        CompleteJob(CallContext::CallFailed);
        return;
    }
    Message message;
    ProtocolRequestBrokerInterface::InitMessage(&message, TICKET_GRANTING_PROTOCOL_ID, Protocol::COMMAND_REQUEST);
    m_acquireContext.m_expirationTime = m_pTicketManager->m_pAuthenticationClient->m_loginDeadline;
    TicketGrantingProtocolClient::RequestTicket_PrepareRequest(
        &m_acquireContext, &message, &m_result, m_uiSourcePrincipalID, m_uiTargetPrincipalID, &m_ticketData);
    m_acquireContext.RegisterCompletionCallback(AcquireTicketCompletionCallback, UserContext(this), false);
    EndPoint* pConnection = m_pTicketManager->m_pAuthenticationClient->m_pCredentials->GetConnection(
        AUTHENTICATION_CONNECTION_TYPE);
    m_acquireContext.SetDependentConnection(pConnection, pConnection->GetConnectionID());
    Buffer* pBuffer = message.GetBuffer();
    CriticalSection& csScheduler = Scheduler::GetInstance()->GetCriticalSection();
    csScheduler.Enter();
    qResult sendResult = pConnection->_Send(pBuffer, 1, false, 0, 0);
    csScheduler.Leave();
    const bool bSent = sendResult;
    SetStep(Step(static_cast<Step::Function>(&JobTicketManagerAcquireTicket::ProcessResponse), L""));
    SetToSuspended();
}

void JobTicketManagerAcquireTicket::ProcessResponse()
{
    if (m_acquireContext.GetState() == CallContext::CallSucceeded)
    {
        Buffer decryptedTicket;
        if (!m_pTicketManager->m_pAuthenticationClient->m_pKerberosEncryption->Decrypt(m_ticketData, &decryptedTicket))
        {
            CompleteJob(CallContext::CallFailed);
            return;
        }
        Key sessionKey;
        Buffer ticketData;
        BitStream stream(&decryptedTicket);
        stream.ExtractRaw(sessionKey.PrepareContentPtr(TICKET_KEY_LENGTH), TICKET_KEY_BITS);
        unsigned int uiTargetPrincipalID;
        stream.Extract(reinterpret_cast<unsigned char*>(&uiTargetPrincipalID), sizeof(uiTargetPrincipalID), 1);
        stream >> ticketData;
        DateTime currentTime;
        DateTime::GetSystemTime(currentTime);
        Ticket* pTicket = new Ticket(ticketData, m_uiSourcePrincipalID, uiTargetPrincipalID, sessionKey, currentTime);
        m_pTicketManager->InsertTicket(uiTargetPrincipalID, pTicket);
        *m_ppTicket = pTicket;
        pTicket->AddRef();
    }
    CompleteJob(m_acquireContext.GetState());
}

}
}
