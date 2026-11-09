#include "nn/nex/JobHttp.hpp"

#include "nn/nex/CallContext.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/DebugString.hpp"
#include "nn/nex/HttpEventListener.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

JobHttp::JobHttp()
    : StepSequenceJob(JOB_TYPE_0, DebugString(L"")),
      m_uiCallId(0),
      m_pConnection(0),
      m_url(L""),
      m_method(HttpConnection::HTTP_GET),
      m_receiveBuffer(),
      m_bHeaderReceived(false),
      m_pListener(0),
      m_uiBufferSize(0),
      m_iWaitTime(g_dispatcherThreadEnabled ? WAIT_TIME_DISPATCHER_THREAD : WAIT_TIME)
{
}

JobHttp::~JobHttp()
{
    if (m_pConnection != 0)
    {
        HttpConnection::ReleaseHttpConnection(m_pConnection);
        m_pConnection = 0;
    }
}

qResult JobHttp::Initialize(unsigned int uiCallId, const String& rUrl, HttpConnection::Method method,
                            HttpEventListener* pListener, unsigned int uiBufferSize)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    m_uiCallId = uiCallId;
    m_url = rUrl;
    m_method = method;
    m_pListener = pListener;
    m_uiBufferSize = uiBufferSize;
    SetStep(Step(static_cast<Step::Function>(&JobHttp::StepHttp), L""));
    return qResult(true);
}

qResult JobHttp::PrepareHttp()
{
    m_pConnection = HttpConnection::GetHttpConnection();

    char* pUrl;
    m_url.CreateCopy(&pUrl);
    qResult result = m_pConnection->Initialize(pUrl, m_method);
    String::ReleaseCopy(pUrl);
    if (!result)
        return result;

    m_pListener->PrepareRequest(m_pConnection);
    if (Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId) == 0)
        return qResult(RESULT_CORE_OPERATION_ABORTED);

    result = m_pConnection->ConnectAsync();
    if (!result)
        return result;

    return qResult(RESULT_SUCCESS);
}

qResult JobHttp::ProceedReceivedBuffer()
{
    for (;;)
    {
        if (!m_receiveBuffer.empty())
        {
            bool bContinue = m_pListener->ProcessResponse(&m_receiveBuffer[0], m_receiveBuffer.size());
            if (Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId) == 0)
                return qResult(RESULT_CORE_OPERATION_ABORTED);

            if (!bContinue)
                return qResult(RESULT_TRANSPORT_DATA_REMAINING);

            m_receiveBuffer.clear();
        }

        unsigned int uiReadSize = 0;
        m_receiveBuffer.resize(m_uiBufferSize);
        qResult result = m_pConnection->Read(&m_receiveBuffer[0], &uiReadSize, m_receiveBuffer.size());
        if (!result)
            return result;

        m_receiveBuffer.resize(uiReadSize);
        if (m_receiveBuffer.empty())
        {
            if (m_pConnection->IsFinished())
                return qResult(RESULT_SUCCESS);

            return qResult(RESULT_TRANSPORT_DATA_REMAINING);
        }
    }
}

void JobHttp::CompleteJob(const qResult& rResult)
{
    CallContext* pCallContext = Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId);
    if (pCallContext != 0)
    {
        if (rResult)
            pCallContext->SignalSuccess(rResult);
        else
            pCallContext->SignalFailure(rResult);
    }

    if (m_pConnection != 0)
    {
        HttpConnection::ReleaseHttpConnection(m_pConnection);
        m_pConnection = 0;
    }

    SetToComplete();
}

void JobHttp::StepHttp()
{
    if (Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId) == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    qResult result = PrepareHttp();
    if (result)
    {
        SetStep(Step(static_cast<Step::Function>(&JobHttp::StepWaitingForHttp), L""));
        return;
    }

    CompleteJob(result);
}

void JobHttp::StepWaitingForHttp()
{
    if (Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId) == 0)
    {
        CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
        return;
    }

    unsigned int uiChunkCount = 0;
    while (m_pConnection->IsUseChunkedPost() && !m_pConnection->IsCompleteChunkedPost()
           && uiChunkCount++ <= MAX_POST_CHUNKS_PER_STEP)
    {
        unsigned int uiChunkSize = m_uiBufferSize;
        if (m_pConnection->GetWritableSize() < uiChunkSize)
            uiChunkSize = m_pConnection->GetWritableSize();

        if (uiChunkSize == 0)
            break;

        qVector<unsigned char> chunk(uiChunkSize);
        bool bLastChunk = false;
        unsigned int uiWritten = m_pListener->PostChunkedBuffer(m_pConnection, &chunk[0], chunk.size(), &bLastChunk);
        if (uiWritten > uiChunkSize)
        {
            CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
            return;
        }

        if (Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId) == 0)
        {
            CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
            return;
        }

        chunk.resize(uiWritten);
        if (!chunk.empty())
        {
            qResult result = m_pConnection->WritePostChunk(&chunk[0], chunk.size());
            if (!result)
            {
                CompleteJob(result);
                return;
            }
        }

        if (bLastChunk)
        {
            m_pConnection->CompleteChunkedPost();
            break;
        }

        if (uiWritten == 0)
            break;
    }

    if (!m_bHeaderReceived)
    {
        qResult result = m_pConnection->IsReceivingHeaderCompleted();
        if (!result)
        {
            if (result == static_cast<int>(RESULT_CORE_UNKNOWN))
            {
                SetToWaiting(m_iWaitTime);
                return;
            }

            if (result == static_cast<int>(RESULT_TRANSPORT_CONNECTION_FAILURE))
            {
                CompleteJob(qResult(RESULT_TRANSPORT_CONNECTION_FAILURE));
                return;
            }

            CompleteJob(result);
            return;
        }

        m_bHeaderReceived = true;
        int iStatusCode;
        if (!m_pConnection->GetStatusCode(&iStatusCode) || iStatusCode == 0)
        {
            CompleteJob(qResult(RESULT_TRANSPORT_SERVER_REQUEST_ERROR));
            return;
        }

        m_pListener->ProcessResponseHeader(m_pConnection, iStatusCode);
        if (Core::GetInstance()->GetCallContextRegister()->GetCallContext(m_uiCallId) == 0)
        {
            CompleteJob(qResult(RESULT_CORE_OPERATION_ABORTED));
            return;
        }

        m_pConnection->NoMoreResponseHeader();
    }

    qResult result = ProceedReceivedBuffer();
    if (result == static_cast<int>(RESULT_TRANSPORT_DATA_REMAINING))
        SetToWaiting(m_iWaitTime);
    else
        CompleteJob(result);
}

}
}
