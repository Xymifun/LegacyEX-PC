#include "nn/nex/ProtocolRequestBroker.hpp"

#include "nn/nex/CallProtocolMethodOperation.hpp"
#include "nn/nex/ClientProtocol.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/EndPoint.hpp"
#include "nn/nex/ServerProtocol.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/InetAddress.hpp"
#include "nn/nex/ProtocolRegistry.hpp"
#include "nn/nex/RVClientCore.hpp"
#include "nn/nex/SecurityContextManager.hpp"
#include "nn/nex/LogToClientController.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/StringStream.hpp"

namespace nn
{
namespace nex
{

ProtocolRequestBroker::~ProtocolRequestBroker()
{
}

bool ProtocolRequestBroker::RegisterProtocol(Protocol* protocol)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (protocol->m_pProtocolRequestBroker != this)
        protocol->AssociateProtocolRequestBroker(this);
    ProtocolMap& protocols = protocol->GetProtocolType() == Protocol::TYPE_SERVER ? m_serverProtocols
                                                                                  : m_clientProtocols;
    if (protocols.find(protocol->GetProtocolID()) != protocols.end())
        return false;
    protocols[protocol->GetProtocolID()] = protocol;
    return true;
}

void ProtocolRequestBroker::UnregisterProtocol(Protocol* protocol)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    ProtocolMap& protocols = protocol->GetProtocolType() == Protocol::TYPE_SERVER ? m_serverProtocols
                                                                                  : m_clientProtocols;
    ProtocolMap::iterator entry = protocols.find(protocol->GetProtocolID());
    if (entry != protocols.end())
        protocols.erase(entry);
}

Protocol* ProtocolRequestBroker::GetProtocol(unsigned short protocolId, Protocol::_Type type)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    ProtocolMap& protocols = type == Protocol::TYPE_SERVER ? m_serverProtocols : m_clientProtocols;
    ProtocolMap::iterator entry = protocols.find(protocolId);
    if (entry != protocols.end())
        return entry->second;
    if (m_pParentBroker == 0)
        return 0;
    Protocol* parentProtocol = m_pParentBroker->GetProtocol(protocolId, type);
    if (parentProtocol == 0)
        return 0;
    Protocol* clone = parentProtocol->Clone();
    if (clone->GetProtocolID() == 0)
        clone->SetProtocolID(parentProtocol->GetProtocolID());
    RegisterProtocol(clone);
    return clone;
}

bool ProtocolRequestBroker::RegisterServerProtocol(unsigned short protocolId, ServerProtocol* protocol)
{
    protocol->SetProtocolID(protocolId);
    RegisterProtocol(protocol);
    return true;
}

bool ProtocolRequestBroker::UnregisterServerProtocol(unsigned short protocolId)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    ProtocolMap::iterator entry = m_serverProtocols.find(protocolId);
    if (entry == m_serverProtocols.end())
        return false;
    m_serverProtocols.erase(entry);
    return true;
}

ServerProtocol* ProtocolRequestBroker::GetServerProtocol(unsigned short protocolId)
{
    return static_cast<ServerProtocol*>(GetProtocol(protocolId, Protocol::TYPE_SERVER));
}

bool ProtocolRequestBroker::CloneProtocols()
{
    if (m_pParentBroker == 0)
        return true;
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    for (ProtocolMap::iterator entry = m_pParentBroker->m_serverProtocols.begin();
         entry != m_pParentBroker->m_serverProtocols.end(); ++entry)
    {
        if (m_serverProtocols.find(entry->first) == m_serverProtocols.end())
        {
            Protocol* protocol = entry->second;
            Protocol* clone = protocol->Clone();
            if (clone != 0)
            {
                if (clone->GetProtocolID() == 0)
                    clone->SetProtocolID(protocol->GetProtocolID());
                RegisterProtocol(clone);
            }
        }
    }
    return true;
}

Operation* ProtocolRequestBroker::GetCurrentOperation()
{
    return m_operationManager.GetCurrentOperation();
}

qResult ProtocolRequestBroker::DispatchLocalRMC(Message* message, ClientProtocol* client, ServerProtocol* server,
                                                EndPoint* endPoint)
{
    qResult result = server->FlagIsSet(PROTOCOL_FLAG_THREAD_SAFE_RMC)
                         ? DispatchThreadSafeLocalRMC(message, client, server, endPoint)
                         : DispatchBlockingLocalRMC(message, client, server, endPoint);
    return result;
}

qResult ProtocolRequestBroker::DispatchBlockingLocalRMC(Message* message, ClientProtocol* client,
                                                        ServerProtocol* server, EndPoint* endPoint)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (server->m_pProtocolRequestBroker == 0)
        server->AssociateProtocolRequestBroker(this);
    return DispatchThreadSafeLocalRMC(message, client, server, endPoint);
}

