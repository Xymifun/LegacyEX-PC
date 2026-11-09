#include "nn/nex/JobDataStoreHpp.hpp"

#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/ClientProtocol.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/DataStorePostHttpEventListener.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/HMACChecksum.hpp"
#include "nn/nex/Key.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/Protocol.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/StringStream.hpp"

namespace nn
{
namespace nex
{

JobDataStoreHpp::JobDataStoreHpp()
    : StepSequenceJob(JOB_TYPE_2, DebugString(L"")),
      m_uiCallId(0),
      m_url(),
      m_method(HttpConnection::HTTP_POST),
      m_request(),
      m_callContext(),
      m_httpClient(),
      m_uiHttpRequestCount(0),
      m_iStatusCode(0),
      m_response(),
      m_bResponseTooLarge(false),
      m_location(L""),
      m_uiThreadPriority(0),
      m_uiPrincipalId(0),
      m_password(L""),
      m_accessKey(L""),
      m_iTimeout(INFINITE_TIMEOUT),
      m_retrieveGameAuthTokenClient(),
      m_token(),
      m_key(),
      m_pListener(0),
      m_headers(),
      m_formFields(),
      m_rootCa(),
      m_priorityType(ServiceHttpClient::PRIORITY_TYPE_DEFAULT)
{
}

JobDataStoreHpp::~JobDataStoreHpp()
{
    delete m_pListener;
}

qResult JobDataStoreHpp::Initialize(unsigned int uiCallId, ClientProtocol* pProtocol, const Buffer& rRequest,
                                    unsigned int uiThreadPriority, unsigned int uiPrincipalId, const String& rPassword,
                                    const String& rAccessKey, const String& rUrl, int iTimeout,
                                    ServiceHttpClient::PriorityType priorityType)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    m_uiCallId = uiCallId;
    m_url = rUrl;
    m_request = rRequest;
    m_pProtocol = pProtocol;
    m_uiThreadPriority = uiThreadPriority;
    m_uiPrincipalId = uiPrincipalId;
    m_password = rPassword;
    m_accessKey = rAccessKey;
    m_iTimeout = iTimeout;
    m_priorityType = priorityType;
    SetStep(Step(static_cast<Step::Function>(&JobDataStoreHpp::StepRetrieveGameAuthToken), L""));
    return qResult(true);
}

void JobDataStoreHpp::PrepareRequest(HttpConnection* pConnection)
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    m_pListener->PrepareRequest(pConnection);
}

void JobDataStoreHpp::ProcessResponseHeader(HttpConnection* pConnection, int iStatusCode)
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
}

bool JobDataStoreHpp::ProcessResponse(const unsigned char* pData, unsigned int uiSize)
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

    m_bResponseTooLarge = true;
    m_callContext.Cancel(CallContext::CallCanceled);
    return true;
}

unsigned int JobDataStoreHpp::PostChunkedBuffer(HttpConnection* pConnection, unsigned char* pBuffer,
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

void JobDataStoreHpp::CompleteJob(const qResult& rResult)
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
        m_callContext.Cancel(CallContext::CallCanceled);
    else if (rResult)
        pCallContext->SignalSuccess(rResult);
    else
        pCallContext->SignalFailure(rResult);

    SetToComplete();
}

inline void JobDataStoreHpp::AddRequestHeader(const String& rKey, const String& rValue)
{
    DataStoreKeyValue header;
    header.m_Key = rKey;
    header.m_Value = rValue;
    m_headers.push_back(header);
}

void JobDataStoreHpp::PrepareRequestHeaders()
{
    m_headers.clear();

    Buffer signature1;
    {
        HMACChecksum hmac;
        String key(m_accessKey);
        while (key.GetLength() < SIGNATURE_KEY_MIN_LENGTH)
            key += L"0";
        hmac.SetKey(Key(key));
        hmac.ComputeChecksum(m_request, &signature1);
    }

    Buffer signature2;
    {
        HMACChecksum hmac;
        String key(m_key);
        while (key.GetLength() < SIGNATURE_KEY_MIN_LENGTH)
            key += L"0";
        hmac.SetKey(Key(m_key));
        hmac.ComputeChecksum(m_request, &signature2);
    }

    String pid;
    pid.Format(L"%u", m_uiPrincipalId);
    AddRequestHeader("pid", pid);
    AddRequestHeader("version", "NEX_3_10_0");
    AddRequestHeader("token", m_token);
    AddRequestHeader("signature1", signature1.ToString());
    AddRequestHeader("signature2", signature2.ToString());
    if (m_priorityType != ServiceHttpClient::PRIORITY_TYPE_DEFAULT)
    {
        StringStream stream;
        stream.StreamNumber(m_priorityType);
        AddRequestHeader("Priority-Type", stream.GetBuffer());
    }
}

void JobDataStoreHpp::StepRetrieveGameAuthToken()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    m_callContext.Reset();
    if (!m_retrieveGameAuthTokenClient.RetrieveGameAuthToken(&m_callContext, m_uiPrincipalId, m_password, &m_token,
                                                             &m_key))
    {
        CompleteJob(qResult(RESULT_CORE_CALL_INITIATION_FAILURE));
        return;
    }

    Core::GetInstance()->GetCallContextRegister()->AssociateCallContext(m_uiCallId, m_callContext.GetCallID());
    SetToSuspended();
    ResumeOnCallCompletion(
        &m_callContext, new Step(static_cast<Step::Function>(&JobDataStoreHpp::StepWaitingRetrieveGameAuthToken), L""));
}

