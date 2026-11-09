#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_REMOTELOGDEVICEPROTOCOLCLIENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_REMOTELOGDEVICEPROTOCOLCLIENT_HPP

#include "nn/nex/ClientProtocol.hpp"

namespace nn
{
namespace nex
{

class String;

class RemoteLogDeviceProtocolClient : public ClientProtocol
{
public:
    explicit RemoteLogDeviceProtocolClient(unsigned short uiProtocolID = 0);

    virtual void ExtractCallSpecificResults(Message* pMessage, ProtocolCallContext* pCallContext);

    virtual Protocol* CreateResponder() const { return new RemoteLogDeviceProtocolClient(); }

    static void ProtoReturn_Log(Message* pMessage, ProtocolCallContext* pCallContext) {}

    bool CallLog(ProtocolCallContext* pCallContext, const String& rMessage);

    static bool Log_PrepareRequest(ProtocolCallContext* pCallContext, Message* pMessage, const String& rMessage);
};

}
}

#endif
