#include "nn/nex/_Proto_RemoteLogDeviceProtocolServer.hpp"

#include <stdint.h>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

void _Proto_RemoteLogDeviceProtocolServer::DispatchProtocolMessage(Message* pRequest, Message* pResponse,
                                                                   bool* pResponseRequired, EndPoint*)
{
    const uint16_t uiMethodID = static_cast<uint16_t>(ExtractMethodID(pRequest));
    if (uiMethodID == METHOD_LOG)
    {
        {
            String message;
            _Type_string::Extract(pRequest, &message);
            if (!pRequest->HasError())
            {
                Log(message);
                AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_LOG);
            }
        }
        *pResponseRequired = true;
        return;
    }
    SetCallError(qResult(RESULT_NOT_IMPLEMENTED));
}

}
}
