#include "nn/nex/RetrieveGameAuthTokenClient.hpp"

#include "nn/nex/CallContext.hpp"
#include "nn/nex/JobRetrieveGameAuthToken.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

RetrieveGameAuthTokenClient::RetrieveGameAuthTokenClient()
{
}

RetrieveGameAuthTokenClient::~RetrieveGameAuthTokenClient()
{
}

qResult RetrieveGameAuthTokenClient::RetrieveGameAuthToken(CallContext* pCallContext, unsigned int uiPrincipalId,
                                                           const String& rPassword, String* pToken, String* pKey)
{
    if (!pCallContext->InitiateCall())
        return qResult(RETRIEVE_CALL_FAILED_RESULT);

    JobRetrieveGameAuthToken* pJob = new JobRetrieveGameAuthToken();
    qResult result = pJob->Initialize(pCallContext->GetCallID(), uiPrincipalId, rPassword, pToken, pKey);
    if (!result)
    {
        pJob->ReleaseRef();
        return result;
    }

    Scheduler::GetInstance()->Queue(pJob, false);
    return qResult(RETRIEVE_CALL_SUCCEEDED_RESULT);
}

}
}
