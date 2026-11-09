#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HTTPENGINECONFIGURATIONTHREAD_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HTTPENGINECONFIGURATIONTHREAD_HPP

#include "nn/nex/HTTPEngineConfiguration.hpp"

namespace nn
{
namespace nex
{

class HTTPEngineConfigurationThread : public HTTPEngineConfiguration
{
public:
    HTTPEngineConfigurationThread();
    virtual ~HTTPEngineConfigurationThread() {}

    virtual bool SetOption(HTTPModule::ConfigurationOptions option, va_list arguments);
    virtual void TraceOptions(unsigned int uiFilter);

    unsigned int GetEngineThreadSleepPeriod() const;
    unsigned int GetEngineThreadAffinity() const;

private:
    unsigned int m_engineThreadAffinity;
    unsigned int m_engineThreadSleepPeriod;
};

}
}

#endif
