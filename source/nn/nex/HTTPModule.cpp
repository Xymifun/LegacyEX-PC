#include "nn/nex/HTTPModule.hpp"

#include <cstdarg>

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/HTTPModuleImpl.hpp"

namespace nn
{
namespace nex
{

HTTPModule* HTTPModule::s_pInstance = 0;
CriticalSection HTTPModule::s_criticalSection;

HTTPModule::HTTPModule() : m_pImpl(new HTTPModuleImpl)
{
}

HTTPModule::~HTTPModule()
{
    delete m_pImpl;
}

HTTPModule* HTTPModule::AcquireInstance()
{
    ScopedCS lock(&s_criticalSection);
    if (s_pInstance == 0)
    {
        s_pInstance = new HTTPModule;
    }
    else
    {
        s_pInstance->AddRef();
    }
    return s_pInstance;
}

void HTTPModule::ReleaseInstance()
{
    ScopedCS lock(&s_criticalSection);
    if (s_pInstance->GetRefCount() == 1)
    {
        s_pInstance->ReleaseRef();
        s_pInstance = 0;
    }
    else
    {
        s_pInstance->ReleaseRef();
    }
}

HTTPModule* HTTPModule::GetInstance()
{
    ScopedCS lock(&s_criticalSection);
    return s_pInstance;
}

bool HTTPModule::SetEngineOpt(unsigned int uiEngineHandle, ConfigurationOptions option, ...)
{
    va_list arguments;
    va_start(arguments, option);
    bool bResult = true;
    HTTPEngine* pEngine = m_pImpl->GetEngineFactory()->GetHTTPEngine(uiEngineHandle);
    if (pEngine != 0)
    {
        bResult = pEngine->SetOption(option, arguments);
    }
    va_end(arguments);
    return bResult;
}

const String& HTTPModule::GetProxyName()
{
    return m_pImpl->GetProxyName();
}

unsigned int HTTPModule::GetProxyPort()
{
    return m_pImpl->GetProxyPort();
}

const String& HTTPModule::GetProxyUsername()
{
    return m_pImpl->GetProxyUsername();
}

const String& HTTPModule::GetProxyPassword()
{
    return m_pImpl->GetProxyPassword();
}

}
}
