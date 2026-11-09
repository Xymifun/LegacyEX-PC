#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREHPPCLIENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREHPPCLIENT_HPP

#include "nn/nex/RootObject.hpp"
#include "nn/nex/ServiceHttpClient.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class Buffer;
class CallContext;
class ClientProtocol;
class String;

class DataStoreHppClient : public RootObject
{
public:
    DataStoreHppClient();
    virtual ~DataStoreHppClient();

    qResult Request(CallContext* pCallContext, ClientProtocol* pProtocol, const Buffer& rRequest,
                    unsigned int uiThreadPriority, unsigned int uiPrincipalId, const String& rPassword,
                    const String& rAccessKey, const String& rUrl, int iTimeout,
                    ServiceHttpClient::PriorityType priorityType);

private:
    enum RequestResult
    {
        REQUEST_SUCCEEDED_RESULT = 0x10001
    };
};

}
}

#endif
