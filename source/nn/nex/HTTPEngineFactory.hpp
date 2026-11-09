#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HTTPENGINEFACTORY_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HTTPENGINEFACTORY_HPP

#include <cstdarg>
#include <map>

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/HTTPEngine.hpp"
#include "nn/nex/HTTPEngineConfiguration.hpp"
#include "nn/nex/HTTPModule.hpp"
#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class HTTPEngineFactory : public RootObject
{
public:
    HTTPEngineFactory();
    ~HTTPEngineFactory();

    unsigned int AddEngine(HTTPModule::HttpEngineType type, HTTPEngine* pEngine);
    HTTPEngine* GetHTTPEngine(unsigned int uiEngineHandle);
    bool SetGlobaEngineOption(HTTPModule::HttpEngineType type, HTTPModule::ConfigurationOptions option,
                              va_list arguments);
    void TraceGlobaEngineOptions(HTTPModule::HttpEngineType type, unsigned int uiFilter);

private:
    typedef std::map<unsigned int, HTTPEngine*, std::less<unsigned int>,
                     MemAllocator<std::pair<const unsigned int, HTTPEngine*> > >
        EngineMap;
    typedef std::map<HTTPModule::HttpEngineType, qList<unsigned int>*, std::less<HTTPModule::HttpEngineType>,
                     MemAllocator<std::pair<const HTTPModule::HttpEngineType, qList<unsigned int>*> > >
        EngineHandleMap;
    typedef std::map<HTTPModule::HttpEngineType, HTTPEngineConfiguration*, std::less<HTTPModule::HttpEngineType>,
                     MemAllocator<std::pair<const HTTPModule::HttpEngineType, HTTPEngineConfiguration*> > >
        ConfigurationMap;

    unsigned int m_unresolved0;
    EngineMap m_engines;
    EngineHandleMap m_engineHandles;
    ConfigurationMap m_configurations;
    unsigned int m_nextEngineHandle;
    CriticalSection m_criticalSection;
};

}
}

#endif
