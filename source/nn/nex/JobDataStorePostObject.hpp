#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBDATASTOREPOSTOBJECT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBDATASTOREPOSTOBJECT_HPP

#include <algorithm>
#include <string.h>

#include "nn/nex/CallContext.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/DataStoreLogicServerClientInterface.hpp"
#include "nn/nex/DataStorePostHttpEventListener.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/JobDataStoreFileServer.hpp"
#include "nn/nex/MemoryManager.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/StepSequenceJob.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class DataStorePostObjectEventListener;

template <typename TTraits>
class JobDataStorePostObject : public StepSequenceJob
{
public:
    JobDataStorePostObject();
    virtual ~JobDataStorePostObject();

    virtual qResult Initialize(unsigned int uiCallId, DataStoreLogicServerClientInterface* pClient,
                               const typename TTraits::TPreparePostParam& rParam, unsigned int uiThreadPriority,
                               bool bUseHttps, unsigned int uiTransferBufferSize, unsigned int uiMinimumTransferRate,
                               int iMinimumTimeout, typename TTraits::TDataId* pDataId,
                               DataStorePostObjectEventListener* pListener, unsigned int uiBufferSize,
                               const unsigned char* pData, bool bCopyData);

    void CompleteJob(const qResult& rResult);
    void StepLogicServerPreparePostObject();
    void StepFileServerPostObject();
    void StepLogicServerCompletePostObject();
    void StepWaitingLogicServerCompletePostObject();

private:
    enum Result
    {
        RESULT_SUCCESS = 0x10001,
        RESULT_CORE_OPERATION_ABORTED = static_cast<int>(0x80010004u),
        RESULT_CORE_CALL_FAILED = static_cast<int>(0x8001000Du)
    };

    enum
    {
        MILLISECONDS_PER_SECOND = 1000,
        TIMEOUT_INFINITE = 0x7FFFFFFF
    };

    enum DataFlag
    {
        DATA_FLAG_NEED_COMPLETION = 0x40
    };

    unsigned int m_uiCallId;
    DataStoreLogicServerClientInterface* m_pClient;
    DataStoreFileServerClient* m_pFileServerClient;
    bool m_bUseHttps;
    unsigned int m_uiTransferBufferSize;
    unsigned int m_uiThreadPriority;
    unsigned int m_uiMinimumTransferRate;
    int m_iMinimumTimeout;
    DataStorePostObjectEventListener* m_pListener;
    unsigned int m_uiBufferSize;
    ProtocolCallContext m_context;
    typename TTraits::TPreparePostParam m_prepareParam;
    typename TTraits::TDataId* m_pDataId;
    typename TTraits::TReqPostInfo m_reqPostInfo;
    unsigned char* m_pCopiedData;
    const unsigned char* m_pData;
    qResult m_postResult;
    DataStorePostHttpEventListener* m_pHttpListener;
};

template <typename TTraits>
JobDataStorePostObject<TTraits>::JobDataStorePostObject()
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
      m_pDataId(0),
      m_reqPostInfo(),
      m_pCopiedData(0),
      m_pData(0),
      m_postResult(true),
      m_pHttpListener(0)
{
}

template <typename TTraits>
JobDataStorePostObject<TTraits>::~JobDataStorePostObject()
{
    delete m_pHttpListener;
    delete m_pFileServerClient;
    if (m_pCopiedData != 0)
        MemoryManager::Free(m_pCopiedData);
}

template <typename TTraits>
qResult JobDataStorePostObject<TTraits>::Initialize(
    unsigned int uiCallId, DataStoreLogicServerClientInterface* pClient,
    const typename TTraits::TPreparePostParam& rParam, unsigned int uiThreadPriority, bool bUseHttps,
    unsigned int uiTransferBufferSize, unsigned int uiMinimumTransferRate, int iMinimumTimeout,
    typename TTraits::TDataId* pDataId, DataStorePostObjectEventListener* pListener, unsigned int uiBufferSize,
    const unsigned char* pData, bool bCopyData)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    m_uiCallId = uiCallId;
    m_pClient = pClient;
    m_pFileServerClient = new DataStoreFileServerClient();
    m_pListener = pListener;
    m_prepareParam = rParam;
    m_uiThreadPriority = uiThreadPriority;
    m_iMinimumTimeout = iMinimumTimeout;
    m_uiTransferBufferSize = uiTransferBufferSize;
    m_uiMinimumTransferRate = uiMinimumTransferRate;
    m_bUseHttps = bUseHttps;
    m_uiBufferSize = uiBufferSize;
    m_pDataId = pDataId;
    if (pData != 0)
    {
        if (bCopyData)
        {
            m_pCopiedData = static_cast<unsigned char*>(MemoryManager::Allocate(rParam.m_uiSize));
            memcpy(m_pCopiedData, pData, rParam.m_uiSize);
        }
        else
        {
            m_pData = pData;
        }
    }

    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    m_context.SetCredentials(static_cast<ProtocolCallContext*>(pCallContext)->GetCredentials());
    SetStep(Step(static_cast<Step::Function>(&JobDataStorePostObject::StepLogicServerPreparePostObject), L""));
    return qResult(true);
}

