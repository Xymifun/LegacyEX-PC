#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__PROTO_MONITORINGPROTOCOLSERVER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__PROTO_MONITORINGPROTOCOLSERVER_HPP

#include "nn/nex/ServerProtocol.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class _Proto_MonitoringProtocolServer : public ServerProtocol
{
public:
    enum _MethodID
    {
        METHOD_PING_DAEMON = 1,
        METHOD_GET_CLUSTER_MEMBERS = 2
    };

    explicit _Proto_MonitoringProtocolServer(unsigned int uiFlags) : ServerProtocol(uiFlags) {}

    virtual void DispatchProtocolMessage(Message* pRequest, Message* pResponse, bool* pResponseRequired,
                                         EndPoint* pEndPoint);
    virtual bool PingDaemon() = 0;
    virtual void GetClusterMembers(qList<String>* pClusterMembers) = 0;

private:
    enum
    {
        RESULT_NOT_IMPLEMENTED = static_cast<int>(0x80010002u),
        RESPONSE_METHOD_FLAG = 0x8000
    };
};

}
}

#endif
