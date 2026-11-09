#include "nn/nex/HTTPEngineFactory.hpp"

#include "nn/nex/HTTPEngineConfigurationRV.hpp"
#include "nn/nex/HTTPEngineConfigurationThread.hpp"
#include "nn/nex/HTTPEngineRV.hpp"

namespace nn
{
namespace nex
{

HTTPEngineFactory::HTTPEngineFactory()
{
    m_nextEngineHandle = 0;
    HTTPEngineConfiguration*& rRVConfiguration = m_configurations[HTTPModule::HTTP_ENGINE_RV];
    rRVConfiguration = new HTTPEngineConfigurationRV();
    HTTPEngineConfiguration*& rThreadConfiguration = m_configurations[HTTPModule::HTTP_ENGINE_THREAD];
    rThreadConfiguration = new HTTPEngineConfigurationThread();
}

HTTPEngineFactory::~HTTPEngineFactory()
{
    for (EngineMap::iterator it = m_engines.begin(); it != m_engines.end(); ++it)
    {
        delete it->second;
    }
    for (EngineHandleMap::iterator it = m_engineHandles.begin(); it != m_engineHandles.end(); ++it)
    {
        delete it->second;
    }
    for (ConfigurationMap::iterator it = m_configurations.begin(); it != m_configurations.end(); ++it)
    {
        delete it->second;
    }
}

unsigned int HTTPEngineFactory::AddEngine(HTTPModule::HttpEngineType type, HTTPEngine* pEngine)
{
    ScopedCS lock(&m_criticalSection);
    pEngine->Initialize();
    unsigned int uiEngineHandle = m_nextEngineHandle++;
    if (m_engineHandles.find(type) == m_engineHandles.end())
    {
        m_engineHandles[type] = new qList<unsigned int>;
    }
    m_engineHandles[type]->push_back(uiEngineHandle);
    m_engines[uiEngineHandle] = pEngine;
    return uiEngineHandle;
}

HTTPEngine* HTTPEngineFactory::GetHTTPEngine(unsigned int uiEngineHandle)
{
    ScopedCS lock(&m_criticalSection);
    if (m_engines.find(uiEngineHandle) == m_engines.end())
    {
        if (uiEngineHandle == 0)
        {
            if (m_engines.begin() != m_engines.end())
            {
                return m_engines.begin()->second;
            }
            HTTPEngine* pEngine = new HTTPEngineRV(m_configurations[HTTPModule::HTTP_ENGINE_RV]);
            unsigned int uiNewHandle = AddEngine(HTTPModule::HTTP_ENGINE_RV, pEngine);
            return m_engines[uiNewHandle];
        }
        return 0;
    }
    return m_engines[uiEngineHandle];
}

bool HTTPEngineFactory::SetGlobaEngineOption(HTTPModule::HttpEngineType type, HTTPModule::ConfigurationOptions option,
                                             va_list arguments)
{
    if (m_configurations.find(type) != m_configurations.end())
    {
        return m_configurations[type]->SetOption(option, arguments);
    }
    return false;
}

void HTTPEngineFactory::TraceGlobaEngineOptions(HTTPModule::HttpEngineType type, unsigned int uiFilter)
{
    m_configurations[type]->TraceOptions(uiFilter);
}

}
}
