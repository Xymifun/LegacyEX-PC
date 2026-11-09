#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBHTTP_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBHTTP_HPP

#include "nn/nex/HttpConnection.hpp"
#include "nn/nex/StepSequenceJob.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qResult.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class HttpEventListener;

class JobHttp : public StepSequenceJob
{
public:
    enum Result
    {
        RESULT_SUCCESS = 0x10001,
        RESULT_CORE_UNKNOWN = static_cast<int>(0x80010001u),
        RESULT_CORE_NOT_IMPLEMENTED = static_cast<int>(0x80010002u),
        RESULT_CORE_OPERATION_ABORTED = static_cast<int>(0x80010004u),
        RESULT_TRANSPORT_CONNECTION_FAILURE = static_cast<int>(0x80050002u),
        RESULT_TRANSPORT_SERVER_REQUEST_ERROR = static_cast<int>(0x8005000Bu),
        RESULT_TRANSPORT_DATA_REMAINING = static_cast<int>(0x80050016u)
    };

    JobHttp();
    virtual ~JobHttp();

    virtual qResult Initialize(unsigned int uiCallId, const String& rUrl, HttpConnection::Method method,
                               HttpEventListener* pListener, unsigned int uiBufferSize);

    qResult PrepareHttp();
    qResult ProceedReceivedBuffer();
    void CompleteJob(const qResult& rResult);
    void StepHttp();
    void StepWaitingForHttp();

protected:
    enum
    {
        MAX_POST_CHUNKS_PER_STEP = 100,
        WAIT_TIME = 5,
        WAIT_TIME_DISPATCHER_THREAD = 10
    };

    unsigned int m_uiCallId;
    HttpConnection* m_pConnection;
    String m_url;
    HttpConnection::Method m_method;
    qVector<unsigned char> m_receiveBuffer;
    bool m_bHeaderReceived;
    HttpEventListener* m_pListener;
    unsigned int m_uiBufferSize;
    int m_iWaitTime;
};

}
}

#endif
