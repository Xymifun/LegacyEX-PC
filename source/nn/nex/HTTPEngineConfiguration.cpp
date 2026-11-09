#include "nn/nex/HTTPEngineConfiguration.hpp"

namespace nn
{
namespace nex
{

HTTPEngineConfiguration::HTTPEngineConfiguration()
{
    m_engineTransferBufferSize = 0;
}

void HTTPEngineConfiguration::TraceOptions(unsigned int)
{
}

unsigned int HTTPEngineConfiguration::GetEngineTransferBufferSize() const
{
    return m_engineTransferBufferSize;
}

}
}
