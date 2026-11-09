#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CONFIGURATIONVARIABLE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CONFIGURATIONVARIABLE_HPP

#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class ConfigurationVariable : public RootObject
{
public:
    ConfigurationVariable(wchar_t* szName, wchar_t* szDefaultValue);
    virtual ~ConfigurationVariable();

    String& GetName();
    void SetName(wchar_t* szName);
    String& GetValue();
    void SetValue(const wchar_t* szValue);
    void SetValue(String strValue);
    unsigned int GetSourceType();

    bool Read();
    bool ReadFromDefaultValue();
    bool ReadFromFile();
    bool ReadFromEnvironment();
    bool ReadFromRegistry();
    bool WriteToRegistry();

    void InitVariables();

private:
    String m_strName;
    String m_strValue;
    unsigned int m_uiSourceType;
};

}
}

#endif
