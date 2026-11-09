#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HTTPCLIENTREQUESTDATA_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HTTPCLIENTREQUESTDATA_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class HTTPClientImpl;
class HTTPResponse;

class HTTPClientRequestData : public RootObject
{
public:
    HTTPClientRequestData(unsigned int uiCallID, HTTPClientImpl* pClient, unsigned int uiRequestID,
                          HTTPResponse* pResponse)
        : m_uiCallID(uiCallID),
          m_pClient(pClient),
          m_uiRequestID(uiRequestID),
          m_pResponse(pResponse),
          m_bCompleted(false)
    {
    }

    unsigned int GetCallID() const { return m_uiCallID; }

    HTTPClientImpl* GetClient() const { return m_pClient; }

    const unsigned int& GetRequestID() const { return m_uiRequestID; }

    HTTPResponse* GetResponse() const { return m_pResponse; }

    bool IsCompleted() const { return m_bCompleted; }

    void SetCompleted() { m_bCompleted = true; }

private:
    unsigned int m_uiCallID;
    HTTPClientImpl* m_pClient;
    unsigned int m_uiRequestID;
    HTTPResponse* m_pResponse;
    bool m_bCompleted;
};

}
}

#endif
