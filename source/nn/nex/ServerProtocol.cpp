#include "nn/nex/ServerProtocol.hpp"

#include "nn/nex/CallProtocolMethodOperation.hpp"
#include "nn/nex/EndPoint.hpp"
#include "nn/nex/InetAddress.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/SecurityContextManager.hpp"
#include "nn/nex/StationURL.hpp"

#include <cwchar>

namespace nn
{
namespace nex
{

ServerProtocol::ServerProtocol(unsigned int uiFlags) : Protocol(uiFlags)
{
}

ServerProtocol::~ServerProtocol()
{
}

void ServerProtocol::SetCallError(qResult result)
{
    CallProtocolMethodOperation* pOperation = static_cast<CallProtocolMethodOperation*>(GetCurrentOperation());
    pOperation->m_result = qResult(result);
}

void ServerProtocol::DispatchCallRequest(unsigned int uiAttemptCount, Message* pRequest, Message* pResponse,
                                         bool* pResponseRequired, int* pAttempt, EndPoint* pEndPoint)
{
    if (pEndPoint == 0)
    {
        unsigned int uiPID = GetCallerPID();
        unsigned int uiCID = GetCallerCID();
        unsigned int uiAddress = SecurityContextManager::GetInstance()->GetCurrentAddress();
        SecurityContextManager::GetInstance()->Push(uiPID, uiCID, uiAddress);
    }
    else
    {
        unsigned int uiAddress = 0;
        const StationURL* pStationURL = &pEndPoint->m_stationURL;
        if (pStationURL != 0)
            uiAddress = pStationURL->GetInetAddress()->GetAddress();
        if (uiAddress == 0)
            uiAddress = SecurityContextManager::GetInstance()->GetCurrentAddress();
        SecurityContextManager::GetInstance()->Push(pEndPoint->m_uiPrincipalID, pEndPoint->GetConnectionID(),
                                                    uiAddress);
        SetIncomingConnection(pEndPoint);
    }

    if (IsOldRVDDLVersion(pEndPoint))
    {
        pRequest->m_bCompactFormat = true;
        pResponse->m_bCompactFormat = true;
    }

    if (FlagIsSet(2))
    {
        if (UseAttemptCountMethod())
            DispatchProtocolMessageWithAttemptCount(uiAttemptCount, pRequest, pResponse, pResponseRequired, pAttempt,
                                                    pEndPoint);
        else
            DispatchProtocolMessage(pRequest, pResponse, pResponseRequired, pEndPoint);
    }
    else
    {
        ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
        if (UseAttemptCountMethod())
            DispatchProtocolMessageWithAttemptCount(uiAttemptCount, pRequest, pResponse, pResponseRequired, pAttempt,
                                                    pEndPoint);
        else
            DispatchProtocolMessage(pRequest, pResponse, pResponseRequired, pEndPoint);
    }

    SetIncomingConnection(0);
    SecurityContextManager::GetInstance()->Pop();

    if (FlagIsSet(8))
        *pResponseRequired = false;
}

}
}
