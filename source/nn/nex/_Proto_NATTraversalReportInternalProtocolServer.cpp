#include "nn/nex/_Proto_NATTraversalReportInternalProtocolServer.hpp"

#include <stdint.h>

#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int CALL_ERROR_UNKNOWN_METHOD = 0x80010002u;
const unsigned int RESPONSE_METHOD_FLAG = 0x8000;

}

void _Proto_NATTraversalReportInternalProtocolServer::DispatchProtocolMessage(Message* pRequest, Message* pResponse,
                                                                              bool* pResponseRequired, EndPoint*)
{
    const uint16_t uiMethodID = static_cast<uint16_t>(ExtractMethodID(pRequest));
    switch (uiMethodID)
    {
    default:
        SetCallError(qResult(static_cast<int>(CALL_ERROR_UNKNOWN_METHOD)));
        return;
    case METHOD_REPORT_NAT_TRAVERSAL_RESULT:
    {
        unsigned int uiCID;
        bool bResult;
        unsigned int uiRTT;
        pRequest->ExtractPrimitive(&uiCID);
        if (!pRequest->HasError())
        {
            *pRequest >> bResult;
            if (!pRequest->HasError())
            {
                pRequest->ExtractPrimitive(&uiRTT);
                if (!pRequest->HasError())
                {
                    ReportNATTraversalResult(uiCID, bResult, uiRTT);
                    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_REPORT_NAT_TRAVERSAL_RESULT);
                }
            }
        }
        break;
    }
    case METHOD_REPORT_NAT_PROPERTIES:
    {
        unsigned int uiNATMapping;
        unsigned int uiNATFiltering;
        unsigned int uiRTT;
        pRequest->ExtractPrimitive(&uiNATMapping);
        if (!pRequest->HasError())
        {
            pRequest->ExtractPrimitive(&uiNATFiltering);
            if (!pRequest->HasError())
            {
                pRequest->ExtractPrimitive(&uiRTT);
                if (!pRequest->HasError())
                {
                    ReportNATProperties(uiNATMapping, uiNATFiltering, uiRTT);
                    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_REPORT_NAT_PROPERTIES);
                }
            }
        }
        break;
    }
    case METHOD_REPORT_NAT_TRAVERSAL_RESULT_DETAIL:
    {
        unsigned int uiCID;
        bool bResult;
        int iConnectionType;
        unsigned int uiRTT;
        pRequest->ExtractPrimitive(&uiCID);
        if (!pRequest->HasError())
        {
            *pRequest >> bResult;
            if (!pRequest->HasError())
            {
                pRequest->ExtractPrimitive(&iConnectionType);
                if (!pRequest->HasError())
                {
                    pRequest->ExtractPrimitive(&uiRTT);
                    if (!pRequest->HasError())
                    {
                        ReportNATTraversalResultDetail(uiCID, bResult, iConnectionType, uiRTT);
                        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_REPORT_NAT_TRAVERSAL_RESULT_DETAIL);
                    }
                }
            }
        }
        break;
    }
    case 0:
        SetCallError(qResult(static_cast<int>(CALL_ERROR_UNKNOWN_METHOD)));
        return;
    }
    *pResponseRequired = true;
}

}
}
