#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBDATASTOREGETOBJECT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBDATASTOREGETOBJECT_HPP

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

#include "nn/nex/CallContext.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/DataStoreLogicServerClientInterface.hpp"
#include "nn/nex/DataStorePrepareGetParam.hpp"
#include "nn/nex/DataStorePrepareGetParamV1.hpp"
#include "nn/nex/DataStoreReqGetAdditionalMeta.hpp"
#include "nn/nex/DataStoreReqGetInfo.hpp"
#include "nn/nex/DataStoreReqGetInfoV1.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/HttpConnection.hpp"
#include "nn/nex/HttpEventListener.hpp"
#include "nn/nex/JobDataStoreFileServer.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/StepSequenceJob.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

template <typename TTraits>
class JobDataStoreGetObject : public StepSequenceJob, public HttpEventListener
{
public:
    typedef typename TTraits::TDataId TDataId;

    JobDataStoreGetObject();
    virtual ~JobDataStoreGetObject();

    virtual qResult Initialize(unsigned int uiCallId, DataStoreLogicServerClientInterface* pClient,
                               const typename TTraits::TPrepareGetParam& rParam, unsigned int uiThreadPriority,
                               bool bUseHttps, unsigned int uiTransferBufferSize, unsigned int uiMinimumTransferRate,
                               int iMinimumTimeout, typename TTraits::TGetObjectEventListener* pListener,
                               unsigned int uiBufferSize, unsigned char* pReceiveBuffer,
                               unsigned int uiReceiveBufferSize, bool bOrMetaBinary, unsigned int* pReceivedSize,
                               const typename TTraits::TReqGetInfo* pReqGetInfo);

    virtual void PrepareRequest(HttpConnection* pConnection);
    virtual void ProcessResponseHeader(HttpConnection* pConnection, int iStatusCode);
    virtual bool ProcessResponse(const unsigned char* pData, unsigned int uiSize);

    void CompleteJob(const qResult& rResult);
    void StepLogicServerPrepareGetObject();
    void StepWaitingLogicServerPrepareGetObject();
    void StepFileServerGetObject();
    void StepWaitingFileServerGetObject();
    void StepProcessMetaBinary();

private:
    enum Result
    {
        RESULT_SUCCESS = 0x10001,
        RESULT_CORE_OPERATION_ABORTED = static_cast<int>(0x80010004u),
        RESULT_CORE_CALL_FAILED = static_cast<int>(0x8001000Du),
        RESULT_TRANSPORT_RESPONSE_BUFFER_TOO_SMALL = static_cast<int>(0x80050017u),
        RESULT_DATASTORE_UNKNOWN = static_cast<int>(0x80690001u),
        RESULT_DATASTORE_OPERATION_NOT_ALLOWED = static_cast<int>(0x8069000Bu)
    };

    enum
    {
        MILLISECONDS_PER_SECOND = 1000,
        TIMEOUT_INFINITE = 0x7FFFFFFF,
        NUMBER_BASE = 10,
        WAIT_CONTEXT_LISTENER_BUSY = 16
    };

    static uint64_t GetDataId(const DataStorePrepareGetParam& rParam, const DataStoreReqGetInfo& rReqGetInfo)
    {
        return rReqGetInfo.m_uiDataId != 0 ? rReqGetInfo.m_uiDataId : rParam.m_uiDataId;
    }

    static uint32_t GetDataId(const DataStorePrepareGetParamV1& rParam, const DataStoreReqGetInfoV1& rReqGetInfo)
    {
        return rParam.m_uiDataId;
    }

    unsigned int m_uiCallId;
    DataStoreLogicServerClientInterface* m_pClient;
    DataStoreFileServerClient* m_pFileServerClient;
    bool m_bUseHttps;
    unsigned int m_uiTransferBufferSize;
    unsigned int m_uiThreadPriority;
    unsigned int m_uiMinimumTransferRate;
    int m_iMinimumTimeout;
    typename TTraits::TGetObjectEventListener* m_pListener;
    unsigned int m_uiBufferSize;
    ProtocolCallContext m_context;
    typename TTraits::TPrepareGetParam m_prepareParam;
    typename TTraits::TReqGetInfo m_reqGetInfo;
    DataStoreReqGetAdditionalMeta m_additionalMeta;
    unsigned char* m_pReceiveBuffer;
    unsigned int m_uiReceiveBufferSize;
    bool m_bOrMetaBinary;
    unsigned int* m_pReceivedSize;
    unsigned int m_uiReceivedSize;
    bool m_bReceiveBufferOverflow;
    bool m_bContentLengthMismatch;
    bool m_bFirstResponse;
};

