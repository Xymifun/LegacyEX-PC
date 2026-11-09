#include "nn/nex/_Proto_TicketGrantingProtocolServer.hpp"

#include <stdint.h>

#include "nn/nex/Buffer.hpp"
#include "nn/nex/DDLClassMetaType.hpp"
#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Data.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/RVConnectionData.hpp"
#include "nn/nex/_Type_buffer.hpp"
#include "nn/nex/_Type_qresult.hpp"

namespace nn
{
namespace nex
{

namespace
{

typedef _Type_any<DDLClassMetaType<Data>, _Type_string> CustomDataAny;

const unsigned int CALL_ERROR_UNKNOWN_METHOD = 0x80010002u;
const unsigned int RESPONSE_METHOD_FLAG = 0x8000;

}

void _Proto_TicketGrantingProtocolServer::DispatchProtocolMessage(Message* pRequest, Message* pResponse,
                                                                  bool* pResponseRequired, EndPoint*)
{
    const uint16_t uiMethodID = static_cast<uint16_t>(ExtractMethodID(pRequest));
    switch (uiMethodID)
    {
    case 0:
    default:
        SetCallError(qResult(static_cast<int>(CALL_ERROR_UNKNOWN_METHOD)));
        return;
    case METHOD_LOGIN:
    {
        String userName;
        _Type_string::Extract(pRequest, &userName);
        if (pRequest->HasError())
            break;
        unsigned int uiPrincipalID;
        Buffer ticket;
        RVConnectionData connectionData;
        String returnMessage;
        const qResult result = Login(userName, &uiPrincipalID, &ticket, &connectionData, &returnMessage);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_LOGIN);
        _Type_qresult::Add(pResponse, result);
        _Type_uint32::Add(pResponse, uiPrincipalID);
        _Type_buffer::Add(pResponse, ticket);
        _DDL_RVConnectionData::Add(pResponse, connectionData);
        _Type_string::Add(pResponse, returnMessage);
        break;
    }
    case METHOD_LOGIN_EX:
    {
        String userName;
        _Type_string::Extract(pRequest, &userName);
        if (pRequest->HasError())
            break;
        unsigned int uiPrincipalID;
        Buffer ticket;
        RVConnectionData connectionData;
        String returnMessage;
        AnyObjectHolder<Data, String> customData;
        CustomDataAny::Extract(pRequest, &customData);
        if (pRequest->HasError())
            break;
        const qResult result = LoginEx(userName, &uiPrincipalID, &ticket, &connectionData, &returnMessage, customData);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_LOGIN_EX);
        _Type_qresult::Add(pResponse, result);
        _Type_uint32::Add(pResponse, uiPrincipalID);
        _Type_buffer::Add(pResponse, ticket);
        _DDL_RVConnectionData::Add(pResponse, connectionData);
        _Type_string::Add(pResponse, returnMessage);
        break;
    }
    case METHOD_REQUEST_TICKET:
    {
        unsigned int uiSourcePrincipalID;
        _Type_uint32::Extract(pRequest, &uiSourcePrincipalID);
        if (pRequest->HasError())
            break;
        unsigned int uiTargetPrincipalID;
        _Type_uint32::Extract(pRequest, &uiTargetPrincipalID);
        if (pRequest->HasError())
            break;
        Buffer ticket;
        const qResult result = RequestTicket(uiSourcePrincipalID, uiTargetPrincipalID, &ticket);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_REQUEST_TICKET);
        _Type_qresult::Add(pResponse, result);
        _Type_buffer::Add(pResponse, ticket);
        break;
    }
    case METHOD_GET_PID:
    {
        String userName;
        _Type_string::Extract(pRequest, &userName);
        if (pRequest->HasError())
            break;
        const unsigned int uiPrincipalID = GetPID(userName);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GET_PID);
        _Type_uint32::Add(pResponse, uiPrincipalID);
        break;
    }
    case METHOD_GET_NAME:
    {
        unsigned int uiPrincipalID;
        _Type_uint32::Extract(pRequest, &uiPrincipalID);
        if (pRequest->HasError())
            break;
        const String userName = GetName(uiPrincipalID);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GET_NAME);
        _Type_string::Add(pResponse, userName);
        break;
    }
    case METHOD_LOGIN_WITH_CONTEXT:
    {
        AnyObjectHolder<Data, String> loginData;
        CustomDataAny::Extract(pRequest, &loginData);
        if (pRequest->HasError())
            break;
        unsigned int uiPrincipalID;
        Buffer ticket;
        RVConnectionData connectionData;
        const qResult result = LoginWithContext(loginData, &uiPrincipalID, &ticket, &connectionData);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_LOGIN_WITH_CONTEXT);
        _Type_qresult::Add(pResponse, result);
        _Type_uint32::Add(pResponse, uiPrincipalID);
        _Type_buffer::Add(pResponse, ticket);
        _DDL_RVConnectionData::Add(pResponse, connectionData);
        break;
    }
    }
    *pResponseRequired = true;
}

}
}
