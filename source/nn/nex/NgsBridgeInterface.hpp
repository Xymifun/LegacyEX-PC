#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NGSBRIDGEINTERFACE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NGSBRIDGEINTERFACE_HPP

#include "nn/nex/RootObject.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class Credentials;
class NotificationEventHandler;
class ProtocolCallContext;
class StationURL;

class NgsBridgeInterface : public RootObject
{
public:
    virtual ~NgsBridgeInterface() {}

    virtual Credentials* GetCredentials() = 0;
    virtual unsigned int GetPrincipalID() = 0;
    virtual bool RegisterNotificationEventHandler(NotificationEventHandler* pHandler) = 0;
    virtual bool UnregisterNotificationEventHandler(NotificationEventHandler* pHandler) = 0;
    virtual bool ReplaceURL(ProtocolCallContext* pCallContext, const StationURL& rCurrentURL,
                            const StationURL& rNewURL) = 0;
    virtual void SendReport(unsigned int uiReportId, const void* pData, unsigned int uiSize) = 0;
    virtual qResult IsConnected() const = 0;
};

}
}

#endif
