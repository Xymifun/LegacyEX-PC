#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBWAITFORNOTIFICATION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBWAITFORNOTIFICATION_HPP

#include "nn/nex/Chrono.hpp"
#include "nn/nex/NotificationEvent.hpp"
#include "nn/nex/StepSequenceJob.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class MyNotificationEventHandler;
class NotificationEventFilterInterface;
class NotificationEventManager;
class ProtocolCallContext;

class JobWaitForNotification : public StepSequenceJob
{
public:
    JobWaitForNotification();
    virtual ~JobWaitForNotification();

    virtual qResult Initialize(unsigned int uiCallId, unsigned int uiEventType, unsigned int uiEventSubType,
                               int iTimeout, unsigned int uiParam1, NotificationEventFilterInterface** ppFilter,
                               NotificationEvent* pEvent);

    static bool Start(ProtocolCallContext* pContext, unsigned int uiEventType, unsigned int uiEventSubType,
                      int iTimeout, unsigned int uiParam1, NotificationEvent* pEvent);
    static bool Start(ProtocolCallContext* pContext, unsigned int uiEventType, unsigned int uiEventSubType,
                      int iTimeout, NotificationEventFilterInterface** ppFilter, NotificationEvent* pEvent);

    void StepWaitingNotify();
    void CompleteJob(const qResult& rResult);

private:
    static bool StartImpl(ProtocolCallContext* pContext, unsigned int uiEventType, unsigned int uiEventSubType,
                          int iTimeout, unsigned int uiParam1, NotificationEventFilterInterface** ppFilter,
                          NotificationEvent* pEvent);

    unsigned int m_uiCallId;
    MyNotificationEventHandler* m_pEventHandler;
    NotificationEventManager* m_pEventManager;
    Chrono m_chrono;
    bool m_bRegistered;
    NotificationEvent* m_pResultEvent;
    NotificationEvent m_event;
    int m_iTimeout;
};

}
}

#endif
