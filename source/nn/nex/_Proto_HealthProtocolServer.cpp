#include "nn/nex/_Proto_HealthProtocolServer.hpp"

#include <stdint.h>

#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

void _Proto_HealthProtocolServer::DispatchProtocolMessage(Message* pRequest, Message* pResponse,
                                                          bool* pResponseRequired, EndPoint*)
{
    switch (static_cast<uint16_t>(ExtractMethodID(pRequest)))
    {
    case METHOD_PING_DAEMON:
    {
        const bool bResult = PingDaemon();
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_PING_DAEMON);
        *pResponse << bResult;
        *pResponseRequired = true;
        return;
    }
    case METHOD_PING_DATABASE:
    {
        const bool bResult = PingDatabase();
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_PING_DATABASE);
        *pResponse << bResult;
        *pResponseRequired = true;
        return;
    }
    case METHOD_RUN_SANITY_CHECK:
    {
        const bool bResult = RunSanityCheck();
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_RUN_SANITY_CHECK);
        *pResponse << bResult;
        *pResponseRequired = true;
        return;
    }
    case METHOD_FIX_SANITY_ERRORS:
    {
        const bool bResult = FixSanityErrors();
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_FIX_SANITY_ERRORS);
        *pResponse << bResult;
        *pResponseRequired = true;
        return;
    }
    default:
        SetCallError(qResult(RESULT_NOT_IMPLEMENTED));
        return;
    }
}

}
}
