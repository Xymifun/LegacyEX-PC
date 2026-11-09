#include "nn/nex/HTTPEngine.hpp"

namespace nn
{
namespace nex
{

HTTPEngine::HTTPEngine()
{
}

HTTPEngine::~HTTPEngine()
{
}

bool HTTPEngine::SetOption(HTTPModule::ConfigurationOptions option, va_list arguments)
{
    return m_pConfiguration->SetOption(option, arguments);
}

void HTTPEngine::TraceOptions(unsigned int uiFilter)
{
    m_pConfiguration->TraceOptions(uiFilter);
}

}
}
