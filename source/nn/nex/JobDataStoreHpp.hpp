#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBDATASTOREHPP_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBDATASTOREHPP_HPP

#include "nn/nex/Buffer.hpp"
#include "nn/nex/CallContext.hpp"
#include "nn/nex/DataStoreKeyValue.hpp"
#include "nn/nex/HttpClient.hpp"
#include "nn/nex/HttpConnection.hpp"
#include "nn/nex/HttpEventListener.hpp"
#include "nn/nex/RetrieveGameAuthTokenClient.hpp"
#include "nn/nex/ServiceHttpClient.hpp"
#include "nn/nex/StepSequenceJob.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qResult.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class ClientProtocol;
class DataStorePostHttpEventListener;

class JobDataStoreHpp : public StepSequenceJob, public HttpEventListener
{
public:
    JobDataStoreHpp();
    virtual ~JobDataStoreHpp();

    virtual qResult Initialize(unsigned int uiCallId, ClientProtocol* pProtocol, const Buffer& rRequest,
                               unsigned int uiThreadPriority, unsigned int uiPrincipalId, const String& rPassword,
                               const String& rAccessKey, const String& rUrl, int iTimeout,
                               ServiceHttpClient::PriorityType priorityType);
    virtual void PrepareRequest(HttpConnection* pConnection);
    virtual void ProcessResponseHeader(HttpConnection* pConnection, int iStatusCode);
    virtual bool ProcessResponse(const unsigned char* pData, unsigned int uiSize);
    virtual unsigned int PostChunkedBuffer(HttpConnection* pConnection, unsigned char* pBuffer, unsigned int uiSize,
                                           bool* pbLastChunk);

    void CompleteJob(const qResult& rResult);
    void PrepareRequestHeaders();
    void StepRetrieveGameAuthToken();
    void StepWaitingRetrieveGameAuthToken();
    void ResetHttpResult();
    void StepHttp();
    bool IsHttpStatusCodeSuccess();
    qResult ProcessFinalResponse(const qVector<unsigned char>& rResponse);
    void StepWaitingForHttp();

private:
    enum Result
    {
        RESULT_SUCCESS = 0x10001,
        RESULT_CORE_OPERATION_ABORTED = static_cast<int>(0x80010004u),
        RESULT_CORE_CALL_INITIATION_FAILURE = static_cast<int>(0x8001000Du),
        RESULT_TRANSPORT_UNKNOWN = static_cast<int>(0x80050001u),
        RESULT_TRANSPORT_SERVER_REQUEST_ERROR = static_cast<int>(0x8005000Bu),
        RESULT_TRANSPORT_TEMPORARY_SERVER_ERROR = static_cast<int>(0x80050019u),
        RESULT_TRANSPORT_PERMANENT_SERVER_ERROR = static_cast<int>(0x8005001Au),
        RESULT_TRANSPORT_SERVICE_UNAVAILABLE = static_cast<int>(0x8005001Bu)
    };

    enum HttpStatusCode
    {
        HTTP_STATUS_TEMPORARY_REDIRECT = 307,
        HTTP_STATUS_BAD_REQUEST = 400,
        HTTP_STATUS_FORBIDDEN = 403,
        HTTP_STATUS_REQUEST_ENTITY_TOO_LARGE = 413,
        HTTP_STATUS_INTERNAL_SERVER_ERROR = 500,
        HTTP_STATUS_SERVICE_UNAVAILABLE = 503
    };

    enum
    {
        INFINITE_TIMEOUT = 0x7FFFFFFF,
        HTTP_BUFFER_SIZE = 16384,
        MAX_RESPONSE_SIZE = 10240,
        MIN_RESPONSE_SIZE = 5,
        MAX_REDIRECT_COUNT = 4,
        SIGNATURE_KEY_MIN_LENGTH = 16,
        APPEND_AT_END = 0xFFFFFFFFu
    };

    void AddRequestHeader(const String& rKey, const String& rValue);

    unsigned int m_uiCallId;
    String m_url;
    HttpConnection::Method m_method;
    Buffer m_request;
    ClientProtocol* m_pProtocol;
    CallContext m_callContext;
    HttpClient m_httpClient;
    unsigned int m_uiHttpRequestCount;
    int m_iStatusCode;
    qVector<unsigned char> m_response;
    bool m_bResponseTooLarge;
    String m_location;
    unsigned int m_uiThreadPriority;
    unsigned int m_uiPrincipalId;
    String m_password;
    String m_accessKey;
    int m_iTimeout;
    RetrieveGameAuthTokenClient m_retrieveGameAuthTokenClient;
    String m_token;
    String m_key;
    DataStorePostHttpEventListener* m_pListener;
    qVector<DataStoreKeyValue> m_headers;
    qVector<DataStoreKeyValue> m_formFields;
    qVector<unsigned char> m_rootCa;
    ServiceHttpClient::PriorityType m_priorityType;
};

}
}

#endif
