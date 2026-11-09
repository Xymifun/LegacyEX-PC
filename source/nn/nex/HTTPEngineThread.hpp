#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HTTPENGINETHREAD_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HTTPENGINETHREAD_HPP

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/HTTPEngineCurlBase.hpp"
#include "nn/nex/ObjectThread.hpp"

namespace nn
{
namespace nex
{

class HTTPEngineThread : public HTTPEngineCurlBase
{
public:
    explicit HTTPEngineThread(HTTPEngineConfiguration* pConfiguration);
    virtual ~HTTPEngineThread();

    virtual bool ScheduleRequest(CallContext& rCallContext, unsigned int uiRequestID, HTTPResponse& rResponse,
                                 HTTPClientImpl* pClient);
    virtual void CancelRequest(unsigned int uiRequestID);
    virtual unsigned int RequestCompleted(void* pHandle);
    virtual void Work(void* pParam);

private:
    void EnterActiveState();
    void EnterIdleState();
    bool AllRequestsCompleted();

    CriticalSection m_threadLock;
    ObjectThread<HTTPEngineThread, void*>* m_pThread;
    unsigned int m_uiSleepPeriod;
    unsigned int m_uiIdleSleepPeriod;
    bool m_bStopping;
    bool m_bIdle;
};

}
}

#endif
