#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HTTPENGINECONFIGURATION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HTTPENGINECONFIGURATION_HPP

#include <cstdarg>

#include "nn/nex/HTTPModule.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class HTTPEngineConfiguration : public RootObject
{
public:
    HTTPEngineConfiguration();
    virtual ~HTTPEngineConfiguration() {}

    virtual bool SetOption(HTTPModule::ConfigurationOptions option, va_list arguments) = 0;
    virtual void TraceOptions(unsigned int uiFilter);

    unsigned int GetEngineTransferBufferSize() const;

protected:
    unsigned int m_engineTransferBufferSize;
};

}
}

#endif
