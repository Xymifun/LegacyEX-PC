#include "nn/nex/HTTPEngineConfigurationRV.hpp"

namespace nn
{
namespace nex
{

HTTPEngineConfigurationRV::HTTPEngineConfigurationRV()
{
    m_engineJobPeriod = 100;
}

bool HTTPEngineConfigurationRV::SetOption(HTTPModule::ConfigurationOptions option, va_list arguments)
{
    bool bResult = true;
    switch (option)
    {
    case HTTPModule::ENGINE_JOB_PERIOD:
        m_engineJobPeriod = va_arg(arguments, unsigned int);
        break;
    case HTTPModule::ENGINE_TRANSFER_BUFFER_SIZE:
        m_engineTransferBufferSize = va_arg(arguments, unsigned int);
        break;
    default:
        bResult = false;
        break;
    }
    return bResult;
}

void HTTPEngineConfigurationRV::TraceOptions(unsigned int uiFilter)
{
    HTTPEngineConfiguration::TraceOptions(uiFilter);
}

unsigned int HTTPEngineConfigurationRV::GetEngineJobPeriod() const
{
    return m_engineJobPeriod;
}

}
}
