#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__PROTO_HEALTHPROTOCOLSERVER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__PROTO_HEALTHPROTOCOLSERVER_HPP

#include "nn/nex/ServerProtocol.hpp"

namespace nn
{
namespace nex
{

class _Proto_HealthProtocolServer : public ServerProtocol
{
public:
    enum _MethodID
    {
        METHOD_PING_DAEMON = 1,
        METHOD_PING_DATABASE = 2,
        METHOD_RUN_SANITY_CHECK = 3,
        METHOD_FIX_SANITY_ERRORS = 4
    };

    explicit _Proto_HealthProtocolServer(unsigned int uiFlags) : ServerProtocol(uiFlags) {}

    virtual void DispatchProtocolMessage(Message* pRequest, Message* pResponse, bool* pResponseRequired,
                                         EndPoint* pEndPoint);
    virtual bool PingDaemon() = 0;
    virtual bool PingDatabase() = 0;
    virtual bool RunSanityCheck() = 0;
    virtual bool FixSanityErrors() = 0;

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
