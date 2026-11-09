#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBDATASTOREFILESERVER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBDATASTOREFILESERVER_HPP

#include "nn/nex/CallContext.hpp"
#include "nn/nex/HttpClient.hpp"
#include "nn/nex/HttpConnection.hpp"
#include "nn/nex/HttpEventListener.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/StepSequenceJob.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qResult.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class JobDataStoreFileServer : public StepSequenceJob, public HttpEventListener
{
public:
    JobDataStoreFileServer();
    virtual ~JobDataStoreFileServer();

    virtual qResult Initialize(unsigned int uiCallId, const String& rUrl, HttpConnection::Method method,
                               HttpEventListener* pListener, unsigned int uiBufferSize);

    virtual void PrepareRequest(HttpConnection* pConnection);
    virtual unsigned int PostChunkedBuffer(HttpConnection* pConnection, unsigned char* pBuffer, unsigned int uiSize,
                                           bool* pbLastChunk);
    virtual void ProcessResponseHeader(HttpConnection* pConnection, int iStatusCode);
    virtual bool ProcessResponse(const unsigned char* pData, unsigned int uiSize);

    void CompleteJob(const qResult& rResult);
    bool IsHttpStatusCodeSuccess();
    void ResetHttpResult();
    void StepHttp();
    void StepWaitingForHttp();

private:
    enum Result
    {
        RESULT_SUCCESS = 0x10001,
        RESULT_CORE_OPERATION_ABORTED = static_cast<int>(0x80010004u),
        RESULT_CORE_CALL_FAILED = static_cast<int>(0x8001000Du),
        RESULT_TRANSPORT_TEMPORARY_SERVER_ERROR = static_cast<int>(0x80050019u),
        RESULT_TRANSPORT_PERMANENT_SERVER_ERROR = static_cast<int>(0x8005001Au),
        RESULT_DATASTORE_NOT_FOUND = static_cast<int>(0x80690004u)
    };

    enum HttpStatusCode
    {
        HTTP_STATUS_SUCCESS_MIN = 1,
        HTTP_STATUS_SUCCESS_MAX = 299,
        HTTP_STATUS_TEMPORARY_REDIRECT = 307,
        HTTP_STATUS_NOT_FOUND = 404,
        HTTP_STATUS_CONFLICT = 409,
        HTTP_STATUS_INTERNAL_SERVER_ERROR = 500
    };

    enum
    {
        MAX_ERROR_RESPONSE_SIZE = 0x2800,
        MAX_REDIRECT_ATTEMPTS = 4,
        MAX_RETRY_ATTEMPTS = 2
    };

    unsigned int m_uiCallId;
    String m_url;
    HttpConnection::Method m_method;
    HttpEventListener* m_pListener;
    CallContext m_httpContext;
    HttpClient m_httpClient;
    unsigned int m_uiAttemptCount;
    int m_iStatusCode;
    qVector<unsigned char> m_errorResponse;
    bool m_bErrorResponseOverflow;
    String m_location;
    unsigned int m_uiBufferSize;
};

class DataStoreFileServerClient : public RootObject
{
public:
    DataStoreFileServerClient() {}

    virtual ~DataStoreFileServerClient() {}

    qResult PostObject(CallContext* pContext, const String& rUrl, HttpEventListener* pListener,
                       unsigned int uiBufferSize)
    {
        return ApiImpl(pContext, rUrl, pListener, HttpConnection::HTTP_POST, uiBufferSize);
    }

    qResult GetObject(CallContext* pContext, const String& rUrl, HttpEventListener* pListener,
                      unsigned int uiBufferSize)
    {
        return ApiImpl(pContext, rUrl, pListener, HttpConnection::HTTP_GET, uiBufferSize);
    }

private:
    enum Result
    {
        RESULT_SUCCESS = 0x10001,
        RESULT_CORE_CALL_FAILED = static_cast<int>(0x8001000Du)
    };

    qResult ApiImpl(CallContext* pContext, const String& rUrl, HttpEventListener* pListener,
                    HttpConnection::Method method, unsigned int uiBufferSize)
    {
        if (!pContext->InitiateCall())
            return qResult(RESULT_CORE_CALL_FAILED);

        JobDataStoreFileServer* pJob = new JobDataStoreFileServer();
        qResult result = pJob->Initialize(pContext->GetCallID(), rUrl, method, pListener, uiBufferSize);
        if (!result)
        {
            pJob->ReleaseRef();
            return result;
        }

        Scheduler::GetInstance()->Queue(pJob, false);
        return qResult(RESULT_SUCCESS);
    }
};

}
}

#endif
