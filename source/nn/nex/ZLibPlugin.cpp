#include "nn/nex/ZLibPlugin.hpp"

#include "nn/nex/PluginObject.hpp"
#include "nn/nex/ZLibCompression.hpp"

namespace nn
{
namespace nex
{

ZLibPlugin ZLibPlugin::s_instance;

ZLibPlugin* ZLibPlugin::GetInstance()
{
    return &s_instance;
}

PluginObject* ZLibPlugin::CreatePluginObject()
{
    ZLibCompression* pCompression = new ZLibCompression;
    pCompression->SetPlugin(this);
    return pCompression;
}

void ZLibPlugin::DeletePluginObject(PluginObject* pObject)
{
    delete pObject;
}

}
}
