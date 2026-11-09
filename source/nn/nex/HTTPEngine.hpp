#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HTTPENGINE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HTTPENGINE_HPP

#include <cstdarg>

#include "nn/nex/HTTPEngineConfiguration.hpp"
#include "nn/nex/HTTPModule.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class CallContext;
class HTTPClientImpl;
class HTTPClientRequestData;
class HTTPData;
class HTTPReader;
class HTTPResponse;
class HTTPURL;
class HTTPWriter;

class HTTPEngine : public RootObject
{
public:
    enum TransferProgress
    {
        TRANSFER_NOT_STARTED = 0,
        TRANSFER_STARTED = 1,
        TRANSFER_REQUEST_NOT_FOUND = 2
    };

    HTTPEngine();
    virtual ~HTTPEngine();

    virtual void Initialize() = 0;
    virtual unsigned int CreateGETRequest(const HTTPURL& rURL, const qVector<String>& rHeaders,
                                          HTTPWriter& rWriter) = 0;
    virtual unsigned int CreatePOSTRequest(const HTTPURL& rURL, const qVector<String>& rHeaders, HTTPData& rData,
                                           HTTPWriter& rWriter) = 0;
    virtual unsigned int CreatePUTRequest(const HTTPURL& rURL, const qVector<String>& rHeaders,
                                          HTTPReader& rReader) = 0;
    virtual bool ScheduleRequest(CallContext& rCallContext, unsigned int uiRequestID, HTTPResponse& rResponse,
                                 HTTPClientImpl* pClient) = 0;
    virtual void DestroyRequest(unsigned int uiRequestID) = 0;
    virtual void CancelRequest(unsigned int uiRequestID) = 0;
    virtual TransferProgress GetDownloadProgress(CallContext* pCallContext, unsigned long long& rCurrent,
                                                 unsigned long long& rTotal) = 0;
    virtual TransferProgress GetUploadProgress(CallContext* pCallContext, unsigned long long& rCurrent,
                                               unsigned long long& rTotal) = 0;
    virtual bool SetOption(HTTPModule::ConfigurationOptions option, va_list arguments);

    void TraceOptions(unsigned int uiFilter);

protected:
    qVector<HTTPClientRequestData*> m_requests;
    HTTPEngineConfiguration* m_pConfiguration;
};

}
}

#endif
