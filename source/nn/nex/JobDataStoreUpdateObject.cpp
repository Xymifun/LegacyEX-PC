#include "nn/nex/JobDataStoreUpdateObject.hpp"

#include <algorithm>
#include <string.h>

#include "nn/nex/CallContext.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/DataStoreCompleteUpdateParam.hpp"
#include "nn/nex/DataStoreLogicServerClientInterface.hpp"
#include "nn/nex/DataStorePostHttpEventListener.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/JobDataStoreFileServer.hpp"
#include "nn/nex/MemoryManager.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

JobDataStoreUpdateObject::JobDataStoreUpdateObject()
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
      m_reqUpdateInfo(),
      m_pCopiedData(0),
      m_pData(0),
      m_postResult(RESULT_SUCCESS),
      m_pHttpListener(0)
{
}

JobDataStoreUpdateObject::~JobDataStoreUpdateObject()
{
    delete m_pHttpListener;
    delete m_pFileServerClient;
    if (m_pCopiedData != 0)
        MemoryManager::Free(m_pCopiedData);
}

qResult JobDataStoreUpdateObject::Initialize(unsigned int uiCallId, DataStoreLogicServerClientInterface* pClient,
                                             const DataStorePrepareUpdateParam& rParam, unsigned int uiThreadPriority,
                                             bool bUseHttps, unsigned int uiTransferBufferSize,
                                             unsigned int uiMinimumTransferRate, int iMinimumTimeout,
                                             DataStorePostObjectEventListener* pListener, unsigned int uiBufferSize,
                                             const unsigned char* pData, bool bCopyData)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    m_uiCallId = uiCallId;
    m_pClient = pClient;
    m_pFileServerClient = new DataStoreFileServerClient();
    m_pListener = pListener;
    m_prepareParam = rParam;
    m_uiBufferSize = uiBufferSize;
    m_uiTransferBufferSize = uiTransferBufferSize;
    m_uiThreadPriority = uiThreadPriority;
    m_bUseHttps = bUseHttps;
    m_iMinimumTimeout = iMinimumTimeout;
    m_uiMinimumTransferRate = uiMinimumTransferRate;
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
    SetStep(Step(static_cast<Step::Function>(&JobDataStoreUpdateObject::StepLogicServerPrepareUpdateObject), L""));
    return qResult(true);
}

void JobDataStoreUpdateObject::CompleteJob(const qResult& rResult)
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
        pCallContext->SignalSuccess(rResult);
        SetToComplete();
        return;
    }

    pCallContext->SignalFailure(rResult);
    SetToComplete();
}

void JobDataStoreUpdateObject::StepLogicServerPrepareUpdateObject()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    m_context.Reset();
    if (!m_pClient->PrepareUpdateObject(&m_context, m_prepareParam, &m_reqUpdateInfo))
    {
        CompleteJob(qResult(RESULT_CORE_CALL_FAILED));
        return;
    }

    Core::GetInstance()->GetCallContextRegister()->AssociateCallContext(m_uiCallId, m_context.GetCallID());
    SetToSuspended();
    ResumeOnCallCompletion(
        &m_context, new Step(static_cast<Step::Function>(&JobDataStoreUpdateObject::StepFileServerPostObject), L""));
}

void JobDataStoreUpdateObject::StepFileServerPostObject()
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
        m_uiCallId, m_reqUpdateInfo.m_RequestHeaders, m_reqUpdateInfo.m_FormFields, m_reqUpdateInfo.m_RootCaCert,
        m_uiThreadPriority, m_uiTransferBufferSize, pData, m_prepareParam.m_uiSize, m_pListener);
    String url = (m_reqUpdateInfo.m_Url.FindSubstringNoCase(L"http://") == 0
                  || m_reqUpdateInfo.m_Url.FindSubstringNoCase(L"https://") == 0)
                     ? m_reqUpdateInfo.m_Url
                     : (m_bUseHttps ? L"https://" : L"http://") + m_reqUpdateInfo.m_Url;
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
        new Step(static_cast<Step::Function>(&JobDataStoreUpdateObject::StepLogicServerCompleteUpdateObject), L""));
}

void JobDataStoreUpdateObject::StepLogicServerCompleteUpdateObject()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    if (m_context.GetState() != CallContext::CallSucceeded)
        m_postResult = m_context.GetResult();

    m_context.Reset();
    DataStoreCompleteUpdateParam param;
    param.m_uiDataId = m_prepareParam.m_uiDataId;
    param.m_uiVersion = m_reqUpdateInfo.m_uiVersion;
    param.m_bIsSuccess = m_postResult;
    if (!m_pClient->CompleteUpdateObject(&m_context, param))
    {
        CompleteJob(qResult(RESULT_CORE_CALL_FAILED));
        return;
    }

    Core::GetInstance()->GetCallContextRegister()->AssociateCallContext(m_uiCallId, m_context.GetCallID());
    SetToSuspended();
    ResumeOnCallCompletion(
        &m_context,
        new Step(static_cast<Step::Function>(&JobDataStoreUpdateObject::StepWaitingLogicServerCompleteUpdateObject),
                 L""));
}

void JobDataStoreUpdateObject::StepWaitingLogicServerCompleteUpdateObject()
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

    if (!m_postResult)
    {
        CompleteJob(m_postResult);
        return;
    }

    CompleteJob(qResult(RESULT_SUCCESS));
}

}
}
