#include "nn/nex/JobWaitForNotification.hpp"

#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/Credentials.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/MyNotificationEventHandler.hpp"
#include "nn/nex/NotificationEventManager.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/RVClientCore.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

namespace
{

enum
{
    NOTIFICATION_EVENT_PROTOCOL_ID = 14,
    NOTIFICATION_PROTOCOL_FLAG = 8,
    WAIT_FOR_NOTIFICATION_POLL_DELAY = 16
};

enum WaitResult
{
    WAIT_CONNECTION_LOST_RESULT = static_cast<int>(0x80010004u),
    WAIT_TIMED_OUT_RESULT = static_cast<int>(0x8001000Bu),
    WAIT_CALL_FAILED_RESULT = static_cast<int>(0x8001000Du),
    WAIT_SUCCEEDED_RESULT = 0x10001
};

}

JobWaitForNotification::JobWaitForNotification()
    : StepSequenceJob(JOB_TYPE_2, DebugString(L"")),
      m_uiCallId(0),
      m_bRegistered(false),
      m_iTimeout(0)
{
}

JobWaitForNotification::~JobWaitForNotification()
{
    if (m_bRegistered)
    {
        RVClientCore::GetInstance()->UnregisterGlobalNotificationEventManager(m_pEventManager);
        m_pEventManager->UnregisterNotificationEventHandler(m_pEventHandler);
        delete m_pEventHandler;
        m_pEventHandler = 0;
        delete m_pEventManager;
        m_pEventManager = 0;
    }
}

bool JobWaitForNotification::StartImpl(ProtocolCallContext* pContext, unsigned int uiEventType,
                                       unsigned int uiEventSubType, int iTimeout, unsigned int uiParam1,
                                       NotificationEventFilterInterface** ppFilter, NotificationEvent* pEvent)
{
    if (!pContext->InitiateCall())
        return qResult(WAIT_CALL_FAILED_RESULT);

    JobWaitForNotification* pJob = new JobWaitForNotification();
    qResult result = pJob->Initialize(pContext->m_uiCallId, uiEventType, uiEventSubType, iTimeout, uiParam1, ppFilter,
                                      pEvent);
    if (!result)
    {
        pJob->ReleaseRef();
        return qResult(WAIT_CALL_FAILED_RESULT);
    }

    Scheduler::GetInstance()->Queue(pJob, false);
    return qResult(WAIT_SUCCEEDED_RESULT);
}

bool JobWaitForNotification::Start(ProtocolCallContext* pContext, unsigned int uiEventType, unsigned int uiEventSubType,
                                   int iTimeout, unsigned int uiParam1, NotificationEvent* pEvent)
{
    return StartImpl(pContext, uiEventType, uiEventSubType, iTimeout, uiParam1, 0, pEvent);
}

bool JobWaitForNotification::Start(ProtocolCallContext* pContext, unsigned int uiEventType, unsigned int uiEventSubType,
                                   int iTimeout, NotificationEventFilterInterface** ppFilter, NotificationEvent* pEvent)
{
    return StartImpl(pContext, uiEventType, uiEventSubType, iTimeout, 0, ppFilter, pEvent);
}

qResult JobWaitForNotification::Initialize(unsigned int uiCallId, unsigned int uiEventType, unsigned int uiEventSubType,
                                           int iTimeout, unsigned int uiParam1,
                                           NotificationEventFilterInterface** ppFilter, NotificationEvent* pEvent)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());

    m_uiCallId = uiCallId;
    m_iTimeout = iTimeout;

    ProtocolCallContext* pContext = static_cast<ProtocolCallContext*>(
        Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId));
    Credentials* pCredentials = pContext->GetCredentials();

    m_pEventManager = new NotificationEventManager(0);
    m_pEventManager->m_bDispatchServiceEvents = true;
    m_pEventManager->SetProtocolID(NOTIFICATION_EVENT_PROTOCOL_ID);
    m_pEventManager->SetFlag(NOTIFICATION_PROTOCOL_FLAG);
    m_pEventManager->m_uiPrincipalID = pCredentials->m_value;
    m_pEventManager->m_uiConnectionID = pCredentials->m_state;

    m_pEventHandler = new MyNotificationEventHandler(uiEventType, uiEventSubType, &m_event);
    m_pEventManager->RegisterNotificationEventHandler(m_pEventHandler);
    RVClientCore::GetInstance()->RegisterGlobalNotificationEventManager(m_pEventManager);
    m_bRegistered = true;
    m_chrono.Start();

    if (ppFilter != 0)
        *ppFilter = m_pEventHandler;
    else
        m_pEventHandler->SetEventParam1(uiParam1);

    m_pResultEvent = pEvent;
    SetStep(Step(static_cast<Step::Function>(&JobWaitForNotification::StepWaitingNotify), L""));
    return qResult(true);
}

void JobWaitForNotification::CompleteJob(const qResult& rResult)
{
    CallContext* pContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pContext != 0)
    {
        if (rResult)
        {
            if (m_pResultEvent != 0)
                *m_pResultEvent = *m_pEventHandler->m_pEvent;

            pContext->SignalSuccess(rResult);
            SetToComplete();
            return;
        }

        pContext->SignalFailure(rResult);
    }

    SetToComplete();
}

void JobWaitForNotification::StepWaitingNotify()
{
    CallContext* pContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pContext == 0)
    {
        CompleteJob(qResult(WAIT_CONNECTION_LOST_RESULT));
        return;
    }

    if (!m_pEventHandler->m_bEventReceived)
    {
        if (m_chrono.Check() > m_iTimeout)
        {
            CompleteJob(qResult(WAIT_TIMED_OUT_RESULT));
            return;
        }

        SetToWaiting(WAIT_FOR_NOTIFICATION_POLL_DELAY);
        return;
    }

    CompleteJob(qResult(WAIT_SUCCEEDED_RESULT));
}

}
}
