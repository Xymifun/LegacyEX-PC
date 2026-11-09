#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HEALTHPROTOCOLCLIENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HEALTHPROTOCOLCLIENT_HPP

#include "nn/nex/ClientProtocol.hpp"

namespace nn
{
namespace nex
{

class HealthProtocolClient : public ClientProtocol
{
public:
    explicit HealthProtocolClient(unsigned short uiProtocolID = 0);

    virtual void ExtractCallSpecificResults(Message* pMessage, ProtocolCallContext* pCallContext);

    virtual Protocol* CreateResponder() const { return new HealthProtocolClient(); }

    static void ProtoReturn_PingDaemon(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallPingDaemon(ProtocolCallContext* pCallContext, bool* pReturnValue);
    static void ProtoReturn_PingDatabase(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallPingDatabase(ProtocolCallContext* pCallContext, bool* pReturnValue);
    static void ProtoReturn_RunSanityCheck(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallRunSanityCheck(ProtocolCallContext* pCallContext, bool* pReturnValue);
    static void ProtoReturn_FixSanityErrors(Message* pMessage, ProtocolCallContext* pCallContext);
    bool CallFixSanityErrors(ProtocolCallContext* pCallContext, bool* pReturnValue);

    static bool PingDaemon_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pReturnValue);
    static bool PingDatabase_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pReturnValue);
    static bool RunSanityCheck_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, bool* pReturnValue);
    static bool FixSanityErrors_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage,
                                               bool* pReturnValue);
};

}
}

#endif
