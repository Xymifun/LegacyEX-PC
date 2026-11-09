#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CONFIGURATION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CONFIGURATION_HPP

#include "nn/nex/RootObject.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class ConfigurationVariable;

class Configuration : public RootObject
{
public:
    Configuration();
    virtual ~Configuration();

    void CreateVariable(ConfigurationVariable* pVariable);
    ConfigurationVariable* FindVariable(wchar_t* szName);
    ConfigurationVariable* GetVariable(unsigned int uiIndex);
    unsigned int GetVariablesNumber();

private:
    qVector<ConfigurationVariable*> m_vVariables;
};

}
}

#endif
