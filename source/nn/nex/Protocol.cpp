#include "nn/nex/Protocol.hpp"

#include <cwchar>

#include "nn/nex/Message.hpp"
#include "nn/nex/EndPoint.hpp"
#include "nn/nex/ProtocolRequestBrokerInterface.hpp"
#include "nn/nex/CallContext.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/CallProtocolMethodOperation.hpp"

namespace nn
{
namespace nex
{

struct ProtocolKey
{
    unsigned char m_uiCommand : 1;
    unsigned char m_uiProtocolID : 7;
};

Protocol::Protocol(unsigned int uiFlags)
    : SystemComponent(String(L"<unknown>")),
      m_uiProtocolID(0),
      m_pOutgoingConnection(0),
      m_pProtocolRequestBroker(0),
      m_uiFlags(uiFlags),
      m_pIncomingConnection(0),
      m_bLocalLoopback(false),
      m_uiCallerPID(0),
      m_uiCallerCID(0)
{
}

Protocol::~Protocol()
{
}

const wchar_t* Protocol::GetType() const
{
    return L"Protocol";
}

void Protocol::EnforceDeclareSysComponentMacro()
{
}

bool Protocol::FlagIsSet(unsigned int uiFlag) const
{
    return (m_uiFlags & uiFlag) == uiFlag;
}

void Protocol::SetFlag(unsigned int uiFlag)
{
    m_uiFlags |= uiFlag;
}

void Protocol::SetProtocolID(unsigned short uiProtocolID)
{
    m_uiProtocolID = uiProtocolID;
}

bool Protocol::BeginInitialization()
{
    if (FlagIsSet(1) && m_pProtocolRequestBroker != 0)
        m_pProtocolRequestBroker->RegisterProtocol(this);
    return true;
}

bool Protocol::BeginTermination()
{
    if (FlagIsSet(1) && m_pProtocolRequestBroker != 0)
        m_pProtocolRequestBroker->UnregisterProtocol(this);
    return true;
}

void Protocol::SetIncomingConnection(EndPoint* pEndPoint)
{
    if (m_bLocalLoopback && pEndPoint == 0)
        return;

    m_pIncomingConnection = pEndPoint;
    m_bLocalLoopback = false;
    if (pEndPoint == 0)
    {
        m_uiCallerPID = 0;
        m_uiCallerCID = 0;
        return;
    }

    m_uiCallerPID = pEndPoint->m_uiPrincipalID;
    m_uiCallerCID = pEndPoint->m_uiConnectionID;
}

EndPoint* Protocol::GetOutgoingConnection() const
{
    return m_pOutgoingConnection;
}

void Protocol::SetOutgoingConnection(EndPoint* pEndPoint)
{
    m_pOutgoingConnection = pEndPoint;
}

void Protocol::UseLocalLoopback(unsigned int uiCallerPID, unsigned int uiCallerCID)
{
    m_uiCallerPID = uiCallerPID;
    m_uiCallerCID = uiCallerCID;
    m_pIncomingConnection = 0;
    m_bLocalLoopback = true;
}

unsigned int Protocol::GetCallerPID() const
{
    return m_uiCallerPID;
}

unsigned int Protocol::GetCallerCID() const
{
    return m_uiCallerCID;
}

void Protocol::AssociateProtocolRequestBroker(ProtocolRequestBrokerInterface* pRequestBroker)
{
    m_pProtocolRequestBroker = pRequestBroker;
}

OperationManager* Protocol::GetOperationManager() const
{
    return &m_pProtocolRequestBroker->m_operationManager;
}

Operation* Protocol::GetCurrentOperation()
{
    return GetOperationManager()->GetCurrentOperation();
}

void Protocol::AddProtocolKey(Message* pMessage, _Command command, unsigned short uiProtocolID)
{
    if (uiProtocolID < 0x7F)
    {
        ProtocolKey key;
        key.m_uiCommand = command;
        key.m_uiProtocolID = uiProtocolID;
        pMessage->Append(reinterpret_cast<const unsigned char*>(&key), 1, 1);
    }
    else
    {
        ProtocolKey key;
        key.m_uiCommand = command;
        key.m_uiProtocolID = 0x7F;
        pMessage->Append(reinterpret_cast<const unsigned char*>(&key), 1, 1);
        pMessage->Append(reinterpret_cast<const unsigned char*>(&uiProtocolID), 2, 1);
    }
}

void Protocol::AddCallOutcome(Message* pMessage, qResult result)
{
    if (!static_cast<bool>(result))
    {
        unsigned char state = 0;
        pMessage->Append(&state, 1, 1);
        pMessage->Append(reinterpret_cast<const unsigned char*>(&result), 4, 1);
    }
    else
    {
        unsigned char state = 1;
        pMessage->Append(&state, 1, 1);
    }
}

void Protocol::AddCallContextID(Message* pMessage, unsigned int uiCallContextID)
{
    pMessage->Append(reinterpret_cast<const unsigned char*>(&uiCallContextID), 4, 1);
}

void Protocol::AddCallResponse(Message* pMessage, CallProtocolMethodOperation* pOperation, unsigned int uiCallContextID,
                               const Message& rResponse)
{
    qResult result = pOperation->m_result;
    AddCallOutcome(pMessage, result);
    AddCallContextID(pMessage, uiCallContextID);
    if (result)
        pMessage->AddDataFromMessage(rResponse);
}

void Protocol::ExtractProtocolKey(Message* pMessage, _Command& rCommand, unsigned short& rProtocolID)
{
    ProtocolKey key;
    pMessage->ExtractPrimitive(reinterpret_cast<unsigned char*>(&key));
    rProtocolID = key.m_uiProtocolID;
    rCommand = static_cast<_Command>(key.m_uiCommand);
    if (rProtocolID == 0x7F)
        pMessage->ExtractPrimitive(&rProtocolID);
}

void Protocol::ExtractCallOutcome(Message* pMessage, qResult* pResult)
{
    unsigned char state = 0xFF;
    pMessage->ExtractPrimitive(&state);
    if (state == 0)
    {
        int code;
        pMessage->ExtractPrimitive(&code);
        qResult result(code);
        *pResult = result;
    }
    else if (state == 1)
        *pResult = qResult(0x10001);
    else
        *pResult = qResult(static_cast<int>(0x80010001));
}

bool Protocol::RegisterCallContext(Message* pMessage, ProtocolCallContext* pCallContext)
{
    CallContext* pContext = reinterpret_cast<CallContext*>(pCallContext);
    if (!pContext->InitiateCall())
        return false;
    AddCallContextID(pMessage, pContext->m_uiCallId);
    return true;
}

unsigned int Protocol::ExtractCallContextID(Message* pMessage)
{
    unsigned int id;
    pMessage->ExtractPrimitive(&id);
    return id;
}

ProtocolCallContext* Protocol::RetrieveCallContext(Message* pMessage)
{
    const unsigned int id = ExtractCallContextID(pMessage);
    return reinterpret_cast<ProtocolCallContext*>(Core::GetInstance()->GetCallContextRegister()->GetCallContext(id));
}

bool Protocol::IsOldRVDDLVersion(EndPoint* pEndPoint)
{
    bool bIsOldVersion = false;
    if (pEndPoint != 0)
    {
        if (pEndPoint->m_uiPrincipalID - 0x6E < 10 || (pEndPoint->GetSupportedFunctionsFlags() & 0x08000000u) == 0)
            bIsOldVersion = true;
    }
    return bIsOldVersion;
}

unsigned int Protocol::ExtractMethodID(Message* pMessage)
{
    unsigned int uiMethodID;
    pMessage->ExtractPrimitive(&uiMethodID);
    return uiMethodID;
}

void Protocol::AddMethodID(Message* pMessage, unsigned int uiMethodID)
{
    unsigned int uiValue = uiMethodID;
    pMessage->Append(reinterpret_cast<const unsigned char*>(&uiValue), 4, 1);
}

}
}
