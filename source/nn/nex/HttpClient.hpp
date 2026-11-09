#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HTTPCLIENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HTTPCLIENT_HPP

#include "nn/nex/HttpConnection.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/qResult.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class CallContext;
class HttpEventListener;
class String;

class HttpClient : public RootObject
{
public:
    HttpClient();
    virtual ~HttpClient();

    qResult Start(CallContext* pCallContext, const String& rUrl, HttpConnection::Method method,
                  HttpEventListener* pListener, unsigned int uiBufferSize);
    qResult Start(CallContext* pCallContext, const String& rUrl, HttpConnection::Method method, unsigned int uiMaxSize,
                  qVector<unsigned char>* pBuffer, int* pStatusCode);
};

}
}

#endif
