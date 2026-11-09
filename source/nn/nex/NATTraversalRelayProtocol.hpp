#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NATTRAVERSALRELAYPROTOCOL_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NATTRAVERSALRELAYPROTOCOL_HPP

#include "nn/nex/NATTraversalProtocolClient.hpp"
#include "nn/nex/NATTraversalReportInternalProtocolClient.hpp"
#include "nn/nex/_Proto_NATTraversalProtocolServer.hpp"

namespace nn
{
namespace nex
{

class Credentials;
class EndPoint;
class NATTraversalRelayClient;
class ProtocolRequestBrokerInterface;

class NATTraversalRelayProtocol : public _Proto_NATTraversalProtocolServer
{
public:
    NATTraversalRelayProtocol();
    virtual ~NATTraversalRelayProtocol();

    virtual void RequestProbeInitiation(const qList<StationURL>& rTargetList);
    virtual void InitiateProbe(const StationURL& rStationToProbe);
    virtual void RequestProbeInitiationExt(const qList<StationURL>& rTargetList, const StationURL& rStationToProbe);
    virtual void ReportNATTraversalResult(const unsigned int& rCID, const bool& rResult, const unsigned int& rRTT);
    virtual void ReportNATProperties(const unsigned int& rNATMapping, const unsigned int& rNATFiltering,
                                     const unsigned int& rRTT);
    virtual void GetRelaySignatureKey(int* pRelayMode, DateTime* pCurrentUTCTime, String* pRelayAddress,
                                      unsigned short* pRelayPort, int* pRelayAddressType, unsigned int* pGameServerID);
    virtual void ReportNATTraversalResultDetail(const unsigned int& rCID, const bool& rResult,
                                                const int& rConnectionType, const unsigned int& rRTT);
    virtual void SetDefaultCredentials(Credentials* pCredentials);

    void CloneRelayProtocol(NATTraversalRelayProtocol* pTarget) const;
    void RegisterProtocols(ProtocolRequestBrokerInterface* pRequestBroker);
    void UnregisterProtocols(ProtocolRequestBrokerInterface* pRequestBroker);
    void SetClientConnection(EndPoint* pEndPoint);

protected:
    friend class NATTraversalRelayClient;
    NATTraversalRelayClient* m_pRelayClient;
    NATTraversalProtocolClient* m_pProtocolClient;
    NATTraversalReportInternalProtocolClient m_reportInternalClient;
    Protocol* m_pCloneableProtocol;
};

}
}

#endif
