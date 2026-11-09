#include "nn/nex/JobDataStoreFileServer.hpp"

#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/DebugString.hpp"

namespace nn
{
namespace nex
{

JobDataStoreFileServer::JobDataStoreFileServer()
    : StepSequenceJob(JOB_TYPE_2, DebugString(L"")),
      m_uiCallId(0),
      m_url(L""),
      m_method(HttpConnection::HTTP_GET),
      m_pListener(0),
      m_httpContext(),
      m_httpClient(),
      m_uiAttemptCount(0),
      m_iStatusCode(0),
      m_errorResponse(),
      m_bErrorResponseOverflow(false),
      m_location(L""),
      m_uiBufferSize(0)
{
}

JobDataStoreFileServer::~JobDataStoreFileServer()
{
}

void JobDataStoreFileServer::CompleteJob(const qResult& rResult)
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        m_httpContext.Cancel(CallContext::CallCanceled);
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

void JobDataStoreFileServer::PrepareRequest(HttpConnection* pConnection)
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    pConnection->SetUnknown9(false);
    pConnection->SetUnknown10(false);
    m_pListener->PrepareRequest(pConnection);
}

unsigned int JobDataStoreFileServer::PostChunkedBuffer(HttpConnection* pConnection, unsigned char* pBuffer,
                                                       unsigned int uiSize, bool* pbLastChunk)
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return 0;
    }

    return m_pListener->PostChunkedBuffer(pConnection, pBuffer, uiSize, pbLastChunk);
}

bool JobDataStoreFileServer::IsHttpStatusCodeSuccess()
{
    return m_iStatusCode >= HTTP_STATUS_SUCCESS_MIN && m_iStatusCode <= HTTP_STATUS_SUCCESS_MAX;
}

void JobDataStoreFileServer::ProcessResponseHeader(HttpConnection* pConnection, int iStatusCode)
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    m_iStatusCode = iStatusCode;
    if (iStatusCode == HTTP_STATUS_TEMPORARY_REDIRECT)
        pConnection->GetResponseHeader("Location", &m_location);

    if (IsHttpStatusCodeSuccess())
        m_pListener->ProcessResponseHeader(pConnection, iStatusCode);
}

bool JobDataStoreFileServer::ProcessResponse(const unsigned char* pData, unsigned int uiSize)
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return false;
    }

    if (!IsHttpStatusCodeSuccess())
    {
        if (m_errorResponse.size() + uiSize > MAX_ERROR_RESPONSE_SIZE)
        {
            m_bErrorResponseOverflow = true;
            m_httpContext.Cancel(CallContext::CallCanceled);
            return true;
        }

        m_errorResponse.insert(m_errorResponse.end(), pData, pData + uiSize);
    }
    else if (m_method != HttpConnection::HTTP_POST)
    {
        m_pListener->ProcessResponse(pData, uiSize);
    }

    return true;
}

qResult JobDataStoreFileServer::Initialize(unsigned int uiCallId, const String& rUrl, HttpConnection::Method method,
                                           HttpEventListener* pListener, unsigned int uiBufferSize)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    m_uiCallId = uiCallId;
    m_url = rUrl;
    m_pListener = pListener;
    m_uiBufferSize = uiBufferSize;
    m_method = method;
    SetStep(Step(static_cast<Step::Function>(&JobDataStoreFileServer::StepHttp), L""));
    return qResult(true);
}

void JobDataStoreFileServer::ResetHttpResult()
{
    m_iStatusCode = 0;
    m_bErrorResponseOverflow = false;
    m_errorResponse.clear();
    m_location = L"";
    ++m_uiAttemptCount;
}

void JobDataStoreFileServer::StepHttp()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    ResetHttpResult();
    m_httpContext.Reset();
    if (!m_httpClient.Start(&m_httpContext, m_url, m_method, this, m_uiBufferSize))
    {
        CompleteJob(qResult(RESULT_CORE_CALL_FAILED));
        return;
    }

    Core::GetInstance()->GetCallContextRegister()->AssociateCallContext(m_uiCallId, m_httpContext.GetCallID());
    SetToSuspended();
    ResumeOnCallCompletion(&m_httpContext,
                           new Step(static_cast<Step::Function>(&JobDataStoreFileServer::StepWaitingForHttp), L""));
}

void JobDataStoreFileServer::StepWaitingForHttp()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    if (m_httpContext.GetState() != CallContext::CallSucceeded)
    {
        CompleteJob(m_httpContext.GetResult());
        return;
    }

    if (IsHttpStatusCodeSuccess())
    {
        CompleteJob(qResult(RESULT_SUCCESS));
        return;
    }

    m_errorResponse.push_back('\0');
    if (m_bErrorResponseOverflow)
    {
        CompleteJob(qResult(RESULT_TRANSPORT_PERMANENT_SERVER_ERROR));
        return;
    }

    switch (m_iStatusCode)
    {
    case HTTP_STATUS_TEMPORARY_REDIRECT:
        if (String::IsEqual(m_location, L""))
        {
            CompleteJob(qResult(RESULT_TRANSPORT_PERMANENT_SERVER_ERROR));
            return;
        }

        if (m_uiAttemptCount > MAX_REDIRECT_ATTEMPTS)
        {
            CompleteJob(qResult(RESULT_TRANSPORT_PERMANENT_SERVER_ERROR));
            return;
        }

        m_url = m_location;
        SetStep(Step(static_cast<Step::Function>(&JobDataStoreFileServer::StepHttp), L""));
        return;

    case HTTP_STATUS_NOT_FOUND:
        CompleteJob(qResult(RESULT_DATASTORE_NOT_FOUND));
        return;

    case HTTP_STATUS_CONFLICT:
    case HTTP_STATUS_INTERNAL_SERVER_ERROR:
        if (m_uiAttemptCount > MAX_RETRY_ATTEMPTS)
        {
            CompleteJob(qResult(RESULT_TRANSPORT_TEMPORARY_SERVER_ERROR));
            return;
        }

        SetStep(Step(static_cast<Step::Function>(&JobDataStoreFileServer::StepHttp), L""));
        return;

    default:
        CompleteJob(qResult(RESULT_TRANSPORT_PERMANENT_SERVER_ERROR));
        return;
    }
}

}
}
