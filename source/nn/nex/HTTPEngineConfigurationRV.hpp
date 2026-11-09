#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HTTPENGINECONFIGURATIONRV_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HTTPENGINECONFIGURATIONRV_HPP

#include "nn/nex/HTTPEngineConfiguration.hpp"

namespace nn
{
namespace nex
{

class HTTPEngineConfigurationRV : public HTTPEngineConfiguration
{
public:
    HTTPEngineConfigurationRV();
    virtual ~HTTPEngineConfigurationRV() {}

    virtual bool SetOption(HTTPModule::ConfigurationOptions option, va_list arguments);
    virtual void TraceOptions(unsigned int uiFilter);

    void SetEngineJobPeriod(unsigned int uiPeriod);
    unsigned int GetEngineJobPeriod() const;

private:
    unsigned int m_engineJobPeriod;
};

}
}

#endif
