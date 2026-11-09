#include "nn/nex/GlobalNotificationEventManager.hpp"

#include "nn/nex/ClientStreamManager.hpp"
#include "nn/nex/Credentials.hpp"
#include "nn/nex/EndPoint.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/NotificationEvent.hpp"

namespace nn
{
namespace nex
{

uint32_t GlobalNotificationEventManager::RegisterGlobalNotificationEventManager(NotificationEventManager* pManager)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    if (m_managers.find(pManager) != m_managers.end())
        return 0;

    m_managers.insert(pManager);
    return 1;
}

uint32_t GlobalNotificationEventManager::UnregisterGlobalNotificationEventManager(NotificationEventManager* pManager)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    ManagerSet::iterator it = m_managers.find(pManager);
    if (it == m_managers.end())
        return 0;

    m_managers.erase(it);
    return 1;
}

void GlobalNotificationEventManager::ProcessNotificationEvent(unsigned int uiPrincipalID, unsigned int uiConnectionID,
                                                              const NotificationEvent& rEvent)
{
    for (ManagerSet::iterator it = m_managers.begin(); it != m_managers.end(); ++it)
        (*it)->ProcessNotificationEvent(uiPrincipalID, uiConnectionID, rEvent);
}

void GlobalNotificationEventManager::DispatchProtocolMessage(Message* pRequest, Message* pResponse,
                                                             bool* pResponseRequired, EndPoint* pEndPoint)
{
    Credentials* pCredentials = static_cast<ClientStreamManager*>(pEndPoint->m_pEventHandler)->m_pCredentials;
    unsigned int uiPrincipalID = pCredentials->m_value;
    unsigned int uiConnectionID = pCredentials->m_state;
    if (static_cast<uint16_t>(ExtractMethodID(pRequest)) == METHOD_PROCESS_NOTIFICATION_EVENT)
    {
        NotificationEvent event;
        _DDL_NotificationEvent::Extract(pRequest, &event);
        if (pRequest->HasError())
            return;

        ProcessNotificationEvent(uiPrincipalID, uiConnectionID, event);
        AddMethodID(pResponse, 0x8001);
        *pResponseRequired = true;
        return;
    }

    SetCallError(qResult(static_cast<int>(0x80010002u)));
}

}
}