void JobDataStoreHpp::StepWaitingRetrieveGameAuthToken()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    if (m_callContext.GetState() != CallContext::CallSucceeded)
    {
        CompleteJob(m_callContext.GetResult());
        return;
    }

    SetStep(Step(static_cast<Step::Function>(&JobDataStoreHpp::StepHttp), L""));
}

void JobDataStoreHpp::ResetHttpResult()
{
    m_iStatusCode = 0;
    m_bResponseTooLarge = false;
    m_response.clear();
    m_location = L"";
    ++m_uiHttpRequestCount;
}

void JobDataStoreHpp::StepHttp()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    ResetHttpResult();
    m_callContext.Reset();
    m_callContext.SetTimeout(m_iTimeout);
    PrepareRequestHeaders();
    if (m_pListener != 0)
    {
        delete m_pListener;
        m_pListener = 0;
    }

    m_pListener = new DataStorePostHttpEventListener(m_uiCallId, m_headers, m_formFields, m_rootCa, m_uiThreadPriority,
                                                     HTTP_BUFFER_SIZE, m_request.GetData(), m_request.GetLength(), 0);
    if (!m_httpClient.Start(&m_callContext, m_url, m_method, this, HTTP_BUFFER_SIZE))
    {
        CompleteJob(qResult(RESULT_CORE_CALL_INITIATION_FAILURE));
        return;
    }

    Core::GetInstance()->GetCallContextRegister()->AssociateCallContext(m_uiCallId, m_callContext.GetCallID());
    SetToSuspended();
    ResumeOnCallCompletion(&m_callContext,
                           new Step(static_cast<Step::Function>(&JobDataStoreHpp::StepWaitingForHttp), L""));
}

bool JobDataStoreHpp::IsHttpStatusCodeSuccess()
{
    return 0 < m_iStatusCode && m_iStatusCode < 300;
}

qResult JobDataStoreHpp::ProcessFinalResponse(const qVector<unsigned char>& rResponse)
{
    if (rResponse.size() < MIN_RESPONSE_SIZE)
        return qResult(RESULT_TRANSPORT_UNKNOWN);

    Buffer buffer(rResponse.size(), 0);
    buffer.AppendData(&rResponse[0], rResponse.size(), APPEND_AT_END);
    Message message(&buffer);
    qResult result;
    Protocol::ExtractCallOutcome(&message, &result);
    ProtocolCallContext* pProtocolCallContext = Protocol::RetrieveCallContext(&message);
    if (pProtocolCallContext == 0 || pProtocolCallContext->GetCallID() != m_uiCallId)
    {
        const unsigned int uiPosition = message.GetPosition() - sizeof(unsigned int);
        if (uiPosition <= message.GetLength())
            message.SetPosition(uiPosition);

        unsigned int uiCallId;
        message.ExtractPrimitive(&uiCallId);
        return qResult(RESULT_TRANSPORT_UNKNOWN);
    }

    if (result)
        m_pProtocol->ExtractCallSpecificResults(&message, pProtocolCallContext);

    return result;
}

void JobDataStoreHpp::StepWaitingForHttp()
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    if (m_callContext.GetState() != CallContext::CallSucceeded)
    {
        CompleteJob(m_callContext.GetResult());
        return;
    }

    if (IsHttpStatusCodeSuccess())
    {
        CompleteJob(ProcessFinalResponse(m_response));
        return;
    }

    m_response.push_back(0);
    if (m_bResponseTooLarge)
    {
        CompleteJob(qResult(RESULT_TRANSPORT_SERVER_REQUEST_ERROR));
        return;
    }

    switch (m_iStatusCode)
    {
    case HTTP_STATUS_TEMPORARY_REDIRECT:
        if (String::IsEqual(m_location, L""))
        {
            CompleteJob(qResult(RESULT_TRANSPORT_TEMPORARY_SERVER_ERROR));
            return;
        }

        if (m_uiHttpRequestCount > MAX_REDIRECT_COUNT)
        {
            CompleteJob(qResult(RESULT_TRANSPORT_TEMPORARY_SERVER_ERROR));
            return;
        }

        m_url = m_location;
        SetStep(Step(static_cast<Step::Function>(&JobDataStoreHpp::StepHttp), L""));
        return;
    case HTTP_STATUS_BAD_REQUEST:
    case HTTP_STATUS_FORBIDDEN:
        CompleteJob(qResult(RESULT_TRANSPORT_SERVER_REQUEST_ERROR));
        return;
    case HTTP_STATUS_REQUEST_ENTITY_TOO_LARGE:
        CompleteJob(qResult(RESULT_TRANSPORT_SERVER_REQUEST_ERROR));
        return;
    case HTTP_STATUS_INTERNAL_SERVER_ERROR:
        if (m_uiHttpRequestCount != 0)
        {
            CompleteJob(qResult(RESULT_TRANSPORT_TEMPORARY_SERVER_ERROR));
            return;
        }

        SetStep(Step(static_cast<Step::Function>(&JobDataStoreHpp::StepHttp), L""));
        return;
    case HTTP_STATUS_SERVICE_UNAVAILABLE:
        CompleteJob(qResult(RESULT_TRANSPORT_SERVICE_UNAVAILABLE));
        return;
    default:
        CompleteJob(qResult(RESULT_TRANSPORT_PERMANENT_SERVER_ERROR));
        return;
    }
}

}
}
