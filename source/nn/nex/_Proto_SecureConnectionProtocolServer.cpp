#include "nn/nex/_Proto_SecureConnectionProtocolServer.hpp"

#include <stdint.h>

#include "nn/nex/ConnectionData.hpp"
#include "nn/nex/DDLClassMetaType.hpp"
#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Data.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/StationURL.hpp"
#include "nn/nex/_Type_qresult.hpp"
#include "nn/nex/qBuffer.hpp"

namespace nn
{
namespace nex
{

_Proto_SecureConnectionProtocolServer::~_Proto_SecureConnectionProtocolServer()
{
}

void _Proto_SecureConnectionProtocolServer::DispatchProtocolMessage(Message* pRequest, Message* pResponse,
                                                                    bool* pResponseRequired, EndPoint*)
{
    const uint16_t uiMethodID = static_cast<uint16_t>(ExtractMethodID(pRequest));
    switch (uiMethodID)
    {
    case METHOD_REGISTER:
    {
        {
            qList<StationURL> urls;
            _Type_std_list<_Type_stationurl>::Extract(pRequest, &urls);
            if (!pRequest->HasError())
            {
                unsigned int uiConnectionID;
                StationURL publicURL;
                const qResult result = Register(urls, &uiConnectionID, &publicURL);
                AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_REGISTER);
                _Type_qresult::Add(pResponse, result);
                _Type_uint32::Add(pResponse, uiConnectionID);
                _Type_stationurl::Add(pResponse, publicURL);
            }
        }
        *pResponseRequired = true;
        return;
    }
    case METHOD_REQUEST_CONNECTION_DATA:
    {
        unsigned int uiConnectionID;
        _Type_uint32::Extract(pRequest, &uiConnectionID);
        if (!pRequest->HasError())
        {
            unsigned int uiPrincipalID;
            _Type_uint32::Extract(pRequest, &uiPrincipalID);
            if (!pRequest->HasError())
            {
                {
                    qList<ConnectionData> connectionData;
                    const bool bSuccess = RequestConnectionData(uiConnectionID, uiPrincipalID, &connectionData);
                    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_REQUEST_CONNECTION_DATA);
                    *pResponse << bSuccess;
                    _Type_std_list<DDLClassMetaType<ConnectionData> >::Add(pResponse, connectionData);
                }
                *pResponseRequired = true;
                return;
            }
        }
        break;
    }
    case METHOD_REQUEST_URLS:
    {
        unsigned int uiConnectionID;
        _Type_uint32::Extract(pRequest, &uiConnectionID);
        if (!pRequest->HasError())
        {
            unsigned int uiPrincipalID;
            _Type_uint32::Extract(pRequest, &uiPrincipalID);
            if (!pRequest->HasError())
            {
                {
                    qList<StationURL> urls;
                    const bool bSuccess = RequestURLs(uiConnectionID, uiPrincipalID, &urls);
                    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_REQUEST_URLS);
                    *pResponse << bSuccess;
                    _Type_std_list<_Type_stationurl>::Add(pResponse, urls);
                }
                *pResponseRequired = true;
                return;
            }
        }
        break;
    }
    case METHOD_REGISTER_EX:
    {
        {
            qList<StationURL> urls;
            _Type_std_list<_Type_stationurl>::Extract(pRequest, &urls);
            if (!pRequest->HasError())
            {
                unsigned int uiConnectionID;
                StationURL publicURL;
                AnyObjectHolder<Data, String> customData;
                _Type_any<DDLClassMetaType<Data>, _Type_string>::Extract(pRequest, &customData);
                if (!pRequest->HasError())
                {
                    const qResult result = RegisterEx(urls, &uiConnectionID, &publicURL, customData);
                    AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_REGISTER_EX);
                    _Type_qresult::Add(pResponse, result);
                    _Type_uint32::Add(pResponse, uiConnectionID);
                    _Type_stationurl::Add(pResponse, publicURL);
                }
            }
        }
        *pResponseRequired = true;
        return;
    }
    case METHOD_TEST_CONNECTIVITY:
        TestConnectivity();
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_TEST_CONNECTIVITY);
        *pResponseRequired = true;
        return;
    case METHOD_UPDATE_URLS:
    {
        {
            qList<StationURL> urls;
            _Type_std_list<_Type_stationurl>::Extract(pRequest, &urls);
            if (!pRequest->HasError())
            {
                UpdateURLs(urls);
                AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_UPDATE_URLS);
            }
        }
        *pResponseRequired = true;
        return;
    }
    case METHOD_REPLACE_URL:
    {
        StationURL target;
        _Type_stationurl::Extract(pRequest, &target);
        if (pRequest->HasError())
        {
            *pResponseRequired = true;
            return;
        }
        StationURL replacement;
        _Type_stationurl::Extract(pRequest, &replacement);
        if (pRequest->HasError())
        {
            *pResponseRequired = true;
            return;
        }
        ReplaceURL(target, replacement);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_REPLACE_URL);
        *pResponseRequired = true;
        return;
    }
    case METHOD_SEND_REPORT:
    {
        unsigned int uiReportID;
        _Type_uint32::Extract(pRequest, &uiReportID);
        if (!pRequest->HasError())
        {
            qBuffer report(0, 0);
            _Type_qBuffer::Extract(pRequest, &report);
            if (pRequest->HasError())
            {
                *pResponseRequired = true;
                return;
            }
            SendReport(uiReportID, report);
            AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_SEND_REPORT);
        }
        break;
    }
    case 0:
    default:
        SetCallError(qResult(RESULT_NOT_IMPLEMENTED));
        return;
    }
    *pResponseRequired = true;
}

}
}
