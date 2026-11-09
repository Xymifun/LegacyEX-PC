#include "nn/nex/IORequestContext.hpp"

#include "nn/nex/Buffer.hpp"

namespace nn
{
namespace nex
{

IORequestContext::IORequestContext() : m_pResponse(0), m_pRequest(0), m_pCallback(0), m_userContext()
{
}

IORequestContext::~IORequestContext()
{
    if (m_pRequest != 0)
        m_pRequest->ReleaseRef();

    m_pResponse = m_pRequest = 0;
    m_pCallback = 0;
}

}
}
