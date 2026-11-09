#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__PROTO_REMOTELOGDEVICEPROTOCOLSERVER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__PROTO_REMOTELOGDEVICEPROTOCOLSERVER_HPP

#include "nn/nex/ServerProtocol.hpp"

namespace nn
{
namespace nex
{

class String;

class _Proto_RemoteLogDeviceProtocolServer : public ServerProtocol
{
public:
    enum _MethodID
    {
        METHOD_LOG = 1
    };

    explicit _Proto_RemoteLogDeviceProtocolServer(unsigned int uiFlags) : ServerProtocol(uiFlags) {}

    virtual void DispatchProtocolMessage(Message* pRequest, Message* pResponse, bool* pResponseRequired,
                                         EndPoint* pEndPoint);
    virtual void Log(const String& rMessage) = 0;

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
