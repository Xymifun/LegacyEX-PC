#include "nn/nex/HTTPEngineRV.hpp"

#include "nn/nex/HTTPEngineConfigurationRV.hpp"
#include "nn/nex/HTTPEngineRVJob.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

HTTPEngineRV::HTTPEngineRV(HTTPEngineConfiguration* pConfiguration) : m_pJob(0)
{
    if (pConfiguration != 0)
    {
        m_pConfiguration = new HTTPEngineConfigurationRV(*static_cast<HTTPEngineConfigurationRV*>(pConfiguration));
    }
    else
    {
        m_pConfiguration = new HTTPEngineConfigurationRV;
    }
}

HTTPEngineRV::~HTTPEngineRV()
{
    if (m_pJob != 0)
    {
        m_pJob->SetToComplete();
        m_pJob = 0;
    }
    if (m_pConfiguration != 0)
    {
        delete m_pConfiguration;
        m_pConfiguration = 0;
    }
}

bool HTTPEngineRV::ScheduleRequest(CallContext& rCallContext, unsigned int uiRequestID, HTTPResponse& rResponse,
                                   HTTPClientImpl* pClient)
{
    bool bScheduled = HTTPEngineCurlBase::ScheduleRequest(rCallContext, uiRequestID, rResponse, pClient);
    LockEngineRequest(true);
    HTTPEngineConfigurationRV* pConfiguration = static_cast<HTTPEngineConfigurationRV*>(m_pConfiguration);
    if (m_requests.size() == 1)
    {
        m_pJob = new HTTPEngineRVJob(this);
        m_pJob->SetPeriod(pConfiguration->GetEngineJobPeriod());
        Scheduler::GetInstance()->Queue(m_pJob, false);
    }
    m_pJob->SetPeriod(pConfiguration->GetEngineJobPeriod());
    LockEngineRequest(false);
    return bScheduled;
}

void HTTPEngineRV::CancelRequest(unsigned int uiRequestID)
{
    HTTPEngineCurlBase::CancelRequest(uiRequestID);
    LockEngineRequest(true);
    if (m_requests.size() == 1)
    {
        m_pJob->SetToComplete();
        m_pJob = 0;
    }
    LockEngineRequest(false);
}

unsigned int HTTPEngineRV::RequestCompleted(void* pHandle)
{
    unsigned int uiCallID = HTTPEngineCurlBase::RequestCompleted(pHandle);
    LockEngineRequest(true);
    if (m_requests.size() == 1)
    {
        m_pJob->SetToComplete();
        m_pJob = 0;
    }
    LockEngineRequest(false);
    return uiCallID;
}

}
}
