#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBDATASTORERECEIVENEWARRIVEDOBJECTS_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBDATASTORERECEIVENEWARRIVEDOBJECTS_HPP

#include <stdint.h>
#include <string.h>

#include "nn/nex/CallContext.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/DataStoreCheckConsistencyEventListenerAdaptor.hpp"
#include "nn/nex/DataStoreClientInterface.hpp"
#include "nn/nex/DataStoreGetNewArrivedNotificationsParam.hpp"
#include "nn/nex/DataStoreGetSpecificMetaParam.hpp"
#include "nn/nex/DataStoreGetSpecificMetaParamV1.hpp"
#include "nn/nex/DataStoreLogicServerClientInterface.hpp"
#include "nn/nex/DataStoreNewArrivalCheckToken.hpp"
#include "nn/nex/DataStoreNotification.hpp"
#include "nn/nex/DataStoreNotificationV1.hpp"
#include "nn/nex/DataStorePrepareGetParam.hpp"
#include "nn/nex/DataStorePrepareGetParamV1.hpp"
#include "nn/nex/DataStoreReceiveNewArrivedObjectsEventListenerTemplate.hpp"
#include "nn/nex/DataStoreSpecificMetaInfo.hpp"
#include "nn/nex/DataStoreSpecificMetaInfoV1.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/StepSequenceJob.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qResult.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

template <typename TTraits>
class JobDataStoreReceiveNewArrivedObjects : public StepSequenceJob
{
public:
    typedef bool (DataStoreClientInterface::*GetObjectFunction)(ProtocolCallContext* pContext,
                                                                const typename TTraits::TPrepareGetParam& rParam,
                                                                typename TTraits::TGetObjectEventListener* pListener);

    JobDataStoreReceiveNewArrivedObjects();
    virtual ~JobDataStoreReceiveNewArrivedObjects();

    virtual qResult Initialize(unsigned int uiCallId, DataStoreClientInterface* pClient,
                               DataStoreLogicServerClientInterface* pLogicServerClient, unsigned int uiThreadPriority,
                               bool bUseHttps, unsigned int uiPrincipalId, const unsigned char* pNotificationBuffer,
                               qVector<typename TTraits::TGetObjectEventListener*> getObjectListeners,
                               typename TTraits::TReceiveNewArrivedObjectsEventListener* pListener, bool* pbHasNext);
    virtual qResult Initialize(unsigned int uiCallId, DataStoreClientInterface* pClient, unsigned int uiThreadPriority,
                               unsigned int uiPrincipalId, const unsigned char* pNotificationBuffer,
                               typename TTraits::TReceiveNewArrivedObjectsEventListener* pListener, bool* pbHasNext);

    void CompleteJob(const qResult& rResult);
    void StepCheckNotification();
    void StepWaitingCheckNotification();
    void StepGetNewArrivedNotifications();
    void StepWaitingGetNewArrivedNotifications();
    void StepGetSpecificMeta();
    void StepWaitingGetSpecificMeta();
    void StepGetObject();
    void StepWaitingGetObject();

    static GetObjectFunction s_getObjectFunction;

private:
    enum Result
    {
        RESULT_SUCCESS = 0x10001,
        RESULT_CORE_OPERATION_ABORTED = static_cast<int>(0x80010004u),
        RESULT_CORE_CALL_FAILED = static_cast<int>(0x8001000Du),
        RESULT_DATASTORE_UNKNOWN = static_cast<int>(0x80690001u),
        RESULT_DATASTORE_INVALID_CHECK_TOKEN = static_cast<int>(0x80690008u)
    };

    enum
    {
        FACILITY_DATASTORE = 0x69,
        FORCE_URL_REQUEST_FAILURE_COUNT = 10
    };

