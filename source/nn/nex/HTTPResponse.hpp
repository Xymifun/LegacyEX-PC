#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HTTPRESPONSE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HTTPRESPONSE_HPP

#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class HTTPResponse : public RootObject
{
public:
    HTTPResponse();
    ~HTTPResponse();

    void OnResponseReceived(unsigned int uiStatusCode, const qVector<String>& rHeaders);

    unsigned int GetStatusCode() const;
    const qVector<String>& GetHeaders() const;

private:
    unsigned int m_uiStatusCode;
    qVector<String> m_headers;
};

}
}

#endif
