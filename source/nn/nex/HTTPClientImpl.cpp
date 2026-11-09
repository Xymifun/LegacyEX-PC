#include "nn/nex/HTTPClientImpl.hpp"

#include "nn/nex/HTTPClientRequestData.hpp"

namespace nn
{
namespace nex
{

bool HTTPClientImpl::ScheduleRequest(CallContext& rCallContext, unsigned int uiRequestID, HTTPResponse& rResponse)
{
    return m_pEngine->ScheduleRequest(rCallContext, uiRequestID, rResponse, this);
}

bool HTTPClientImpl::GET(CallContext& rCallContext, const HTTPURL& rURL, HTTPResponse& rResponse, HTTPWriter& rWriter,
                         const qVector<String>& rHeaders)
{
    unsigned int uiRequestID = m_pEngine->CreateGETRequest(rURL, rHeaders, rWriter);
    bool bScheduled = ScheduleRequest(rCallContext, uiRequestID, rResponse);
    if (!bScheduled)
    {
        m_pEngine->DestroyRequest(uiRequestID);
    }
    return bScheduled;
}

bool HTTPClientImpl::POST(CallContext& rCallContext, const HTTPURL& rURL, HTTPData& rData, HTTPResponse& rResponse,
                          HTTPWriter& rWriter, const qVector<String>& rHeaders)
{
    unsigned int uiRequestID = m_pEngine->CreatePOSTRequest(rURL, rHeaders, rData, rWriter);
    bool bScheduled = ScheduleRequest(rCallContext, uiRequestID, rResponse);
    if (!bScheduled)
    {
        m_pEngine->DestroyRequest(uiRequestID);
    }
    return bScheduled;
}

bool HTTPClientImpl::PUT(CallContext& rCallContext, const HTTPURL& rURL, HTTPResponse& rResponse, HTTPReader& rReader,
                         const qVector<String>& rHeaders)
{
    unsigned int uiRequestID = m_pEngine->CreatePUTRequest(rURL, rHeaders, rReader);
    bool bScheduled = ScheduleRequest(rCallContext, uiRequestID, rResponse);
    if (!bScheduled)
    {
        m_pEngine->DestroyRequest(uiRequestID);
    }
    return bScheduled;
}

void HTTPClientImpl::GetDownloadProgress(CallContext& rCallContext, unsigned long long& rCurrent,
                                         unsigned long long& rTotal)
{
    m_pEngine->GetDownloadProgress(&rCallContext, rCurrent, rTotal);
}

void HTTPClientImpl::GetUploadProgress(CallContext& rCallContext, unsigned long long& rCurrent,
                                       unsigned long long& rTotal)
{
    m_pEngine->GetUploadProgress(&rCallContext, rCurrent, rTotal);
}

void HTTPClientImpl::OnCompletionCallback(CallContext* pCallContext, const UserContext* pUserContext)
{
    HTTPClientRequestData* pRequest = static_cast<HTTPClientRequestData*>(pUserContext->GetPointer());
    if (pCallContext->GetState() == CallContext::CallCanceled)
    {
        pRequest->GetClient()->m_pEngine->CancelRequest(pRequest->GetRequestID());
    }
    pRequest->GetClient()->m_pEngine->DestroyRequest(pRequest->GetRequestID());
}

}
}
