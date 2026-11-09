#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HTTPENGINERV_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HTTPENGINERV_HPP

#include "nn/nex/HTTPEngineConfiguration.hpp"
#include "nn/nex/HTTPEngineCurlBase.hpp"

namespace nn
{
namespace nex
{

class HTTPEngineRVJob;

class HTTPEngineRV : public HTTPEngineCurlBase
{
public:
    explicit HTTPEngineRV(HTTPEngineConfiguration* pConfiguration);
    virtual ~HTTPEngineRV();

    virtual bool ScheduleRequest(CallContext& rCallContext, unsigned int uiRequestID, HTTPResponse& rResponse,
                                 HTTPClientImpl* pClient);
    virtual void CancelRequest(unsigned int uiRequestID);
    virtual unsigned int RequestCompleted(void* pHandle);

private:
    HTTPEngineRVJob* m_pJob;
};

}
}

#endif