template <typename TTraits>
JobDataStoreGetObject<TTraits>::JobDataStoreGetObject()
    : StepSequenceJob(JOB_TYPE_2, DebugString(L"")),
      m_uiCallId(0),
      m_pClient(0),
      m_pFileServerClient(0),
      m_bUseHttps(true),
      m_uiTransferBufferSize(0),
      m_uiThreadPriority(0),
      m_uiMinimumTransferRate(0),
      m_iMinimumTimeout(0),
      m_pListener(0),
      m_uiBufferSize(0),
      m_context(),
      m_prepareParam(),
      m_reqGetInfo(),
      m_additionalMeta(),
      m_pReceiveBuffer(0),
      m_uiReceiveBufferSize(0),
      m_bOrMetaBinary(false),
      m_pReceivedSize(0),
      m_uiReceivedSize(0),
      m_bReceiveBufferOverflow(false),
      m_bContentLengthMismatch(false),
      m_bFirstResponse(true)
{
}

template <typename TTraits>
JobDataStoreGetObject<TTraits>::~JobDataStoreGetObject()
{
    delete m_pFileServerClient;
}

template <typename TTraits>
void JobDataStoreGetObject<TTraits>::CompleteJob(const qResult& rResult)
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext != 0)
    {
        if (rResult)
        {
            if (m_pReceivedSize != 0)
                *m_pReceivedSize = m_reqGetInfo.m_uiSize;

            pCallContext->SignalSuccess(rResult);
        }
        else
        {
            pCallContext->SignalFailure(rResult);
        }
    }
    else
    {
        m_context.Cancel(CallContext::CallCanceled);
    }

    SetToComplete();
}

template <typename TTraits>
void JobDataStoreGetObject<TTraits>::PrepareRequest(HttpConnection* pConnection)
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    pConnection->SetThreadPriority(m_uiThreadPriority);
    pConnection->SetTransferBufferSize(m_uiTransferBufferSize);
    for (qVector<DataStoreKeyValue>::const_iterator it = m_reqGetInfo.m_RequestHeaders.begin();
         it != m_reqGetInfo.m_RequestHeaders.end(); ++it)
    {
        char* pName = 0;
        char* pValue = 0;
        it->m_Key.CreateCopy(&pName);
        it->m_Value.CreateCopy(&pValue);
        pConnection->AddHeaderField(pName, pValue);
        String::ReleaseCopy(pName);
        String::ReleaseCopy(pValue);
    }

    if (m_reqGetInfo.m_RootCaCert.size() != 0)
        pConnection->AddRootCa(&m_reqGetInfo.m_RootCaCert[0], m_reqGetInfo.m_RootCaCert.size());
}

template <typename TTraits>
void JobDataStoreGetObject<TTraits>::ProcessResponseHeader(HttpConnection* pConnection, int iStatusCode)
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
        return;

    String contentLength;
    if (pConnection->GetResponseHeader("Content-Length", &contentLength))
    {
        char* pEnd = 0;
        char* pValue = 0;
        contentLength.CreateCopy(&pValue);
        unsigned long uiContentLength = strtoul(pValue, &pEnd, NUMBER_BASE);
        String::ReleaseCopy(pValue);
        if (uiContentLength != 0 && uiContentLength != m_reqGetInfo.m_uiSize)
        {
            m_bContentLengthMismatch = true;
            m_context.Cancel(CallContext::CallCanceled);
        }
    }
}

