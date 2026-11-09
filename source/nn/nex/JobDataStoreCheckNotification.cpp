#include "nn/nex/JobDataStoreCheckNotification.hpp"

#include <stdlib.h>

#include "nn/nex/CallContext.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/DataStoreGetNotificationUrlParam.hpp"
#include "nn/nex/DataStoreLogicServerClientInterface.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/HttpConnection.hpp"
#include "nn/nex/JobDataStoreFileServer.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

JobDataStoreCheckNotification::~JobDataStoreCheckNotification()
{
    delete m_pFileServerClient;
}

void JobDataStoreCheckNotification::CompleteJob(const qResult& rResult)
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        m_context.Cancel(CallContext::CallCanceled);
        SetToComplete();
        return;
    }

    if (rResult)
    {
        if (m_bInvalidNotification)
        {
            *m_pNextNotificationUrl = L"invalidurl";
            *m_pLastDataId = 0;
            *m_pSequence = 0;
            pCallContext->SignalSuccess(rResult);
            SetToComplete();
            return;
        }

        *m_pNextNotificationUrl = m_notificationUrl;
        *m_pLastDataId = m_uiLastDataId;
        *m_pSequence = m_uiSequence;
        pCallContext->SignalSuccess(rResult);
        SetToComplete();
        return;
    }

    pCallContext->SignalFailure(rResult);
    SetToComplete();
}

void JobDataStoreCheckNotification::PrepareRequest(HttpConnection* pConnection)
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    pConnection->SetThreadPriority(m_uiThreadPriority);
    if (m_urlInfo.m_RootCaCert.size() != 0)
        pConnection->AddRootCa(&m_urlInfo.m_RootCaCert[0], m_urlInfo.m_RootCaCert.size());
}

void JobDataStoreCheckNotification::ProcessResponseHeader(HttpConnection* pConnection, int iStatusCode)
{
    m_iStatusCode = iStatusCode;
}

bool JobDataStoreCheckNotification::ProcessResponse(const unsigned char* pData, unsigned int uiSize)
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return false;
    }

    if (m_response.size() + uiSize <= MAX_RESPONSE_SIZE)
    {
        m_response.insert(m_response.end(), pData, pData + uiSize);
        return true;
    }

    m_context.Cancel(CallContext::CallCanceled);
    m_bResponseOverflow = true;
    return false;
}

JobDataStoreCheckNotification::JobDataStoreCheckNotification()
    : StepSequenceJob(JOB_TYPE_2, DebugString(L"")),
      m_uiCallId(0),
      m_pClient(0),
      m_pFileServerClient(0),
      m_pNextNotificationUrl(0),
      m_notificationUrl(L""),
      m_uiThreadPriority(0),
      m_iTimeout(0),
      m_context(),
      m_pLastDataId(0),
      m_uiLastDataId(0),
      m_pSequence(0),
      m_uiSequence(0),
      m_uiKey(0),
      m_urlInfo(),
      m_response(),
      m_iStatusCode(0),
      m_bResponseOverflow(false),
      m_bForceUrlRequest(false),
      m_pbUrlRequestFailed(0),
      m_bInvalidNotification(false)
{
}

qResult JobDataStoreCheckNotification::Initialize(unsigned int uiCallId, DataStoreLogicServerClientInterface* pClient,
                                                  unsigned int uiThreadPriority, int iTimeout,
                                                  const String& rNotificationUrl, String* pNextNotificationUrl,
                                                  uint64_t uiLastDataId, uint64_t* pLastDataId, unsigned int uiSequence,
                                                  unsigned int* pSequence, unsigned int uiKey, bool* pbUrlRequestFailed,
                                                  bool bForceUrlRequest)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    m_pClient = pClient;
    m_uiCallId = uiCallId;
    m_pFileServerClient = new DataStoreFileServerClient();
    m_pNextNotificationUrl = pNextNotificationUrl;
    m_iTimeout = iTimeout;
    m_uiThreadPriority = uiThreadPriority;
    m_notificationUrl = rNotificationUrl;
    m_uiSequence = uiSequence;
    m_uiLastDataId = uiLastDataId;
    m_uiKey = uiKey;
    m_pSequence = pSequence;
    m_pbUrlRequestFailed = pbUrlRequestFailed;
    m_bForceUrlRequest = bForceUrlRequest;
    m_pLastDataId = pLastDataId;
    *pbUrlRequestFailed = false;
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    m_context.SetCredentials(static_cast<ProtocolCallContext*>(pCallContext)->GetCredentials());
    if (!m_bForceUrlRequest && !String::IsEqual(m_notificationUrl, L"")
        && !String::IsEqual(m_notificationUrl, L"invalidurl") && m_uiSequence != 0)
    {
        SetStep(Step(static_cast<Step::Function>(&JobDataStoreCheckNotification::StepFileServerCheckNotificationFile),
                     L""));
        return qResult(true);
    }

    SetStep(Step(static_cast<Step::Function>(&JobDataStoreCheckNotification::StepLogicServerGetNotificationUrl), L""));
    return qResult(true);
}

