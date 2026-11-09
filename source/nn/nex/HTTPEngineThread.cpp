#include "nn/nex/HTTPEngineThread.hpp"

#include "nn/nex/HTTPClientRequestData.hpp"
#include "nn/nex/Platform.hpp"

namespace nn
{
namespace nex
{

void HTTPEngineThread::EnterActiveState()
{
    m_bIdle = false;
}

void HTTPEngineThread::CancelRequest(unsigned int uiRequestID)
{
    HTTPEngineCurlBase::CancelRequest(uiRequestID);
    if (AllRequestsCompleted())
    {
        EnterIdleState();
    }
}

bool HTTPEngineThread::AllRequestsCompleted()
{
    LockEngineRequest(true);
    for (unsigned int i = 0; i < m_requests.size(); ++i)
    {
        if (!m_requests[i]->IsCompleted())
        {
            LockEngineRequest(false);
            return false;
        }
    }
    LockEngineRequest(false);
    return true;
}

void HTTPEngineThread::EnterIdleState()
{
    m_bIdle = true;
}

unsigned int HTTPEngineThread::RequestCompleted(void* pHandle)
{
    unsigned int uiRequestID = HTTPEngineCurlBase::RequestCompleted(pHandle);
    if (AllRequestsCompleted())
    {
        EnterIdleState();
    }
    return uiRequestID;
}

void HTTPEngineThread::Work(void*)
{
    while (!m_bStopping)
    {
        while (m_bIdle)
        {
            Platform::Sleep(m_uiIdleSleepPeriod);
            if (m_bStopping)
            {
                return;
            }
        }
        PerformTransfer();
        Platform::Sleep(m_uiSleepPeriod);
    }
}

}
}