template <typename TTraits>
bool JobDataStoreGetObject<TTraits>::ProcessResponse(const unsigned char* pData, unsigned int uiSize)
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
        return false;

    if (m_pListener != 0)
    {
        if (m_bFirstResponse)
        {
            m_bFirstResponse = false;
            TDataId dataId = GetDataId(m_prepareParam, m_reqGetInfo);
            if (m_bOrMetaBinary)
            {
                m_pListener->BeginInternal(dataId, m_reqGetInfo.m_uiSize, m_additionalMeta.m_uiOwnerId,
                                           m_additionalMeta.m_uiDataType, m_additionalMeta.m_uiVersion, true);
                pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
                if (pCallContext == 0)
                    return false;
            }

            m_pListener->Begin(dataId, m_reqGetInfo.m_uiSize);
            pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
            if (pCallContext == 0)
                return false;
        }

        bool bContinue = m_pListener->ProcessResponse(pData, uiSize);
        if (bContinue)
            m_uiReceivedSize += uiSize;

        return bContinue;
    }

    if (m_pReceiveBuffer != 0)
    {
        if (m_uiReceivedSize + uiSize <= m_uiReceiveBufferSize)
        {
            memcpy(m_pReceiveBuffer + m_uiReceivedSize, pData, uiSize);
            m_uiReceivedSize += uiSize;
            return true;
        }

        m_bReceiveBufferOverflow = true;
        m_context.Cancel(CallContext::CallCanceled);
        return false;
    }

    return true;
}

template <typename TTraits>
qResult JobDataStoreGetObject<TTraits>::Initialize(
    unsigned int uiCallId, DataStoreLogicServerClientInterface* pClient,
    const typename TTraits::TPrepareGetParam& rParam, unsigned int uiThreadPriority, bool bUseHttps,
    unsigned int uiTransferBufferSize, unsigned int uiMinimumTransferRate, int iMinimumTimeout,
    typename TTraits::TGetObjectEventListener* pListener, unsigned int uiBufferSize, unsigned char* pReceiveBuffer,
    unsigned int uiReceiveBufferSize, bool bOrMetaBinary, unsigned int* pReceivedSize,
    const typename TTraits::TReqGetInfo* pReqGetInfo)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    m_uiCallId = uiCallId;
    m_pClient = pClient;
    m_pFileServerClient = new DataStoreFileServerClient();
    m_pListener = pListener;
    m_prepareParam = rParam;
    m_uiReceiveBufferSize = uiReceiveBufferSize;
    m_uiThreadPriority = uiThreadPriority;
    m_uiTransferBufferSize = uiTransferBufferSize;
    m_bUseHttps = bUseHttps;
    m_uiMinimumTransferRate = uiMinimumTransferRate;
    m_iMinimumTimeout = iMinimumTimeout;
    m_pReceiveBuffer = pReceiveBuffer;
    m_bOrMetaBinary = bOrMetaBinary;
    m_uiBufferSize = uiBufferSize;
    m_pReceivedSize = pReceivedSize;
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    m_context.SetCredentials(static_cast<ProtocolCallContext*>(pCallContext)->GetCredentials());
    if (pReqGetInfo != 0)
    {
        m_reqGetInfo = *pReqGetInfo;
        SetStep(Step(static_cast<Step::Function>(&JobDataStoreGetObject::StepFileServerGetObject), L""));
        return qResult(true);
    }

    SetStep(Step(static_cast<Step::Function>(&JobDataStoreGetObject::StepLogicServerPrepareGetObject), L""));
    return qResult(true);
}

template <typename TTraits>
void JobDataStoreGetObject<TTraits>::StepLogicServerPrepareGetObject()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    m_context.Reset();
    if (m_bOrMetaBinary)
    {
        if (!m_pClient->PrepareGetObjectOrMetaBinary(&m_context, m_prepareParam, &m_reqGetInfo, &m_additionalMeta))
        {
            CompleteJob(qResult(RESULT_CORE_CALL_FAILED));
            return;
        }
    }
    else if (!m_pClient->PrepareGetObject(&m_context, m_prepareParam, &m_reqGetInfo))
    {
        CompleteJob(qResult(RESULT_CORE_CALL_FAILED));
        return;
    }

    Core::GetInstance()->GetCallContextRegister()->AssociateCallContext(m_uiCallId, m_context.GetCallID());
    SetToSuspended();
    ResumeOnCallCompletion(
        &m_context,
        new Step(static_cast<Step::Function>(&JobDataStoreGetObject::StepWaitingLogicServerPrepareGetObject), L""));
}

template <typename TTraits>
void JobDataStoreGetObject<TTraits>::StepWaitingLogicServerPrepareGetObject()
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

    m_context.Reset();
    if (m_bOrMetaBinary && String::IsEqual(m_reqGetInfo.m_Url, L""))
    {
        SetStep(Step(static_cast<Step::Function>(&JobDataStoreGetObject::StepProcessMetaBinary), L""));
        return;
    }

    SetStep(Step(static_cast<Step::Function>(&JobDataStoreGetObject::StepFileServerGetObject), L""));
}

