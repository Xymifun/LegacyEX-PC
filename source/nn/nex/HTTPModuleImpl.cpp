#include "nn/nex/HTTPModuleImpl.hpp"

namespace nn
{
namespace nex
{

HTTPModuleImpl::HTTPModuleImpl()
{
}

HTTPModuleImpl::~HTTPModuleImpl()
{
}

HTTPEngineFactory* HTTPModuleImpl::GetEngineFactory()
{
    return &m_engineFactory;
}

const String& HTTPModuleImpl::GetProxyName()
{
    return m_proxySettings.GetProxyName();
}

unsigned int HTTPModuleImpl::GetProxyPort()
{
    return m_proxySettings.GetProxyPort();
}

const String& HTTPModuleImpl::GetProxyUsername()
{
    return m_proxySettings.GetProxyName();
}

const String& HTTPModuleImpl::GetProxyPassword()
{
    return m_proxySettings.GetPassword();
}

}
}
