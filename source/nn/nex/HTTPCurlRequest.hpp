#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HTTPCURLREQUEST_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HTTPCURLREQUEST_HPP

#include "nn/nex/HTTPProxySettings.hpp"
#include "nn/nex/HTTPURL.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class HTTPCurlRequest : public RootObject
{
public:
    enum HTTPMethod
    {
        HTTP_METHOD_GET = 0,
        HTTP_METHOD_POST = 1,
        HTTP_METHOD_PUT = 2
    };

    typedef unsigned int (*TransferCallback)(char* pBuffer, unsigned int uiSize, unsigned int uiCount, void* pUserData);

    HTTPCurlRequest();
    virtual ~HTTPCurlRequest();

    void ClearError();
    void Clear();
    bool SetHeaderWriter(TransferCallback pCallback, void* pUserData);
    bool SetMethod(HTTPMethod method);
    bool SetOption(unsigned int uiOption, const String& rValue);
    bool SetUrl(const HTTPURL& rURL);
    bool Reset(void* pShare, HTTPMethod method, const HTTPURL& rURL);
    bool SetWriter(TransferCallback pCallback, void* pUserData);
    bool SetReader(TransferCallback pCallback, void* pUserData, unsigned long long uiLength);
    bool AllowRedirection(bool bAllow);
    bool SetHeaders(const qVector<String>& rHeaders);
    bool SetProxy(const HTTPProxySettings& rProxy);
    bool SetTransferBufferSize(unsigned int uiSize);
    unsigned int GetResponseCode();
    const qVector<String>& GetHeaders() const;
    void Trace(unsigned int uiFilter) const;
    void* GetHandle();
    void GetLastError(String* pError) const;

    template <typename T>
    bool SetOption(unsigned int uiOption, const T& rValue);

    template <typename T>
    bool GetInfo(unsigned int uiInfo, T& rValue);

private:
    void* m_pHandle;
    unsigned int m_uiError;
    void* m_pHeaderList;
    unsigned int m_unresolved12;
    qVector<String> m_headers;
    bool m_bUnresolved32;
};

}
}

#endif
