#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXNATRELAYINTERFACE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEXNATRELAYINTERFACE_HPP

#include "nn/nex/NATRelayInterface.hpp"

namespace nn
{
namespace nex
{
class CallContext;
class InetAddress;
class NATTraversalRelayClient;
class ProtocolCallContext;
class RelaySignatureKey;
class StationURL;
}
}

namespace nn
{
namespace pia
{
namespace inet
{

class NatRelayInterface;

class NexNatRelayInterface : public nex::NATRelayInterface
{
public:
    NexNatRelayInterface() : m_relayClient(0), m_natRelay(0), m_callContext(0) {}
    virtual ~NexNatRelayInterface() {}
    virtual void RegisterRelayClient(nex::NATTraversalRelayClient* relayClient);
    virtual void UnregisterRelayClient();
    virtual void RelayProbeRequest(const nex::qList<nex::StationURL>&) {}
    virtual void RelayProbeRequest(const nex::qList<nex::StationURL>& targetUrls, const nex::StationURL& sourceUrl);
    virtual void CheckCurrentPublicPort(nex::CallContext*, const nex::InetAddress&, bool) {}
    virtual void DetectSimplifiedNATProperties(nex::CallContext*, const nex::InetAddress&) {}
    virtual void RequestProbe(const nex::StationURL& targetUrl);
    virtual void ReportNATTraversalResult(const unsigned int& connectionId, const bool& succeeded,
                                          const unsigned int& rtt);
    virtual void ReportNATProperties(unsigned int mapping, unsigned int filtering, unsigned int portIncrement);
    virtual bool GetRelaySignatureKey(nex::CallContext* callContext, nex::RelaySignatureKey* key);
    virtual void ReportNATTraversalResultDetail(const unsigned int&, const bool&, const nex::NATTraversalResult&,
                                                unsigned int&)
    {
    }
    virtual void UpdateConnectionState(unsigned int connectionId, const nex::StationURL& stationUrl);
    virtual void AssociateRelay(NatRelayInterface* natRelay, nex::ProtocolCallContext* callContext)
    {
        m_natRelay = natRelay;
        m_callContext = callContext;
    }

private:
    nex::NATTraversalRelayClient* m_relayClient;
    NatRelayInterface* m_natRelay;
    nex::ProtocolCallContext* m_callContext;
};

}
}
}

#endif
