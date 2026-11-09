#include "nn/nex/Configuration.hpp"

#include "nn/nex/ConfigurationVariable.hpp"

namespace nn
{
namespace nex
{

Configuration::~Configuration()
{
    for (unsigned int i = 0; i < m_vVariables.size(); ++i)
    {
        delete m_vVariables[i];
        m_vVariables[i] = 0;
    }

    m_vVariables.clear();
}

}
}
