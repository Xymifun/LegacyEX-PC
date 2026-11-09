#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HTTPMODULEIMPL_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HTTPMODULEIMPL_HPP

#include "nn/nex/HTTPEngineFactory.hpp"
#include "nn/nex/HTTPProxySettings.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class HTTPModuleImpl : public RootObject
{
public:
    HTTPModuleImpl();
    ~HTTPModuleImpl();

    HTTPEngineFactory* GetEngineFactory();

    void SetProxy(const String& rName, unsigned int uiPort, const String& rUsername, const String& rPassword);
    void ResetProxySettings();
    void SetProxyName(const String& rName);
    const String& GetProxyName();
    void SetProxyPort(unsigned int uiPort);
    unsigned int GetProxyPort();
    void SetProxyUsername(const String& rUsername);
    const String& GetProxyUsername();
    void SetProxyPassword(const String& rPassword);
    const String& GetProxyPassword();

private:
    HTTPProxySettings m_proxySettings;
    HTTPEngineFactory m_engineFactory;
};

}
}

#endif
