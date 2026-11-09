#include "nn/nex/_Proto_MonitoringProtocolServer.hpp"

#include <stdint.h>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

void _Proto_MonitoringProtocolServer::DispatchProtocolMessage(Message* pRequest, Message* pResponse,
                                                              bool* pResponseRequired, EndPoint*)
{
    const uint16_t uiMethodID = static_cast<uint16_t>(ExtractMethodID(pRequest));
    switch (uiMethodID)
    {
    case METHOD_PING_DAEMON:
    {
        const bool bResult = PingDaemon();
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_PING_DAEMON);
        *pResponse << bResult;
        *pResponseRequired = true;
        return;
    }
    case METHOD_GET_CLUSTER_MEMBERS:
    {
        qList<String> clusterMembers;
        GetClusterMembers(&clusterMembers);
        AddMethodID(pResponse, RESPONSE_METHOD_FLAG | METHOD_GET_CLUSTER_MEMBERS);
        _Type_std_list<_Type_string>::Add(pResponse, clusterMembers);
    }
        *pResponseRequired = true;
        return;
    default:
        SetCallError(qResult(RESULT_NOT_IMPLEMENTED));
        return;
    }
}

}
}