template <typename TTraits>
void JobDataStoreGetObject<TTraits>::StepFileServerGetObject()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    String url = (m_reqGetInfo.m_Url.FindSubstringNoCase(L"http://") == 0
                  || m_reqGetInfo.m_Url.FindSubstringNoCase(L"https://") == 0)
                     ? m_reqGetInfo.m_Url
                     : (m_bUseHttps ? L"https://" : L"http://") + m_reqGetInfo.m_Url;
    if (m_uiMinimumTransferRate != 0)
    {
        int iTimeout = m_reqGetInfo.m_uiSize / m_uiMinimumTransferRate * MILLISECONDS_PER_SECOND;
        m_context.SetTimeout(std::max(iTimeout, m_iMinimumTimeout));
    }
    else
    {
        m_context.SetTimeout(TIMEOUT_INFINITE);
    }

    if (!m_pFileServerClient->GetObject(&m_context, url, this, m_uiBufferSize))
    {
        CompleteJob(qResult(RESULT_CORE_CALL_FAILED));
        return;
    }

    Core::GetInstance()->GetCallContextRegister()->AssociateCallContext(m_uiCallId, m_context.GetCallID());
    SetToSuspended();
    ResumeOnCallCompletion(
        &m_context, new Step(static_cast<Step::Function>(&JobDataStoreGetObject::StepWaitingFileServerGetObject), L""));
}

template <typename TTraits>
void JobDataStoreGetObject<TTraits>::StepWaitingFileServerGetObject()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    if (m_context.GetState() != CallContext::CallSucceeded)
    {
        if (m_bReceiveBufferOverflow)
        {
            CompleteJob(qResult(RESULT_TRANSPORT_RESPONSE_BUFFER_TOO_SMALL));
            return;
        }

        if (m_bContentLengthMismatch)
        {
            CompleteJob(qResult(RESULT_DATASTORE_UNKNOWN));
            return;
        }

        CompleteJob(m_context.GetResult());
        return;
    }

    if (m_uiReceivedSize != m_reqGetInfo.m_uiSize)
    {
        CompleteJob(qResult(RESULT_DATASTORE_UNKNOWN));
        return;
    }

    CompleteJob(qResult(RESULT_SUCCESS));
}

template <typename TTraits>
void JobDataStoreGetObject<TTraits>::StepProcessMetaBinary()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    if (m_additionalMeta.m_MetaBinary.empty())
    {
        CompleteJob(qResult(RESULT_DATASTORE_OPERATION_NOT_ALLOWED));
        return;
    }

    if (m_pListener != 0)
    {
        if (m_bFirstResponse)
        {
            m_bFirstResponse = false;
            TDataId dataId = GetDataId(m_prepareParam, m_reqGetInfo);
            m_pListener->BeginInternal(dataId, m_additionalMeta.m_MetaBinary.size(), m_additionalMeta.m_uiOwnerId,
                                       m_additionalMeta.m_uiDataType, m_additionalMeta.m_uiVersion, false);
            pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
            if (pCallContext == 0)
            {
                CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
                return;
            }

            m_pListener->Begin(dataId, m_additionalMeta.m_MetaBinary.size());
            pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
            if (pCallContext == 0)
            {
                CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
                return;
            }
        }

        if (!m_pListener->ProcessResponse(m_additionalMeta.m_MetaBinary.front(), m_additionalMeta.m_MetaBinary.size()))
        {
            SetToWaiting(WAIT_CONTEXT_LISTENER_BUSY);
            return;
        }

        m_uiReceivedSize += m_additionalMeta.m_MetaBinary.size();
        CompleteJob(qResult(RESULT_SUCCESS));
        return;
    }

    if (m_pReceiveBuffer != 0)
    {
        if (m_additionalMeta.m_MetaBinary.size() <= m_uiReceiveBufferSize)
        {
            memcpy(m_pReceiveBuffer, m_additionalMeta.m_MetaBinary.front(), m_additionalMeta.m_MetaBinary.size());
            m_uiReceivedSize += m_additionalMeta.m_MetaBinary.size();
            CompleteJob(qResult(RESULT_SUCCESS));
        }
        else
        {
            CompleteJob(qResult(RESULT_TRANSPORT_RESPONSE_BUFFER_TOO_SMALL));
        }
    }
}

}
}

#endif
