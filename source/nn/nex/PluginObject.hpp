#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PLUGINOBJECT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PLUGINOBJECT_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Plugin;

class PluginObject : public RootObject
{
public:
    PluginObject() : m_pPlugin(0) {}

    virtual ~PluginObject() {}

    void SetPlugin(Plugin* pPlugin) { m_pPlugin = pPlugin; }

private:
    Plugin* m_pPlugin;
};

}
}

#endif
