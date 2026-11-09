#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NATTRAVERSALREPORTINTERNALPROTOCOLCLIENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NATTRAVERSALREPORTINTERNALPROTOCOLCLIENT_HPP

#include "nn/nex/ClientProtocol.hpp"

namespace nn
{
namespace nex
{

class NATTraversalReportInternalProtocolClient : public ClientProtocol
{
public:
    explicit NATTraversalReportInternalProtocolClient(unsigned short uiProtocolID = 0);

    virtual void ExtractCallSpecificResults(Message* pMessage, ProtocolCallContext* pCallContext);
    virtual Protocol* CreateResponder() const;

    static void ProtoReturn_ReportNATTraversalResult(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallReportNATTraversalResult(ProtocolCallContext* pCallContext, const unsigned int& rCID, const bool& rResult,
                                      const unsigned int& rRTT);
    static void ProtoReturn_ReportNATProperties(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallReportNATProperties(ProtocolCallContext* pCallContext, const unsigned int& rNATMapping,
                                 const unsigned int& rNATFiltering, const unsigned int& rRTT);
    static void ProtoReturn_ReportNATTraversalResultDetail(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallReportNATTraversalResultDetail(ProtocolCallContext* pCallContext, const unsigned int& rCID,
                                            const bool& rResult, const int& rConnectionType, const unsigned int& rRTT);

    static bool ReportNATTraversalResult_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                        const unsigned int& rCID, const bool& rResult,
                                                        const unsigned int& rRTT);
    static bool ReportNATProperties_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                   const unsigned int& rNATMapping, const unsigned int& rNATFiltering,
                                                   const unsigned int& rRTT);
    static bool ReportNATTraversalResultDetail_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                              const unsigned int& rCID, const bool& rResult,
                                                              const int& rConnectionType, const unsigned int& rRTT);
};

}
}

#endif
