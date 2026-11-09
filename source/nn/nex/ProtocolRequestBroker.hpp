#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PROTOCOLREQUESTBROKER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PROTOCOLREQUESTBROKER_HPP

#include <functional>
#include <map>
#include <utility>

#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/ProtocolRequestBrokerInterface.hpp"
#include "nn/nex/SingleThreadCallPolicy.hpp"

namespace nn
{
namespace nex
{

class ProtocolRequestBroker : public ProtocolRequestBrokerInterface
{
public:
    ProtocolRequestBroker(ProtocolRequestBroker* pParentBroker);
    virtual ~ProtocolRequestBroker();

    virtual void ProcessMessage(CallProtocolMethodOperation*, EndPoint*, Buffer*);
    virtual void ProcessCallRequest(unsigned short, Message*, Message*, bool*, EndPoint*);
    virtual void ProcessCallResponse(unsigned short, Message*, EndPoint*);
    virtual bool RegisterProtocol(Protocol*);
    virtual void UnregisterProtocol(Protocol*);
    virtual Protocol* GetProtocol(unsigned short, Protocol::_Type);
    virtual bool CloneProtocols();
    virtual bool RegisterServerProtocol(unsigned short, ServerProtocol*);
    virtual bool UnregisterServerProtocol(unsigned short);
    virtual ServerProtocol* GetServerProtocol(unsigned short);
    virtual qResult DispatchLocalRMC(Message*, ClientProtocol*, ServerProtocol*, EndPoint*);
    virtual qResult DispatchBlockingLocalRMC(Message*, ClientProtocol*, ServerProtocol*, EndPoint*);
    virtual qResult DispatchThreadSafeLocalRMC(Message*, ClientProtocol*, ServerProtocol*, EndPoint*);
    void PeerDisconnected(EndPoint*);
    void FaultDetection(EndPoint*, unsigned int);
    Operation* GetCurrentOperation();
    void TraceProtocols(unsigned long long);
    static bool SystemStateIsValid(bool);

private:
    enum
    {
        RESULT_NOT_IMPLEMENTED = static_cast<int>(0x80010002u)
    };

    enum
    {
        NO_RESPONSE_PROTOCOL_ID = 1
    };

    enum
    {
        PROTOCOL_FLAG_THREAD_SAFE_RMC = 2
    };

    void ProcessMessageCore(CallProtocolMethodOperation*, EndPoint*, Buffer*);
    void DispatchCallToProtocol(ServerProtocol*, Message*, Message*, bool*, EndPoint*);
    void ProcessLocalCallRequest(ServerProtocol*, Message*, Message*, bool*, EndPoint*);

    typedef std::pair<const unsigned short, Protocol*> ProtocolEntry;
    typedef std::map<unsigned short, Protocol*, std::less<unsigned short>, MemAllocator<ProtocolEntry> > ProtocolMap;

    ProtocolMap m_clientProtocols;
    ProtocolMap m_serverProtocols;
    EndPoint* m_pCurrentEndPoint;
    ProtocolRequestBroker* m_pParentBroker;
    SingleThreadCallPolicy m_callPolicy;
};

}
}

#endif
