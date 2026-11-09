#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PROTOCOLREQUESTBROKERINTERFACE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PROTOCOLREQUESTBROKERINTERFACE_HPP

#include "nn/nex/BandwidthCounterMap.hpp"
#include "nn/nex/OperationManager.hpp"
#include "nn/nex/Protocol.hpp"

namespace nn
{
namespace nex
{

class ClientProtocol;
class ServerProtocol;

extern unsigned int (*g_protocolCurrentPID)();
extern unsigned int (*g_protocolCurrentCID)();

class ProtocolRequestBrokerInterface : public RootObject
{
public:
    ProtocolRequestBrokerInterface();
    virtual ~ProtocolRequestBrokerInterface();

    virtual void ProcessMessage(CallProtocolMethodOperation* operation, EndPoint* endPoint, Buffer* buffer) = 0;
    virtual void ProcessCallRequest(unsigned short protocolId, Message* request, Message* response,
                                    bool* responseRequired, EndPoint* endPoint) = 0;
    virtual void ProcessCallResponse(unsigned short protocolId, Message* response, EndPoint* endPoint) = 0;
    virtual bool RegisterProtocol(Protocol* protocol) = 0;
    virtual void UnregisterProtocol(Protocol* protocol) = 0;
    virtual Protocol* GetProtocol(unsigned short protocolId, Protocol::_Type type) = 0;
    virtual bool CloneProtocols() = 0;
    virtual bool RegisterServerProtocol(unsigned short protocolId, ServerProtocol* protocol) = 0;
    virtual bool UnregisterServerProtocol(unsigned short protocolId) = 0;
    virtual ServerProtocol* GetServerProtocol(unsigned short protocolId) = 0;
    virtual qResult DispatchLocalRMC(Message* message, ClientProtocol* client, ServerProtocol* server,
                                     EndPoint* endPoint) = 0;
    virtual qResult DispatchBlockingLocalRMC(Message* message, ClientProtocol* client, ServerProtocol* server,
                                             EndPoint* endPoint) = 0;
    virtual qResult DispatchThreadSafeLocalRMC(Message* message, ClientProtocol* client, ServerProtocol* server,
                                               EndPoint* endPoint) = 0;

    static void InitMessage(Message* message, unsigned short protocolId, Protocol::_Command command);
    void Receive(EndPoint* endPoint, Buffer* buffer);

protected:
    friend class Protocol;

    BandwidthCounterMap m_bandwidthCounters;
    OperationManager m_operationManager;
};

}
}

#endif