void JobDataStoreCheckNotification::StepLogicServerGetNotificationUrl()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    m_context.Reset();
    DataStoreGetNotificationUrlParam param;
    if (!m_bResponseOverflow)
        param.m_PreviousUrl = m_notificationUrl;

    if (!m_pClient->GetNotificationUrl(&m_context, param, &m_urlInfo))
    {
        *m_pbUrlRequestFailed = true;
        CompleteJob(qResult(RESULT_CORE_CALL_FAILED));
        return;
    }

    Core::GetInstance()->GetCallContextRegister()->AssociateCallContext(m_uiCallId, m_context.GetCallID());
    SetToSuspended();
    ResumeOnCallCompletion(
        &m_context,
        new Step(static_cast<Step::Function>(&JobDataStoreCheckNotification::StepWaitingLogicServerGetNotificationUrl),
                 L""));
}

void JobDataStoreCheckNotification::StepWaitingLogicServerGetNotificationUrl()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    if (m_context.GetState() == CallContext::CallSucceeded)
    {
        m_notificationUrl = m_urlInfo.m_Url + m_urlInfo.m_Key + m_urlInfo.m_Query;
        if (m_notificationUrl.FindSubstringNoCase(L"http://") == 0
            || m_notificationUrl.FindSubstringNoCase(L"https://") == 0)
        {
            m_notificationUrl = m_notificationUrl + L"";
        }
        else
        {
            m_notificationUrl = L"https://" + m_notificationUrl;
        }

        if (m_uiSequence == 0)
            m_uiSequence = 1;

        CompleteJob(qResult(RESULT_SUCCESS));
        return;
    }

    *m_pbUrlRequestFailed = true;
    CompleteJob(m_context.GetResult());
}

void JobDataStoreCheckNotification::StepFileServerCheckNotificationFile()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    m_context.Reset();
    m_context.SetTimeout(m_iTimeout);
    if (!m_pFileServerClient->GetObject(&m_context, m_notificationUrl, this, RESPONSE_BUFFER_SIZE))
    {
        CompleteJob(qResult(RESULT_CORE_CALL_FAILED));
        return;
    }

    Core::GetInstance()->GetCallContextRegister()->AssociateCallContext(m_uiCallId, m_context.GetCallID());
    SetToSuspended();
    ResumeOnCallCompletion(&m_context,
                           new Step(static_cast<Step::Function>(
                                        &JobDataStoreCheckNotification::StepWaitingFileServerCheckNotificationFile),
                                    L""));
}

void JobDataStoreCheckNotification::StepWaitingFileServerCheckNotificationFile()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    if (m_context.GetState() == CallContext::CallSucceeded)
    {
        unsigned int uiKey = m_uiKey;
        unsigned int uiSequence = m_uiSequence;
        m_response.push_back('\0');
        char* pEnd = 0;
        uint64_t uiDataId = strtoull(reinterpret_cast<char*>(&m_response[0]), &pEnd, NUMBER_BASE);
        if (*pEnd == ',')
        {
            unsigned long uiFileKey = strtoul(pEnd + 1, &pEnd, NUMBER_BASE);
            if (uiKey != 0 && *pEnd == ',' && uiFileKey == uiKey)
            {
                unsigned long uiFileSequence = strtoul(pEnd + 1, &pEnd, NUMBER_BASE);
                if ((uiFileSequence != 0 && uiSequence == uiFileSequence) || uiSequence == 1 || uiSequence == 0)
                {
                    m_uiSequence = uiFileSequence;
                    if (uiDataId > m_uiLastDataId)
                        m_uiLastDataId = uiDataId;

                    CompleteJob(qResult(RESULT_SUCCESS));
                    return;
                }
            }
        }

        m_bInvalidNotification = true;
        CompleteJob(qResult(RESULT_SUCCESS));
        return;
    }

    qResult result = m_context.GetResult();
    if (!m_bResponseOverflow)
    {
        CompleteJob(result);
        return;
    }

    SetStep(Step(static_cast<Step::Function>(&JobDataStoreCheckNotification::StepLogicServerGetNotificationUrl), L""));
}

}
}
