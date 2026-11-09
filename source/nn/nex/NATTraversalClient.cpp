#include "nn/nex/NATTraversalClient.hpp"

#include "nn/nex/Network.hpp"

namespace nn
{
namespace nex
{

NATTraversalClient::NATTraversalClient() : ServiceClient(SERVICE_ID)
{
    m_relayClient.Init();
    m_relayClient.RegisterRelayInterface(&m_relay);
    m_relay.AssociateEngine(Network::GetInstance()->GetNATTraversalEngine());
}

NATTraversalClient::~NATTraversalClient()
{
    m_relayClient.UnregisterRelayInterface();
    m_relay.AssociateEngine(0);
}

bool NATTraversalClient::IsConnected() const
{
    return m_relayClient.IsConnected();
}

void NATTraversalClient::UpdateProtocolsDefaultCredentials(Credentials* pCredentials)
{
    if (pCredentials != 0)
        m_relayClient.Bind(pCredentials);
    else
        m_relayClient.Unbind();
}

}
}