template <typename TTraits>
void JobDataStorePostObject<TTraits>::CompleteJob(const qResult& rResult)
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext != 0)
    {
        if (rResult)
        {
            *m_pDataId = m_reqPostInfo.m_uiDataId;
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
void JobDataStorePostObject<TTraits>::StepLogicServerPreparePostObject()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    m_context.Reset();
    if (!m_pClient->PreparePostObject(&m_context, m_prepareParam, &m_reqPostInfo))
    {
        CompleteJob(qResult(RESULT_CORE_CALL_FAILED));
        return;
    }

    Core::GetInstance()->GetCallContextRegister()->AssociateCallContext(m_uiCallId, m_context.GetCallID());
    SetToSuspended();
    ResumeOnCallCompletion(
        &m_context, new Step(static_cast<Step::Function>(&JobDataStorePostObject::StepFileServerPostObject), L""));
}

template <typename TTraits>
void JobDataStorePostObject<TTraits>::StepFileServerPostObject()
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
    const unsigned char* pData = m_pCopiedData != 0 ? m_pCopiedData : m_pData;
    m_pHttpListener = new DataStorePostHttpEventListener(
        m_uiCallId, m_reqPostInfo.m_RequestHeaders, m_reqPostInfo.m_FormFields, m_reqPostInfo.m_RootCaCert,
        m_uiThreadPriority, m_uiTransferBufferSize, pData, m_prepareParam.m_uiSize, m_pListener);
    String url = (m_reqPostInfo.m_Url.FindSubstringNoCase(L"http://") == 0
                  || m_reqPostInfo.m_Url.FindSubstringNoCase(L"https://") == 0)
                     ? m_reqPostInfo.m_Url
                     : (m_bUseHttps ? L"https://" : L"http://") + m_reqPostInfo.m_Url;
    if (m_uiMinimumTransferRate != 0)
    {
        int iTimeout = m_prepareParam.m_uiSize / m_uiMinimumTransferRate * MILLISECONDS_PER_SECOND;
        m_context.SetTimeout(std::max(iTimeout, m_iMinimumTimeout));
    }
    else
    {
        m_context.SetTimeout(TIMEOUT_INFINITE);
    }

    if (!m_pFileServerClient->PostObject(&m_context, url, m_pHttpListener, m_uiBufferSize))
    {
        CompleteJob(qResult(RESULT_CORE_CALL_FAILED));
        return;
    }

    Core::GetInstance()->GetCallContextRegister()->AssociateCallContext(m_uiCallId, m_context.GetCallID());
    SetToSuspended();
    ResumeOnCallCompletion(
        &m_context,
        new Step(static_cast<Step::Function>(&JobDataStorePostObject::StepLogicServerCompletePostObject), L""));
}

template <typename TTraits>
void JobDataStorePostObject<TTraits>::StepLogicServerCompletePostObject()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    if (m_context.GetState() != CallContext::CallSucceeded)
        m_postResult = m_context.GetResult();

    if (m_prepareParam.m_uiFlag & DATA_FLAG_NEED_COMPLETION)
    {
        CompleteJob(m_postResult);
        return;
    }

    m_context.Reset();
    typename TTraits::TCompletePostParam param;
    param.m_uiDataId = m_reqPostInfo.m_uiDataId;
    param.m_bIsSuccess = m_postResult;
    if (!m_pClient->CompletePostObject(&m_context, param))
    {
        CompleteJob(m_postResult ? m_postResult : qResult(RESULT_CORE_CALL_FAILED));
        return;
    }

    Core::GetInstance()->GetCallContextRegister()->AssociateCallContext(m_uiCallId, m_context.GetCallID());
    SetToSuspended();
    ResumeOnCallCompletion(
        &m_context,
        new Step(static_cast<Step::Function>(&JobDataStorePostObject::StepWaitingLogicServerCompletePostObject), L""));
}

template <typename TTraits>
void JobDataStorePostObject<TTraits>::StepWaitingLogicServerCompletePostObject()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    if (!m_postResult)
    {
        CompleteJob(m_postResult);
        return;
    }

    if (m_context.GetState() != CallContext::CallSucceeded)
    {
        CompleteJob(m_context.GetResult());
        return;
    }

    CompleteJob(qResult(RESULT_SUCCESS));
}

}
}

#endif
