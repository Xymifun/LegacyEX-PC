#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NATRELAYINTERFACE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NATRELAYINTERFACE_HPP

#include "nn/nex/NATTraversalResult.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class CallContext;
class InetAddress;
class NATTraversalRelayClient;
class RelaySignatureKey;
class StationURL;

class NATRelayInterface : public RootObject
{
public:
    NATRelayInterface();
    virtual ~NATRelayInterface();

    virtual void RegisterRelayClient(NATTraversalRelayClient* pRelayClient) = 0;
    virtual void UnregisterRelayClient() = 0;
    virtual void RelayProbeRequest(const qList<StationURL>& rTargetURLs) = 0;
    virtual void RelayProbeRequest(const qList<StationURL>& rTargetURLs, const StationURL& rSourceURL) = 0;
    virtual void CheckCurrentPublicPort(CallContext* pCallContext, const InetAddress& rAddress, bool bForce) = 0;
    virtual void DetectSimplifiedNATProperties(CallContext* pCallContext, const InetAddress& rAddress) = 0;
    virtual void RequestProbe(const StationURL& rTargetURL) = 0;
    virtual void ReportNATTraversalResult(const unsigned int& rConnectionID, const bool& rSucceeded,
                                          const unsigned int& rRVConnectionID) = 0;
    virtual void ReportNATProperties(unsigned int uiMapping, unsigned int uiFiltering, unsigned int uiPortIncrease) = 0;
    virtual bool GetRelaySignatureKey(CallContext* pCallContext, RelaySignatureKey* pKey) = 0;
    virtual void ReportNATTraversalResultDetail(const unsigned int& rConnectionID, const bool& rSucceeded,
                                                const NATTraversalResult& rResult, unsigned int& rRVConnectionID) = 0;
    virtual void UpdateConnectionState(unsigned int uiConnectionID, const StationURL& rStationURL) = 0;
};

}
}

#endif
