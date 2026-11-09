#ifndef SOURCE_NN_NEX_PROTOCOLREGISTRY_HPP
#define SOURCE_NN_NEX_PROTOCOLREGISTRY_HPP

#include "nn/nex/Protocol.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/qMap.hpp"

namespace nn
{
namespace nex
{

class ProtocolRegistry : public RootObject
{
public:
    ProtocolRegistry();
    virtual ~ProtocolRegistry();
    bool RegisterProtocol(Protocol* protocol);
    bool RegisterProtocol(unsigned short protocolId, Protocol* protocol);
    Protocol* GetProtocol(unsigned short protocolId, Protocol::_Type type);
    Protocol* GetServerProtocol(unsigned short protocolId);
    void UnregisterProtocol(Protocol* protocol);

private:
    typedef qMap<unsigned short, Protocol*> ProtocolMap;
    ProtocolMap m_protocols;
    ProtocolMap m_serverProtocols;
};

}
}

#endif