void ProtocolRequestBroker::DispatchCallToProtocol(ServerProtocol* protocol, Message* request, Message* response,
                                                   bool* responseRequired, EndPoint* endPoint)
{
    if (request->m_bCompactFormat)
        response->m_bCompactFormat = true;
    CallPolicy& policy = m_callPolicy;
    policy.BeginCallImpl();
    int postponementDelay = 0;
    protocol->DispatchCallRequest(static_cast<CallProtocolMethodOperation*>(GetCurrentOperation())->GetAttemptCount(),
                                  request, response, responseRequired, &postponementDelay, endPoint);
    if (postponementDelay > 0)
        static_cast<CallProtocolMethodOperation*>(GetCurrentOperation())->PostponeOperation(postponementDelay);
    policy.EndCallImpl();
}

void ProtocolRequestBroker::ProcessLocalCallRequest(ServerProtocol* protocol, Message* request, Message* response,
                                                    bool* responseRequired, EndPoint* endPoint)
{
    unsigned int callContextId = Protocol::ExtractCallContextID(request);
    Message payload;
    *responseRequired = false;
    if (protocol == 0)
    {
        *responseRequired = true;
        CallProtocolMethodOperation* operation = static_cast<CallProtocolMethodOperation*>(GetCurrentOperation());
        operation->m_result = qResult(RESULT_NOT_IMPLEMENTED);
    }
    else
    {
        DispatchCallToProtocol(protocol, request, &payload, responseRequired, endPoint);
    }

    if (*responseRequired && !static_cast<CallProtocolMethodOperation*>(GetCurrentOperation())->m_postponed)
    {
        Protocol::AddCallResponse(response, static_cast<CallProtocolMethodOperation*>(GetCurrentOperation()),
                                  callContextId, payload);
    }
}

qResult ProtocolRequestBroker::DispatchThreadSafeLocalRMC(Message* message, ClientProtocol* client,
                                                          ServerProtocol* server, EndPoint* endPoint)
{
    Message response;
    bool responseRequired = true;
    if (server->m_pProtocolRequestBroker == 0)
        server->AssociateProtocolRequestBroker(this);
    if (client->m_bLocalLoopback)
        server->UseLocalLoopback(client->GetCallerPID(), client->GetCallerCID());
    qResult result;
    CallProtocolMethodOperation operation;
    operation.InitializeForNewCallAttempt();
    m_operationManager.OperationBegins(&operation);
    ProcessLocalCallRequest(server, message, &response, &responseRequired, endPoint);
    result = static_cast<CallProtocolMethodOperation*>(GetCurrentOperation())->m_result;
    m_operationManager.OperationEnds(&operation);
    if (responseRequired)
    {
        ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
        client->ProcessResponse(&response, 0);
    }
    return result;
}

void ProtocolRequestBroker::ProcessMessageCore(CallProtocolMethodOperation* operation, EndPoint* endPoint,
                                               Buffer* buffer)
{
    Message message(buffer);
    Protocol::_Command command;
    unsigned short protocolId;
    Protocol::ExtractProtocolKey(&message, command, protocolId);
    m_bandwidthCounters[protocolId]->m_incoming += buffer->GetLength();
    if (command == Protocol::COMMAND_REQUEST)
    {
        operation->InitializeForNewCallAttempt();
        m_operationManager.OperationBegins(operation);
        Message response;
        bool responseRequired = false;
        LogToClientController::Accumulate();
        ProcessCallRequest(protocolId, &message, &response, &responseRequired, endPoint);
        LogToClientController::Flush(endPoint);
        if (!operation->m_postponed && responseRequired)
        {
            Buffer* out = response.GetBuffer();
            {
                ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
                endPoint->_Send(out, 1, false, 0, 0);
            }
            m_bandwidthCounters[protocolId]->m_outgoing += out->GetLength();
        }
        m_operationManager.OperationEnds(operation);
    }
    else
    {
        ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
        if (m_pParentBroker == 0)
        {
            ProcessCallResponse(protocolId, &message, endPoint);
        }
        else
        {
            m_pParentBroker->ProcessCallResponse(protocolId, &message, endPoint);
        }
    }

    if (message.HasError())
    {
        StringStream stream;
        stream << L"ProtocolRequestBroker: Error processing message. pBuffer=" << buffer << L" ProtocolID=";
        stream.StreamNumber(protocolId) << L" CntSize=";
        stream.StreamNumber(buffer->GetLength()) << L" Size=";
        stream.StreamNumber(buffer->GetRealSize() - buffer->GetHeadShiftSize());
        if (endPoint != 0)
        {
            stream << L"  EndPoint: CID=";
            stream.StreamNumber(endPoint->m_uiConnectionID) << L" PID=";
            stream.StreamNumber(endPoint->m_uiPrincipalID) << L" URL=" << endPoint->m_stationURL;
        }
        else
        {
            stream << L"  <EndPoint is NULL>";
        }
    }
}

