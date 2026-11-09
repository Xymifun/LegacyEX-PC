#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_RVNATRELAY_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_RVNATRELAY_HPP

#include "nn/nex/NATRelayInterface.hpp"
#include "nn/nex/RVConnectivityTester.hpp"
#include "nn/nex/RVNATEcho.hpp"

namespace nn
{
namespace nex
{

class NATTraversalEngine;

class RVNATRelay : public NATRelayInterface
{
public:
    RVNATRelay();
    virtual ~RVNATRelay();

    virtual void RegisterRelayClient(NATTraversalRelayClient* pRelayClient);
    virtual void UnregisterRelayClient();
    virtual void RelayProbeRequest(const qList<StationURL>& rTargetURLs);
    virtual void RelayProbeRequest(const qList<StationURL>& rTargetURLs, const StationURL& rSourceURL);
    virtual void CheckCurrentPublicPort(CallContext* pCallContext, const InetAddress& rAddress, bool bForce);
    virtual void DetectSimplifiedNATProperties(CallContext* pCallContext, const InetAddress& rAddress);
    virtual void RequestProbe(const StationURL& rTargetURL);
    virtual void ReportNATTraversalResult(const unsigned int& rConnectionID, const bool& rSucceeded,
                                          const unsigned int& rRVConnectionID);
    virtual void ReportNATProperties(unsigned int uiMapping, unsigned int uiFiltering, unsigned int uiPortIncrease);
    virtual bool GetRelaySignatureKey(CallContext* pCallContext, RelaySignatureKey* pKey);
    virtual void ReportNATTraversalResultDetail(const unsigned int& rConnectionID, const bool& rSucceeded,
                                                const NATTraversalResult& rResult, unsigned int& rRVConnectionID);
    virtual void UpdateConnectionState(unsigned int uiConnectionID, const StationURL& rStationURL);

    void AssociateEngine(NATTraversalEngine* pEngine);

private:
    NATTraversalRelayClient* m_pRelayClient;
    NATTraversalEngine* m_pEngine;
    RVNATEcho m_echo;
    RVConnectivityTester m_connectivityTester;
};

}
}

#endif
