#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NATTRAVERSALPROTOCOLCLIENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NATTRAVERSALPROTOCOLCLIENT_HPP

#include "nn/nex/ClientProtocol.hpp"
#include "nn/nex/DateTime.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class NATTraversalProtocolClient : public ClientProtocol
{
public:
    friend class NATTraversalRelayClient;
    explicit NATTraversalProtocolClient(unsigned short uiProtocolID = 0);

    virtual void ExtractCallSpecificResults(Message* pMessage, ProtocolCallContext* pCallContext);
    virtual Protocol* CreateResponder() const;

    static void ProtoReturn_RequestProbeInitiation(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallRequestProbeInitiation(ProtocolCallContext* pCallContext, const qList<StationURL>& rTargetList);
    static void ProtoReturn_InitiateProbe(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallInitiateProbe(ProtocolCallContext* pCallContext, const StationURL& rStationToProbe);
    static void ProtoReturn_RequestProbeInitiationExt(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallRequestProbeInitiationExt(ProtocolCallContext* pCallContext, const qList<StationURL>& rTargetList,
                                       const StationURL& rStationToProbe);
    static void ProtoReturn_ReportNATTraversalResult(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallReportNATTraversalResult(ProtocolCallContext* pCallContext, const unsigned int& rCID, const bool& rResult,
                                      const unsigned int& rRTT);
    static void ProtoReturn_ReportNATProperties(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallReportNATProperties(ProtocolCallContext* pCallContext, const unsigned int& rNATMapping,
                                 const unsigned int& rNATFiltering, const unsigned int& rRTT);
    static void ProtoReturn_GetRelaySignatureKey(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallGetRelaySignatureKey(ProtocolCallContext* pCallContext, int* pRelayMode, DateTime* pCurrentUTCTime,
                                  String* pRelayAddress, unsigned short* pRelayPort, int* pRelayAddressType,
                                  unsigned int* pGameServerID);
    static void ProtoReturn_ReportNATTraversalResultDetail(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallReportNATTraversalResultDetail(ProtocolCallContext* pCallContext, const unsigned int& rCID,
                                            const bool& rResult, const int& rConnectionType, const unsigned int& rRTT);

    static bool RequestProbeInitiation_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                      const qList<StationURL>& rTargetList);
    static bool InitiateProbe_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                             const StationURL& rStationToProbe);
    static bool RequestProbeInitiationExt_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                         const qList<StationURL>& rTargetList,
                                                         const StationURL& rStationToProbe);
    static bool ReportNATTraversalResult_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                        const unsigned int& rCID, const bool& rResult,
                                                        const unsigned int& rRTT);
    static bool ReportNATProperties_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                   const unsigned int& rNATMapping, const unsigned int& rNATFiltering,
                                                   const unsigned int& rRTT);
    static bool GetRelaySignatureKey_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                    int* pRelayMode, DateTime* pCurrentUTCTime, String* pRelayAddress,
                                                    unsigned short* pRelayPort, int* pRelayAddressType,
                                                    unsigned int* pGameServerID);
    static bool ReportNATTraversalResultDetail_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                              const unsigned int& rCID, const bool& rResult,
                                                              const int& rConnectionType, const unsigned int& rRTT);
};

}
}

#endif
