#include "nn/nex/Plugin.hpp"

namespace nn
{
namespace nex
{

const unsigned long long TRACE_PLUGIN_UNLOAD = 0x1000;

Plugin* Plugin::s_pInstance = 0;

Plugin::Plugin()
{
    s_pInstance = this;
    m_pLibrary = 0;
}

Plugin::~Plugin()
{
}

void Plugin::Unload()
{
    if (m_pLibrary != 0)
    {
        Trace(TRACE_PLUGIN_UNLOAD);
    }
}

void Plugin::Trace(unsigned long long)
{
}

}
}
