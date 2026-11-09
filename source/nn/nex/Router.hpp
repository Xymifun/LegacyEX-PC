#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_ROUTER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_ROUTER_HPP

#include "nn/nex/RootObject.hpp"
#include "nn/nex/RoutingStream.hpp"

namespace nn
{
namespace nex
{

class Buffer;
class InetAddress;
class RootTransport;
class RoutingAddressResolver;

class Router : public RootObject
{
public:
    Router();
    virtual ~Router();

    bool IsEnabled() const { return m_bEnabled; }

    void EnableRouting(bool bEnable);
    void SetTransport(RootTransport* pTransport);
    void UnregisterStream();
    bool ShouldRoute(const InetAddress* pAddress);
    bool HasRoute(const InetAddress* pAddress, const InetAddress* pSource);
    RoutingStream* GetRoutingStream() { return m_pRoutingStream; }
    void AddRoute(const InetAddress& rAddress, const InetAddress& rRoute, bool bReplace);
    bool RouteMessage(Buffer* pBuffer, const InetAddress* pAddress, unsigned short uiPort);
    void Clean();

    void PeriodicHouseCleaning() { m_pRoutingStream->m_routingTable.PeriodicHouseCleaning(); }

    unsigned int GetRoutingIPAddressTemplate();
    void ConstructAndAssociateRoutingAddress(unsigned short uiPort, InetAddress oAddress, InetAddress* pRoutingAddress);

private:
    friend class RoutingStream;

    bool m_bEnabled;
    bool m_bStreamRegistered;
    RoutingAddressResolver* m_pResolver;
    RoutingStream* m_pRoutingStream;
};

}
}

#endif
