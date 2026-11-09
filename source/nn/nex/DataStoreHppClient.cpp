#include "nn/nex/DataStoreHppClient.hpp"

#include "nn/nex/Buffer.hpp"
#include "nn/nex/CallContext.hpp"
#include "nn/nex/JobDataStoreHpp.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

DataStoreHppClient::DataStoreHppClient()
{
}

DataStoreHppClient::~DataStoreHppClient()
{
}

qResult DataStoreHppClient::Request(CallContext* pCallContext, ClientProtocol* pProtocol, const Buffer& rRequest,
                                    unsigned int uiThreadPriority, unsigned int uiPrincipalId, const String& rPassword,
                                    const String& rAccessKey, const String& rUrl, int iTimeout,
                                    ServiceHttpClient::PriorityType priorityType)
{
    JobDataStoreHpp* pJob = new JobDataStoreHpp();
    qResult result = pJob->Initialize(pCallContext->GetCallID(), pProtocol, rRequest, uiThreadPriority, uiPrincipalId,
                                      rPassword, rAccessKey, rUrl, iTimeout, priorityType);
    if (!result)
    {
        pJob->ReleaseRef();
        return result;
    }

    Scheduler::GetInstance()->Queue(pJob, false);
    return qResult(REQUEST_SUCCEEDED_RESULT);
}

}
}
