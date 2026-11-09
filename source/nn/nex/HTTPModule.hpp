#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HTTPMODULE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HTTPMODULE_HPP

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/RefCountedObject.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class HTTPEngine;
class HTTPModuleImpl;

class HTTPModule : public RefCountedObject
{
public:
    enum ConfigurationOptions
    {
        ENGINE_JOB_PERIOD = 0,
        ENGINE_THREAD_AFFINITY = 1,
        ENGINE_TRANSFER_BUFFER_SIZE = 2,
        ENGINE_THREAD_SLEEP_PERIOD = 3
    };

    enum HttpEngineType
    {
        HTTP_ENGINE_RV = 100,
        HTTP_ENGINE_THREAD = 101
    };

    HTTPModule();
    virtual ~HTTPModule();

    static HTTPModule* AcquireInstance();
    static void ReleaseInstance();
    static HTTPModule* GetInstance();

    bool SetEngineOpt(unsigned int uiEngineHandle, ConfigurationOptions option, ...);
    void TraceEngineOpt(unsigned int uiEngineHandle, unsigned int uiFilter);
    bool SetGlobalEngineOpt(HttpEngineType type, ConfigurationOptions option, ...);
    void TraceGlobalEngineOpt(HttpEngineType type, unsigned int uiFilter);

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
    static HTTPModule* s_pInstance;
    static CriticalSection s_criticalSection;

    HTTPModuleImpl* m_pImpl;
};

}
}

#endif
