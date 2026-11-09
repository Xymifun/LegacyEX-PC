#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NGSBRIDGEIMP_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NGSBRIDGEIMP_HPP

#include "nn/nex/NgsBridgeInterface.hpp"

namespace nn
{
namespace nex
{

class NgsFacade;

class NgsBridgeImp : public NgsBridgeInterface
{
public:
    explicit NgsBridgeImp(NgsFacade* pNgsFacade);
    virtual ~NgsBridgeImp();
    virtual Credentials* GetCredentials();
    virtual unsigned int GetPrincipalID();
    virtual bool RegisterNotificationEventHandler(NotificationEventHandler* pHandler);
    virtual bool UnregisterNotificationEventHandler(NotificationEventHandler* pHandler);
    virtual bool ReplaceURL(ProtocolCallContext* pCallContext, const StationURL& rCurrentURL,
                            const StationURL& rNewURL);
    virtual void SendReport(unsigned int uiReportId, const void* pData, unsigned int uiSize);
    virtual qResult IsConnected() const;

private:
    NgsFacade* m_pNgsFacade;
};

}
}

#endif