    unsigned int m_uiCallId;
    DataStoreClientInterface* m_pClient;
    DataStoreLogicServerClientInterface* m_pLogicServerClient;
    bool m_bUseHttps;
    unsigned int m_uiThreadPriority;
    ProtocolCallContext m_context;
    uint64_t m_uiLastDataId;
    String m_notificationUrl;
    unsigned int m_uiSequence;
    unsigned int m_uiPrincipalId;
    qVector<typename TTraits::TNotification> m_notifications;
    DataStoreNewArrivalCheckToken m_checkToken;
    DataStoreNewArrivalCheckToken m_nextCheckToken;
    qVector<typename TTraits::TGetObjectEventListener*> m_getObjectListeners;
    typename TTraits::TReceiveNewArrivedObjectsEventListener* m_pListener;
    unsigned int m_uiGetObjectListenerIndex;
    unsigned int m_uiNotificationIndex;
    bool* m_pbHasNext;
    qVector<typename TTraits::TSpecificMetaInfo> m_metaInfos;
    bool m_bUrlRequestFailed;
    DataStoreCheckConsistencyEventListenerAdaptor<TTraits>* m_pAdaptor;
    bool m_bCheckNewArrivalOnly;
    bool* m_pbHasNewArrival;
    bool m_bNoCheckToken;
};

template <typename TTraits>
JobDataStoreReceiveNewArrivedObjects<TTraits>::JobDataStoreReceiveNewArrivedObjects()
    : StepSequenceJob(JOB_TYPE_2, DebugString(L"")),
      m_uiCallId(0),
      m_pClient(0),
      m_pLogicServerClient(0),
      m_bUseHttps(true),
      m_uiThreadPriority(0),
      m_context(),
      m_uiLastDataId(0),
      m_notificationUrl(L""),
      m_uiSequence(0),
      m_uiPrincipalId(0),
      m_notifications(),
      m_checkToken(),
      m_nextCheckToken(),
      m_getObjectListeners(),
      m_pListener(0),
      m_uiGetObjectListenerIndex(0),
      m_uiNotificationIndex(0),
      m_pbHasNext(0),
      m_metaInfos(),
      m_bUrlRequestFailed(false),
      m_pAdaptor(0),
      m_bCheckNewArrivalOnly(false),
      m_pbHasNewArrival(0),
      m_bNoCheckToken(false)
{
}

template <typename TTraits>
JobDataStoreReceiveNewArrivedObjects<TTraits>::~JobDataStoreReceiveNewArrivedObjects()
{
    delete m_pAdaptor;
}

template <typename TTraits>
qResult JobDataStoreReceiveNewArrivedObjects<TTraits>::Initialize(
    unsigned int uiCallId, DataStoreClientInterface* pClient, DataStoreLogicServerClientInterface* pLogicServerClient,
    unsigned int uiThreadPriority, bool bUseHttps, unsigned int uiPrincipalId, const unsigned char* pNotificationBuffer,
    qVector<typename TTraits::TGetObjectEventListener*> getObjectListeners,
    typename TTraits::TReceiveNewArrivedObjectsEventListener* pListener, bool* pbHasNext)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    m_uiCallId = uiCallId;
    m_uiPrincipalId = uiPrincipalId;
    m_pLogicServerClient = pLogicServerClient;
    m_bUseHttps = bUseHttps;
    m_uiThreadPriority = uiThreadPriority;
    m_pClient = pClient;
    m_getObjectListeners = getObjectListeners;
    m_pListener = pListener;
    *pbHasNext = false;
    m_bCheckNewArrivalOnly = false;
    m_pbHasNext = pbHasNext;
    if (pNotificationBuffer == 0)
    {
        m_checkToken.m_uiReserved0 = 0;
        m_checkToken.m_uiFailureCount = 0;
        m_checkToken.m_uiSequence = 0;
        m_checkToken.m_uiReserved1 = 0;
        m_checkToken.m_uiLastNotificationId = 0;
        memset(m_checkToken.m_szNotificationUrl, 0, sizeof(m_checkToken.m_szNotificationUrl));
        m_checkToken.m_uiReserved2 = 0;
        m_checkToken.UpdateChecksum();
    }
    else
    {
        if (!DataStoreNewArrivalCheckToken::Validate(
                *reinterpret_cast<const DataStoreNewArrivalCheckToken*>(pNotificationBuffer)))
            return qResult(RESULT_DATASTORE_INVALID_CHECK_TOKEN);

        memcpy(&m_checkToken, pNotificationBuffer, sizeof(m_checkToken));
    }

    m_nextCheckToken = m_checkToken;
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    m_context.SetCredentials(static_cast<ProtocolCallContext*>(pCallContext)->GetCredentials());
    SetStep(Step(static_cast<Step::Function>(&JobDataStoreReceiveNewArrivedObjects::StepCheckNotification), L""));
    return qResult(true);
}