void ProtocolRequestBroker::ProcessMessage(CallProtocolMethodOperation* operation, EndPoint* endPoint, Buffer* buffer)
{
    m_pCurrentEndPoint = endPoint;
    ProcessMessageCore(operation, endPoint, buffer);
    m_pCurrentEndPoint = 0;
}

void ProtocolRequestBroker::ProcessCallRequest(unsigned short protocolId, Message* request, Message* response,
                                               bool* responseRequired, EndPoint* endPoint)
{
    ServerProtocol* server = GetServerProtocol(protocolId);
    if (server == 0 && protocolId == NO_RESPONSE_PROTOCOL_ID)
    {
        *responseRequired = false;
        return;
    }

    unsigned int callContextId = Protocol::ExtractCallContextID(request);
    Message payload;
    *responseRequired = false;
    if (server == 0)
    {
        *responseRequired = true;
        CallProtocolMethodOperation* operation = static_cast<CallProtocolMethodOperation*>(GetCurrentOperation());
        operation->m_result = qResult(RESULT_NOT_IMPLEMENTED);
    }
    else
    {
        DispatchCallToProtocol(server, request, &payload, responseRequired, endPoint);
    }

    if (*responseRequired)
    {
        Protocol::AddProtocolKey(response, Protocol::COMMAND_RESPONSE, protocolId);
        Protocol::AddCallResponse(response, static_cast<CallProtocolMethodOperation*>(GetCurrentOperation()),
                                  callContextId, payload);
    }
}

void ProtocolRequestBroker::ProcessCallResponse(unsigned short protocolId, Message* response, EndPoint* endPoint)
{
    ClientProtocol* client = static_cast<ClientProtocol*>(
        RVClientCore::GetInstance()->GetProtocolRegistry()->GetProtocol(protocolId, Protocol::TYPE_CLIENT));
    if (client != 0)
        client->ProcessResponse(response, endPoint);
}

void ProtocolRequestBroker::PeerDisconnected(EndPoint* endPoint)
{
    if (endPoint->m_uiConnectionID != 0 && endPoint->m_uiPrincipalID != 0)
    {
        {
            ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
            for (ProtocolMap::iterator entry = m_clientProtocols.begin(); entry != m_clientProtocols.end(); ++entry)
            {
                Protocol* protocol = entry->second;
                unsigned int address = 0;
                const StationURL* stationURL = &endPoint->m_stationURL;
                if (stationURL != 0)
                    address = stationURL->GetInetAddress()->GetAddress();
                SecurityContextManager::GetInstance()->Push(endPoint->m_uiPrincipalID, endPoint->m_uiConnectionID,
                                                            address);
                protocol->EndPointDisconnected(endPoint);
                SecurityContextManager::GetInstance()->Pop();
            }
        }
        {
            ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
            for (ProtocolMap::iterator entry = m_serverProtocols.begin(); entry != m_serverProtocols.end(); ++entry)
            {
                Protocol* protocol = entry->second;
                unsigned int address = 0;
                const StationURL* stationURL = &endPoint->m_stationURL;
                if (stationURL != 0)
                    address = stationURL->GetInetAddress()->GetAddress();
                SecurityContextManager::GetInstance()->Push(endPoint->m_uiPrincipalID, endPoint->m_uiConnectionID,
                                                            address);
                protocol->EndPointDisconnected(endPoint);
                SecurityContextManager::GetInstance()->Pop();
            }
        }
    }
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    Core::GetInstance()->GetCallContextRegister()->CancelCallContexts(endPoint, endPoint->GetConnectionID());
}

void ProtocolRequestBroker::FaultDetection(EndPoint* endPoint, unsigned int reason)
{
    if (endPoint->m_uiConnectionID != 0 && endPoint->m_uiPrincipalID != 0)
    {
        ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
        for (ProtocolMap::iterator entry = m_serverProtocols.begin(); entry != m_serverProtocols.end(); ++entry)
        {
            Protocol* protocol = entry->second;
            unsigned int address = 0;
            const StationURL* stationURL = &endPoint->m_stationURL;
            if (stationURL != 0)
                address = stationURL->GetInetAddress()->GetAddress();
            SecurityContextManager::GetInstance()->Push(endPoint->m_uiPrincipalID, endPoint->m_uiConnectionID, address);
            protocol->FaultDetected(endPoint, reason);
            SecurityContextManager::GetInstance()->Pop();
        }
    }
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    Core::GetInstance()->GetCallContextRegister()->CancelCallContexts(endPoint, endPoint->GetConnectionID());
}

}
}
