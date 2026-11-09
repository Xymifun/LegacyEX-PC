#include "nn/nex/NgsBridgeImp.hpp"
#include "nn/nex/Credentials.hpp"
#include "nn/nex/Reconstruction.hpp"

namespace nn
{
namespace nex
{

NgsBridgeImp::NgsBridgeImp(NgsFacade* pNgsFacade) : m_pNgsFacade(pNgsFacade)
{
}

NgsBridgeImp::~NgsBridgeImp()
{
}

Credentials* NgsBridgeImp::GetCredentials()
{
    return m_pNgsFacade->GetCredentials();
}

unsigned int NgsBridgeImp::GetPrincipalID()
{
    return GetCredentials()->GetPrincipalID();
}

bool NgsBridgeImp::RegisterNotificationEventHandler(NotificationEventHandler* pHandler)
{
    return m_pNgsFacade->RegisterNotificationEventHandler(pHandler);
}

bool NgsBridgeImp::UnregisterNotificationEventHandler(NotificationEventHandler* pHandler)
{
    return m_pNgsFacade->UnregisterNotificationEventHandler(pHandler);
}

bool NgsBridgeImp::ReplaceURL(ProtocolCallContext* pCallContext, const StationURL& rCurrentURL,
                              const StationURL& rNewURL)
{
    bool result = false;
    if (m_pNgsFacade->GetSecureConnectionClient() != 0)
        result = m_pNgsFacade->GetSecureConnectionClient()->ReplaceURL(pCallContext, rCurrentURL, rNewURL);
    return result;
}

void NgsBridgeImp::SendReport(unsigned int uiReportId, const void* pData, unsigned int uiSize)
{
    if (m_pNgsFacade->GetSecureConnectionClient() != 0)
        m_pNgsFacade->GetSecureConnectionClient()->SendReport(uiReportId, pData, uiSize);
}

qResult NgsBridgeImp::IsConnected() const
{
    return m_pNgsFacade->IsConnected();
}

}
}
