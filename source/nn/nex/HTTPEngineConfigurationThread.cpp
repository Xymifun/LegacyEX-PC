#include "nn/nex/HTTPEngineConfigurationThread.hpp"

namespace nn
{
namespace nex
{

HTTPEngineConfigurationThread::HTTPEngineConfigurationThread()
{
    m_engineThreadAffinity = static_cast<unsigned int>(-1);
    m_engineThreadSleepPeriod = 0;
}

bool HTTPEngineConfigurationThread::SetOption(HTTPModule::ConfigurationOptions option, va_list arguments)
{
    bool bResult = true;
    switch (option)
    {
    case HTTPModule::ENGINE_THREAD_AFFINITY:
        m_engineThreadAffinity = va_arg(arguments, unsigned int);
        break;
    case HTTPModule::ENGINE_THREAD_SLEEP_PERIOD:
        m_engineThreadSleepPeriod = va_arg(arguments, unsigned int);
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

void HTTPEngineConfigurationThread::TraceOptions(unsigned int uiFilter)
{
    HTTPEngineConfiguration::TraceOptions(uiFilter);
}

unsigned int HTTPEngineConfigurationThread::GetEngineThreadSleepPeriod() const
{
    return m_engineThreadSleepPeriod;
}

unsigned int HTTPEngineConfigurationThread::GetEngineThreadAffinity() const
{
    return m_engineThreadAffinity;
}

}
}