template <typename TTraits>
qResult JobDataStoreReceiveNewArrivedObjects<TTraits>::Initialize(
    unsigned int uiCallId, DataStoreClientInterface* pClient, unsigned int uiThreadPriority, unsigned int uiPrincipalId,
    const unsigned char* pNotificationBuffer, typename TTraits::TReceiveNewArrivedObjectsEventListener* pListener,
    bool* pbHasNext)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    m_uiCallId = uiCallId;
    m_pbHasNewArrival = pbHasNext;
    m_bCheckNewArrivalOnly = true;
    m_pLogicServerClient = 0;
    m_uiThreadPriority = uiThreadPriority;
    m_pListener = pListener;
    m_pClient = pClient;
    m_uiPrincipalId = uiPrincipalId;
    m_pbHasNext = 0;
    m_bUseHttps = true;
    *pbHasNext = false;
    m_bNoCheckToken = pNotificationBuffer == 0;
    if (pNotificationBuffer == 0)
    {
        m_checkToken.m_uiReserved0 = 0;
        m_checkToken.m_uiFailureCount = 0;
        m_checkToken.m_uiSequence = 0;
        m_checkToken.m_uiReserved1 = 0;
        m_checkToken.m_uiLastNotificationId = 0;
        memset(m_checkToken.m_szNotificationUrl, 0, sizeof(m_checkToken.m_szNotificationUrl));
        m_checkToken.m_uiReserved2 = 0;
        m_checkToken.UpdateChecksum();
    }
    else
    {
        if (!DataStoreNewArrivalCheckToken::Validate(
                *reinterpret_cast<const DataStoreNewArrivalCheckToken*>(pNotificationBuffer)))
            return qResult(RESULT_DATASTORE_INVALID_CHECK_TOKEN);

        memcpy(&m_checkToken, pNotificationBuffer, sizeof(m_checkToken));
    }

    m_nextCheckToken = m_checkToken;
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    m_context.SetCredentials(static_cast<ProtocolCallContext*>(pCallContext)->GetCredentials());
    SetStep(Step(static_cast<Step::Function>(&JobDataStoreReceiveNewArrivedObjects::StepCheckNotification), L""));
    return qResult(true);
}

template <typename TTraits>
void JobDataStoreReceiveNewArrivedObjects<TTraits>::CompleteJob(const qResult& rResult)
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext != 0)
    {
        if (rResult)
            pCallContext->SignalSuccess(rResult);
        else
            pCallContext->SignalFailure(rResult);
    }
    else
    {
        m_context.Cancel(CallContext::CallCanceled);
    }

    SetToComplete();
}

