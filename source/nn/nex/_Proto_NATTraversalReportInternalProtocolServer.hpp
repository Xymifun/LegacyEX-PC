#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__PROTO_NATTRAVERSALREPORTINTERNALPROTOCOLSERVER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__PROTO_NATTRAVERSALREPORTINTERNALPROTOCOLSERVER_HPP

#include "nn/nex/ServerProtocol.hpp"

namespace nn
{
namespace nex
{

class _Proto_NATTraversalReportInternalProtocolServer : public ServerProtocol
{
public:
    enum _MethodID
    {
        METHOD_REPORT_NAT_TRAVERSAL_RESULT = 1,
        METHOD_REPORT_NAT_PROPERTIES = 2,
        METHOD_REPORT_NAT_TRAVERSAL_RESULT_DETAIL = 3
    };

    explicit _Proto_NATTraversalReportInternalProtocolServer(unsigned int uiFlags) : ServerProtocol(uiFlags) {}

    virtual void DispatchProtocolMessage(Message* pRequest, Message* pResponse, bool* pResponseRequired,
                                         EndPoint* pEndPoint);
    virtual void ReportNATTraversalResult(const unsigned int& rCID, const bool& rResult, const unsigned int& rRTT) = 0;
    virtual void ReportNATProperties(const unsigned int& rNATMapping, const unsigned int& rNATFiltering,
                                     const unsigned int& rRTT) = 0;
    virtual void ReportNATTraversalResultDetail(const unsigned int& rCID, const bool& rResult,
                                                const int& rConnectionType, const unsigned int& rRTT) = 0;
};

}
}

#endif
