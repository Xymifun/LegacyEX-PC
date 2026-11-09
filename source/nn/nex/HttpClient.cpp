#include "nn/nex/HttpClient.hpp"

#include "nn/nex/CallContext.hpp"
#include "nn/nex/JobHttp.hpp"
#include "nn/nex/JobHttpBuffered.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

namespace
{

enum StartResult
{
    START_CALL_FAILED_RESULT = static_cast<int>(0x8001000Du),
    START_SUCCEEDED_RESULT = 0x10001
};

}

HttpClient::HttpClient()
{
}

HttpClient::~HttpClient()
{
}

qResult HttpClient::Start(CallContext* pCallContext, const String& rUrl, HttpConnection::Method method,
                          HttpEventListener* pListener, unsigned int uiBufferSize)
{
    if (!pCallContext->InitiateCall())
        return qResult(START_CALL_FAILED_RESULT);

    JobHttp* pJob = new JobHttp();
    if (!pJob->Initialize(pCallContext->GetCallID(), rUrl, method, pListener, uiBufferSize))
    {
        pJob->ReleaseRef();
        return qResult(START_CALL_FAILED_RESULT);
    }

    Scheduler::GetInstance()->Queue(pJob, false);
    return qResult(START_SUCCEEDED_RESULT);
}

qResult HttpClient::Start(CallContext* pCallContext, const String& rUrl, HttpConnection::Method method,
                          unsigned int uiMaxSize, qVector<unsigned char>* pBuffer, int* pStatusCode)
{
    if (!pCallContext->InitiateCall())
        return qResult(START_CALL_FAILED_RESULT);

    JobHttpBuffered* pJob = new JobHttpBuffered();
    if (!pJob->Initialize(pCallContext->GetCallID(), rUrl, method, uiMaxSize, pBuffer, pStatusCode))
    {
        pJob->ReleaseRef();
        return qResult(START_CALL_FAILED_RESULT);
    }

    Scheduler::GetInstance()->Queue(pJob, false);
    return qResult(START_SUCCEEDED_RESULT);
}

}
}
