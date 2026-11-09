#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HTTPCLIENTIMPL_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HTTPCLIENTIMPL_HPP

#include "nn/nex/CallContext.hpp"
#include "nn/nex/HTTPEngine.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/UserContext.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class HTTPData;
class HTTPReader;
class HTTPResponse;
class HTTPURL;
class HTTPWriter;

class HTTPClientImpl : public RootObject
{
public:
    explicit HTTPClientImpl(HTTPEngine* pEngine);
    ~HTTPClientImpl();

    bool GET(CallContext& rCallContext, const HTTPURL& rURL, HTTPResponse& rResponse, HTTPWriter& rWriter,
             const qVector<String>& rHeaders);
    bool POST(CallContext& rCallContext, const HTTPURL& rURL, HTTPData& rData, HTTPResponse& rResponse,
              HTTPWriter& rWriter, const qVector<String>& rHeaders);
    bool PUT(CallContext& rCallContext, const HTTPURL& rURL, HTTPResponse& rResponse, HTTPReader& rReader,
             const qVector<String>& rHeaders);

    void GetDownloadProgress(CallContext& rCallContext, unsigned long long& rCurrent, unsigned long long& rTotal);
    void GetUploadProgress(CallContext& rCallContext, unsigned long long& rCurrent, unsigned long long& rTotal);

    void Trace(unsigned int uiFilter) const;

    static void OnCompletionCallback(CallContext* pCallContext, const UserContext* pUserContext);

private:
    bool ScheduleRequest(CallContext& rCallContext, unsigned int uiRequestID, HTTPResponse& rResponse);

    HTTPEngine* m_pEngine;
};

}
}

#endif
