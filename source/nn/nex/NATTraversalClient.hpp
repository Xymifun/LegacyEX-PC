#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NATTRAVERSALCLIENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NATTRAVERSALCLIENT_HPP

#include "nn/nex/NATTraversalRelayClient.hpp"
#include "nn/nex/RVNATRelay.hpp"
#include "nn/nex/ServiceClient.hpp"

namespace nn
{
namespace nex
{

class Credentials;

class NATTraversalClient : public ServiceClient
{
public:
    enum
    {
        SERVICE_ID = 3
    };

    NATTraversalClient();
    virtual ~NATTraversalClient();

    virtual bool IsConnected() const;
    virtual void ConnectionStateHasChanged() {}

    virtual void UpdateProtocolsDefaultCredentials(Credentials* pCredentials);

private:
    NATTraversalRelayClient m_relayClient;
    RVNATRelay m_relay;
};

}
}

#endif
