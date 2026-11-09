#include "nn/nex/HTTPResponse.hpp"

namespace nn
{
namespace nex
{

void HTTPResponse::OnResponseReceived(unsigned int uiStatusCode, const qVector<String>& rHeaders)
{
    m_uiStatusCode = uiStatusCode;
    m_headers = rHeaders;
}

}
}
