#include "nn/nex/_Proto_NATTraversalProtocolServer.hpp"

#include <stdint.h>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/_Type_qlist.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int CALL_ERROR_UNKNOWN_METHOD = 0x80010002u;
const unsigned int RESPONSE_METHOD_FLAG = 0x8000;

}

void _Proto_NATTraversalProtocolServer::DispatchProtocolMessage(Message* pRequest, Message* pResponse,
                                                                bool* pResponseRequired, EndPoint*)
{
    const uint16_t uiMethodID = static_cast<uint16_t>(ExtractMethodID(pRequest));
    switch (uiMethodID)
    {
    case METHOD_REQUEST_PROBE_INITIATION:
    {
        qList<StationURL> targetList;
        _Type_qlist<_Type_stationurl>::Extract(pRequest, &targetList);
        if (!pRequest->HasError())
        {
            RequestProbeInitiation(targetList);
            AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_REQUEST_PROBE_INITIATION);
        }
    }
        *pResponseRequired = true;
        return;
    case METHOD_INITIATE_PROBE:
    {
        StationURL stationToProbe;
        _Type_stationurl::Extract(pRequest, &stationToProbe);
        if (!pRequest->HasError())
        {
            InitiateProbe(stationToProbe);
            AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_INITIATE_PROBE);
        }
    }
        *pResponseRequired = true;
        return;
    case METHOD_REQUEST_PROBE_INITIATION_EXT:
    {
        qList<StationURL> targetList;
        _Type_qlist<_Type_stationurl>::Extract(pRequest, &targetList);
        if (!pRequest->HasError())
        {
            StationURL stationToProbe;
            _Type_stationurl::Extract(pRequest, &stationToProbe);
            if (!pRequest->HasError())
            {
                RequestProbeInitiationExt(targetList, stationToProbe);
                AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_REQUEST_PROBE_INITIATION_EXT);
            }
        }
    }
        *pResponseRequired = true;
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
        *pResponseRequired = true;
        return;
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
        *pResponseRequired = true;
        return;
    }
    case METHOD_GET_RELAY_SIGNATURE_KEY:
    {
        int iRelayMode;
        DateTime currentUTCTime;
        String relayAddress;
        unsigned short uiRelayPort;
        int iRelayAddressType;
        unsigned int uiGameServerID;
        GetRelaySignatureKey(&iRelayMode, &currentUTCTime, &relayAddress, &uiRelayPort, &iRelayAddressType,
                             &uiGameServerID);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GET_RELAY_SIGNATURE_KEY);
        pResponse->Append(reinterpret_cast<const unsigned char*>(&iRelayMode), sizeof(iRelayMode), 1);
        const uint64_t uiCurrentUTCTime = static_cast<const DateTime&>(currentUTCTime);
        pResponse->Append(reinterpret_cast<const unsigned char*>(&uiCurrentUTCTime), sizeof(uiCurrentUTCTime), 1);
        _Type_string::Add(pResponse, relayAddress);
        pResponse->Append(reinterpret_cast<const unsigned char*>(&uiRelayPort), sizeof(uiRelayPort), 1);
        pResponse->Append(reinterpret_cast<const unsigned char*>(&iRelayAddressType), sizeof(iRelayAddressType), 1);
        pResponse->Append(reinterpret_cast<const unsigned char*>(&uiGameServerID), sizeof(uiGameServerID), 1);
        *pResponseRequired = true;
        return;
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
        *pResponseRequired = true;
        return;
    }
    case 0:
    default:
        SetCallError(qResult(static_cast<int>(CALL_ERROR_UNKNOWN_METHOD)));
        break;
    }
}

}
}
