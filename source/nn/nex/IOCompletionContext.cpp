#include "nn/nex/IOCompletionContext.hpp"

namespace nn
{
namespace nex
{

IOCompletionContext::IOCompletionContext() : m_unknown2C(0)
{
    Reset();
}

IOCompletionContext::~IOCompletionContext()
{
    Reset();
}

void IOCompletionContext::Reset()
{
    m_unknown0 = 0;
    m_unknown4 = 0;
    m_unknown8 = 0;
    m_unknownC = 0;
    m_uiState = 0;
    m_pSocket = 0;
    m_unknown2C = 0;
    m_uiOperation = 0;
    m_address.Init();
    m_unknown24 = 0;
}

void IOCompletionContext::Trace(unsigned long long uiFilter)
{
}

}
}