template <typename TTraits>
void JobDataStoreReceiveNewArrivedObjects<TTraits>::StepCheckNotification()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    m_context.Reset();
    if (!m_pClient->CheckNotification(&m_context, String(m_checkToken.m_szNotificationUrl), &m_notificationUrl,
                                      m_checkToken.m_uiLastNotificationId, &m_uiLastDataId, m_checkToken.m_uiSequence,
                                      &m_uiSequence, m_uiPrincipalId, &m_bUrlRequestFailed,
                                      m_checkToken.m_uiFailureCount >= FORCE_URL_REQUEST_FAILURE_COUNT))
    {
        CompleteJob(qResult(RESULT_CORE_CALL_FAILED));
        return;
    }

    Core::GetInstance()->GetCallContextRegister()->AssociateCallContext(m_uiCallId, m_context.GetCallID());
    SetToSuspended();
    ResumeOnCallCompletion(
        &m_context,
        new Step(static_cast<Step::Function>(&JobDataStoreReceiveNewArrivedObjects::StepWaitingCheckNotification),
                 L""));
}

template <typename TTraits>
void JobDataStoreReceiveNewArrivedObjects<TTraits>::StepWaitingCheckNotification()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    bool bCheckTokenUpdated = false;
    bool bNoNewArrival = false;
    if (m_context.GetState() != CallContext::CallSucceeded)
    {
        if (m_bUrlRequestFailed)
        {
            CompleteJob(m_context.GetResult());
            return;
        }

        ++m_nextCheckToken.m_uiFailureCount;
        m_nextCheckToken.UpdateChecksum();
        bCheckTokenUpdated = true;
        bNoNewArrival = true;
    }
    else
    {
        if (String::IsEqual(m_notificationUrl, L"invalidurl"))
        {
            m_nextCheckToken.m_uiLastNotificationId = m_uiLastDataId;
            m_nextCheckToken.UpdateChecksum();
            memset(m_nextCheckToken.m_szNotificationUrl, 0, sizeof(m_nextCheckToken.m_szNotificationUrl));
            char* pNotificationUrl = 0;
            m_notificationUrl.CreateCopy(&pNotificationUrl);
            strcpy(m_nextCheckToken.m_szNotificationUrl, pNotificationUrl);
            String::ReleaseCopy(pNotificationUrl);
            m_nextCheckToken.UpdateChecksum();
            m_nextCheckToken.m_uiSequence = m_uiSequence;
            m_nextCheckToken.UpdateChecksum();
            m_pListener->UpdateCheckToken(reinterpret_cast<const unsigned char*>(&m_nextCheckToken), 0,
                                          m_nextCheckToken.m_uiLastNotificationId);
            CompleteJob(qResult(RESULT_SUCCESS));
            return;
        }

        if (m_nextCheckToken.m_uiFailureCount != 0)
        {
            m_nextCheckToken.m_uiFailureCount = 0;
            m_nextCheckToken.UpdateChecksum();
            bCheckTokenUpdated = true;
        }

        if (!String::IsEqual(String(m_checkToken.m_szNotificationUrl), m_notificationUrl))
        {
            memset(m_nextCheckToken.m_szNotificationUrl, 0, sizeof(m_nextCheckToken.m_szNotificationUrl));
            char* pNotificationUrl = 0;
            m_notificationUrl.CreateCopy(&pNotificationUrl);
            strcpy(m_nextCheckToken.m_szNotificationUrl, pNotificationUrl);
            String::ReleaseCopy(pNotificationUrl);
            m_nextCheckToken.UpdateChecksum();
            bCheckTokenUpdated = true;
        }

        if (m_checkToken.m_uiSequence != m_uiSequence)
        {
            m_nextCheckToken.m_uiSequence = m_uiSequence;
            m_nextCheckToken.UpdateChecksum();
            bCheckTokenUpdated = true;
        }

        if (m_checkToken.m_uiLastNotificationId >= m_uiLastDataId)
            bNoNewArrival = true;
    }

    if (m_bCheckNewArrivalOnly && (!bNoNewArrival || m_bNoCheckToken))
        *m_pbHasNewArrival = true;

    if (bCheckTokenUpdated)
        m_pListener->UpdateCheckToken(reinterpret_cast<const unsigned char*>(&m_nextCheckToken), 0,
                                      m_nextCheckToken.m_uiLastNotificationId);

    if (bNoNewArrival || m_bCheckNewArrivalOnly)
    {
        CompleteJob(qResult(RESULT_SUCCESS));
        return;
    }

    SetStep(
        Step(static_cast<Step::Function>(&JobDataStoreReceiveNewArrivedObjects::StepGetNewArrivedNotifications), L""));
}

