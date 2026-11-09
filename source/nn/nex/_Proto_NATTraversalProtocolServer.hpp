#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__PROTO_NATTRAVERSALPROTOCOLSERVER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__PROTO_NATTRAVERSALPROTOCOLSERVER_HPP

#include "nn/nex/DateTime.hpp"
#include "nn/nex/ServerProtocol.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class _Proto_NATTraversalProtocolServer : public ServerProtocol
{
public:
    enum _MethodID
    {
        METHOD_REQUEST_PROBE_INITIATION = 1,
        METHOD_INITIATE_PROBE = 2,
        METHOD_REQUEST_PROBE_INITIATION_EXT = 3,
        METHOD_REPORT_NAT_TRAVERSAL_RESULT = 4,
        METHOD_REPORT_NAT_PROPERTIES = 5,
        METHOD_GET_RELAY_SIGNATURE_KEY = 6,
        METHOD_REPORT_NAT_TRAVERSAL_RESULT_DETAIL = 7
    };

    explicit _Proto_NATTraversalProtocolServer(unsigned int uiFlags) : ServerProtocol(uiFlags) {}

    virtual void DispatchProtocolMessage(Message* pRequest, Message* pResponse, bool* pResponseRequired,
                                         EndPoint* pEndPoint);
    virtual void RequestProbeInitiation(const qList<StationURL>& rTargetList) = 0;
    virtual void InitiateProbe(const StationURL& rStationToProbe) = 0;
    virtual void RequestProbeInitiationExt(const qList<StationURL>& rTargetList, const StationURL& rStationToProbe) = 0;
    virtual void ReportNATTraversalResult(const unsigned int& rCID, const bool& rResult, const unsigned int& rRTT) = 0;
    virtual void ReportNATProperties(const unsigned int& rNATMapping, const unsigned int& rNATFiltering,
                                     const unsigned int& rRTT) = 0;
    virtual void GetRelaySignatureKey(int* pRelayMode, DateTime* pCurrentUTCTime, String* pRelayAddress,
                                      unsigned short* pRelayPort, int* pRelayAddressType,
                                      unsigned int* pGameServerID) = 0;
    virtual void ReportNATTraversalResultDetail(const unsigned int& rCID, const bool& rResult,
                                                const int& rConnectionType, const unsigned int& rRTT) = 0;
};

}
}

#endif
