#include "nn/nex/ClientProtocol.hpp"

#include <cwchar>

#include "nn/nex/CallContext.hpp"
#include "nn/nex/Credentials.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/EndPoint.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/Platform.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/ProtocolRequestBrokerInterface.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/ServerProtocol.hpp"

namespace nn
{
namespace nex
{

namespace
{

enum
{
    RESULT_CORE_SUCCESS = 0x10001,
    RESULT_CORE_UNKNOWN = static_cast<int>(0x80010001u),
    RESULT_CORE_NOT_IMPLEMENTED = static_cast<int>(0x80010002u),
    RESULT_CORE_BUFFER_OVERFLOW = static_cast<int>(0x8001000Fu)
};

}

ClientProtocol::ClientProtocol(unsigned int uiFlags) : Protocol(uiFlags), m_pDefaultCredentials(0)
{
}

ClientProtocol::~ClientProtocol()
{
    SetDefaultCredentials(0);
}

void ClientProtocol::SetDefaultCredentials(Credentials* pCredentials)
{
    if (m_pDefaultCredentials != 0)
    {
        m_pDefaultCredentials->ReleaseRef();
        m_pDefaultCredentials = 0;
    }

    if (pCredentials == 0)
        return;

    m_pDefaultCredentials = pCredentials;
    m_pDefaultCredentials->AddRef();
}

void ClientProtocol::SetCallError(qResult)
{
    Platform::Breakpoint();
}

bool ClientProtocol::SendOverLocalLoopback(ProtocolCallContext* pCallContext, Message* pMessage)
{
    ServerProtocol* pServerProtocol = m_pProtocolRequestBroker->GetServerProtocol(m_uiProtocolID);
    if (pServerProtocol == 0)
    {
        pCallContext->SignalFailure(qResult(RESULT_CORE_NOT_IMPLEMENTED));
        return false;
    }

    Message request(pMessage->GetBuffer());
    _Command command;
    unsigned short uiProtocolID;
    ExtractProtocolKey(&request, command, uiProtocolID);
    qResult result = pServerProtocol->m_pProtocolRequestBroker->DispatchLocalRMC(&request, this, pServerProtocol, 0);
    if (!result)
    {
        if (pCallContext->GetState() == CallContext::CallInProgress)
            pCallContext->SignalFailure(result);

        return false;
    }

    return true;
}

bool ClientProtocol::SendRMCMessage(ProtocolCallContext* pCallContext, Message* pMessage)
{
    if (pCallContext == 0)
        return false;

    if (m_bLocalLoopback)
        return SendOverLocalLoopback(pCallContext, pMessage);

    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    EndPoint* pEndPoint;
    if (pCallContext->GetCredentials() != 0)
    {
        pEndPoint = pCallContext->GetCredentials()->GetConnection(GetProtocolID());
    }
    else
    {
        pEndPoint = GetOutgoingConnection();
        if (pEndPoint == 0 && m_pDefaultCredentials != 0)
            pEndPoint = m_pDefaultCredentials->GetConnection(GetProtocolID());
    }

    if (pEndPoint == 0)
    {
        pCallContext->SetStateImpl(CallContext::CallFailed, qResult(RESULT_CORE_UNKNOWN), false);
        return false;
    }

    Buffer* pBuffer = pMessage->GetBuffer();
    qResult result;
    {
        ScopedCS sendLock(&Scheduler::GetInstance()->GetCriticalSection());
        result = pEndPoint->_Send(pBuffer, 1, false, 0, 0);
    }

    if (result == false)
    {
        pCallContext->SignalFailure(result);
        return false;
    }

    pCallContext->SetDependentConnection(pEndPoint, pEndPoint->GetConnectionID());
    return true;
}

void ClientProtocol::ProcessResponse(Message* pMessage, EndPoint* pEndPoint)
{
    SetIncomingConnection(pEndPoint);
    if (IsOldRVDDLVersion(pEndPoint))
        pMessage->m_bCompactFormat = 1;

    qResult result;
    ExtractCallOutcome(pMessage, &result);
    ProtocolCallContext* pCallContext = RetrieveCallContext(pMessage);
    if (pCallContext != 0)
    {
        if (pMessage->HasError())
        {
            pCallContext->SignalFailure(qResult(RESULT_CORE_BUFFER_OVERFLOW));
            return;
        }

        if (!static_cast<bool>(result))
        {
            pCallContext->SignalFailure(result);
            return;
        }

        ExtractCallSpecificResults(pMessage, pCallContext);
        if (pMessage->HasError())
        {
            pCallContext->SignalFailure(qResult(RESULT_CORE_BUFFER_OVERFLOW));
            return;
        }

        pCallContext->SetStateImpl(CallContext::CallSucceeded, qResult(RESULT_CORE_SUCCESS), true);
    }
}

}
}