template <typename TTraits>
void JobDataStoreReceiveNewArrivedObjects<TTraits>::StepGetNewArrivedNotifications()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    DataStoreGetNewArrivedNotificationsParam param;
    param.m_uiLastNotificationId = m_checkToken.m_uiLastNotificationId;
    param.m_uiLimit = static_cast<uint16_t>(m_getObjectListeners.size());
    if (!m_pLogicServerClient->GetNewArrivedNotifications(&m_context, param, &m_notifications, m_pbHasNext))
    {
        CompleteJob(qResult(RESULT_CORE_CALL_FAILED));
        return;
    }

    Core::GetInstance()->GetCallContextRegister()->AssociateCallContext(m_uiCallId, m_context.GetCallID());
    SetToSuspended();
    ResumeOnCallCompletion(&m_context,
                           new Step(static_cast<Step::Function>(
                                        &JobDataStoreReceiveNewArrivedObjects::StepWaitingGetNewArrivedNotifications),
                                    L""));
}

template <typename TTraits>
void JobDataStoreReceiveNewArrivedObjects<TTraits>::StepWaitingGetNewArrivedNotifications()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    if (m_context.GetState() != CallContext::CallSucceeded)
    {
        CompleteJob(m_context.GetResult());
        return;
    }

    for (typename qVector<typename TTraits::TNotification>::iterator it = m_notifications.begin();
         it != m_notifications.end(); ++it)
    {
    }

    if (m_notifications.size() == 0)
    {
        m_nextCheckToken.m_uiLastNotificationId = m_uiLastDataId;
        m_nextCheckToken.UpdateChecksum();
        m_pListener->UpdateCheckToken(reinterpret_cast<const unsigned char*>(&m_nextCheckToken), 0,
                                      m_nextCheckToken.m_uiLastNotificationId);
        CompleteJob(qResult(RESULT_SUCCESS));
    }

    SetStep(Step(static_cast<Step::Function>(&JobDataStoreReceiveNewArrivedObjects::StepGetSpecificMeta), L""));
}

template <typename TTraits>
void JobDataStoreReceiveNewArrivedObjects<TTraits>::StepGetSpecificMeta()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    typename TTraits::TGetSpecificMetaParam param;
    for (typename qVector<typename TTraits::TNotification>::iterator it = m_notifications.begin();
         it != m_notifications.end(); ++it)
    {
        if (it->m_uiDataId != 0)
            param.m_DataIds.push_back(it->m_uiDataId);
    }

    if (param.m_DataIds.size() == 0)
    {
        SetStep(Step(static_cast<Step::Function>(&JobDataStoreReceiveNewArrivedObjects::StepGetObject), L""));
        return;
    }

    if (!m_pLogicServerClient->GetSpecificMeta(&m_context, param, &m_metaInfos))
    {
        CompleteJob(qResult(RESULT_CORE_CALL_FAILED));
        return;
    }

    Core::GetInstance()->GetCallContextRegister()->AssociateCallContext(m_uiCallId, m_context.GetCallID());
    SetToSuspended();
    ResumeOnCallCompletion(
        &m_context,
        new Step(static_cast<Step::Function>(&JobDataStoreReceiveNewArrivedObjects::StepWaitingGetSpecificMeta), L""));
}

template <typename TTraits>
void JobDataStoreReceiveNewArrivedObjects<TTraits>::StepWaitingGetSpecificMeta()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    if (m_context.GetState() != CallContext::CallSucceeded)
    {
        CompleteJob(m_context.GetResult());
        return;
    }

    SetStep(Step(static_cast<Step::Function>(&JobDataStoreReceiveNewArrivedObjects::StepGetObject), L""));
}

