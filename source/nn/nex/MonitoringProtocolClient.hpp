#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_MONITORINGPROTOCOLCLIENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_MONITORINGPROTOCOLCLIENT_HPP

#include "nn/nex/ClientProtocol.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class MonitoringProtocolClient : public ClientProtocol
{
public:
    explicit MonitoringProtocolClient(unsigned short uiProtocolID = 0);

    virtual void ExtractCallSpecificResults(Message* pMessage, ProtocolCallContext* pCallContext);

    virtual Protocol* CreateResponder() const { return new MonitoringProtocolClient(); }

    static void ProtoReturn_PingDaemon(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallPingDaemon(ProtocolCallContext* pCallContext, bool* pReturnValue);
    static void ProtoReturn_GetClusterMembers(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallGetClusterMembers(ProtocolCallContext* pCallContext, qList<String>* pClusterMembers);

    static bool PingDaemon_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pReturnValue);
    static bool GetClusterMembers_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                                 qList<String>* pClusterMembers);
};

}
}

#endif
