#include "nn/nex/ProtocolRegistry.hpp"

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

ProtocolRegistry::ProtocolRegistry()
{
}

ProtocolRegistry::~ProtocolRegistry()
{
    ProtocolMap::iterator it = m_protocols.begin();
    while (it != m_protocols.end())
    {
        delete it->second;
        ++it;
    }
    it = m_serverProtocols.begin();
    while (it != m_serverProtocols.end())
    {
        delete it->second;
        ++it;
    }
}

bool ProtocolRegistry::RegisterProtocol(Protocol* protocol)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    ProtocolMap& protocols = protocol->GetProtocolType() == Protocol::TYPE_SERVER ? m_serverProtocols : m_protocols;
    if (protocols.find(protocol->GetProtocolID()) != protocols.end())
    {
        delete protocol;
        return true;
    }
    protocols[protocol->GetProtocolID()] = protocol;
    return true;
}

bool ProtocolRegistry::RegisterProtocol(unsigned short protocolId, Protocol* protocol)
{
    protocol->SetProtocolID(protocolId);
    return RegisterProtocol(protocol);
}

Protocol* ProtocolRegistry::GetProtocol(unsigned short protocolId, Protocol::_Type type)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    ProtocolMap& protocols = type == Protocol::TYPE_SERVER ? m_serverProtocols : m_protocols;
    ProtocolMap::iterator it = protocols.find(protocolId);
    if (it == protocols.end())
        return 0;
    return it->second;
}

Protocol* ProtocolRegistry::GetServerProtocol(unsigned short protocolId)
{
    return GetProtocol(protocolId, Protocol::TYPE_SERVER);
}

void ProtocolRegistry::UnregisterProtocol(Protocol* protocol)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    ProtocolMap& protocols = protocol->GetProtocolType() == Protocol::TYPE_SERVER ? m_serverProtocols : m_protocols;
    ProtocolMap::iterator it = protocols.find(protocol->GetProtocolID());
    if (it != protocols.end())
        protocols.erase(it);
}

}
}