template <typename TTraits>
void JobDataStoreReceiveNewArrivedObjects<TTraits>::StepGetObject()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    if (m_uiGetObjectListenerIndex > m_getObjectListeners.size() || m_notifications.size() == m_uiNotificationIndex)
    {
        CompleteJob(qResult(RESULT_SUCCESS));
        return;
    }

    typename TTraits::TSpecificMetaInfo metaInfo;
    bool bFound = false;
    for (typename qVector<typename TTraits::TSpecificMetaInfo>::iterator it = m_metaInfos.begin();
         it != m_metaInfos.end(); ++it)
    {
        if (it->m_uiDataId == m_notifications.at(m_uiNotificationIndex).m_uiDataId)
        {
            metaInfo = *it;
            bFound = true;
        }
    }

    if (!bFound)
    {
        m_nextCheckToken.m_uiLastNotificationId = m_notifications.at(m_uiNotificationIndex).m_uiNotificationId;
        m_nextCheckToken.UpdateChecksum();
        m_pListener->UpdateCheckToken(reinterpret_cast<const unsigned char*>(&m_nextCheckToken), 0,
                                      m_nextCheckToken.m_uiLastNotificationId);
        pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
        if (pCallContext == 0)
        {
            CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
            return;
        }

        ++m_uiNotificationIndex;
        return;
    }

    m_pListener->BeginObject(metaInfo.m_uiDataId, metaInfo.m_uiOwnerId, metaInfo.m_uiSize, metaInfo.m_uiDataType,
                             metaInfo.m_uiVersion, m_notifications.at(m_uiNotificationIndex).m_uiNotificationId);
    pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    delete m_pAdaptor;
    m_pAdaptor = new DataStoreCheckConsistencyEventListenerAdaptor<TTraits>(
        m_getObjectListeners.at(m_uiGetObjectListenerIndex), &m_context, metaInfo);
    typename TTraits::TPrepareGetParam param;
    param.m_uiDataId = m_notifications.at(m_uiNotificationIndex).m_uiDataId;
    if (!(m_pClient->*s_getObjectFunction)(&m_context, param, m_pAdaptor))
    {
        CompleteJob(qResult(RESULT_CORE_CALL_FAILED));
        return;
    }

    Core::GetInstance()->GetCallContextRegister()->AssociateCallContext(m_uiCallId, m_context.GetCallID());
    SetToSuspended();
    ResumeOnCallCompletion(
        &m_context,
        new Step(static_cast<Step::Function>(&JobDataStoreReceiveNewArrivedObjects::StepWaitingGetObject), L""));
}

template <typename TTraits>
void JobDataStoreReceiveNewArrivedObjects<TTraits>::StepWaitingGetObject()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    bool bSkipped = false;
    if (m_context.GetState() == CallContext::CallCanceled)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    if (m_context.GetState() != CallContext::CallSucceeded)
    {
        qResult result = m_context.GetResult();
        if (result.GetFacility() == FACILITY_DATASTORE && result != static_cast<int>(RESULT_DATASTORE_UNKNOWN))
        {
            bSkipped = true;
        }
        else
        {
            CompleteJob(result);
            return;
        }
    }

    m_nextCheckToken.m_uiLastNotificationId = m_notifications.at(m_uiNotificationIndex).m_uiNotificationId;
    m_nextCheckToken.UpdateChecksum();
    m_pListener->UpdateCheckToken(reinterpret_cast<const unsigned char*>(&m_nextCheckToken),
                                  bSkipped ? 0 : m_getObjectListeners.at(m_uiGetObjectListenerIndex),
                                  m_nextCheckToken.m_uiLastNotificationId);
    if (!bSkipped)
        ++m_uiGetObjectListenerIndex;

    ++m_uiNotificationIndex;
    SetStep(Step(static_cast<Step::Function>(&JobDataStoreReceiveNewArrivedObjects::StepGetObject), L""));
}

}
}

#endif
