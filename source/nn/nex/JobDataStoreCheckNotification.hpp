#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBDATASTORECHECKNOTIFICATION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBDATASTORECHECKNOTIFICATION_HPP

#include <stdint.h>

#include "nn/nex/DataStoreReqGetNotificationUrlInfo.hpp"
#include "nn/nex/HttpEventListener.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/StepSequenceJob.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qResult.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class DataStoreFileServerClient;
class DataStoreLogicServerClientInterface;
class HttpConnection;

class JobDataStoreCheckNotification : public StepSequenceJob, public HttpEventListener
{
public:
    JobDataStoreCheckNotification();
    virtual ~JobDataStoreCheckNotification();

    virtual qResult Initialize(unsigned int uiCallId, DataStoreLogicServerClientInterface* pClient,
                               unsigned int uiThreadPriority, int iTimeout, const String& rNotificationUrl,
                               String* pNextNotificationUrl, uint64_t uiLastDataId, uint64_t* pLastDataId,
                               unsigned int uiSequence, unsigned int* pSequence, unsigned int uiKey,
                               bool* pbUrlRequestFailed, bool bForceUrlRequest);

    virtual void PrepareRequest(HttpConnection* pConnection);
    virtual void ProcessResponseHeader(HttpConnection* pConnection, int iStatusCode);
    virtual bool ProcessResponse(const unsigned char* pData, unsigned int uiSize);

    void CompleteJob(const qResult& rResult);
    void StepLogicServerGetNotificationUrl();
    void StepWaitingLogicServerGetNotificationUrl();
    void StepFileServerCheckNotificationFile();
    void StepWaitingFileServerCheckNotificationFile();

private:
    enum Result
    {
        RESULT_SUCCESS = 0x10001,
        RESULT_CORE_OPERATION_ABORTED = static_cast<int>(0x80010004u),
        RESULT_CORE_CALL_FAILED = static_cast<int>(0x8001000Du)
    };

    enum
    {
        MAX_RESPONSE_SIZE = 0x80,
        RESPONSE_BUFFER_SIZE = 0x4000,
        NUMBER_BASE = 10
    };

    unsigned int m_uiCallId;
    DataStoreLogicServerClientInterface* m_pClient;
    DataStoreFileServerClient* m_pFileServerClient;
    String* m_pNextNotificationUrl;
    String m_notificationUrl;
    unsigned int m_uiThreadPriority;
    int m_iTimeout;
    ProtocolCallContext m_context;
    uint64_t* m_pLastDataId;
    uint64_t m_uiLastDataId;
    unsigned int* m_pSequence;
    unsigned int m_uiSequence;
    unsigned int m_uiKey;
    DataStoreReqGetNotificationUrlInfo m_urlInfo;
    qVector<unsigned char> m_response;
    int m_iStatusCode;
    bool m_bResponseOverflow;
    bool m_bForceUrlRequest;
    bool* m_pbUrlRequestFailed;
    bool m_bInvalidNotification;
};

}
}

#endif
