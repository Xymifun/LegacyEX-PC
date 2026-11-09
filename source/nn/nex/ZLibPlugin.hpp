#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_ZLIBPLUGIN_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_ZLIBPLUGIN_HPP

#include "nn/nex/CompressionPlugin.hpp"

namespace nn
{
namespace nex
{

class ZLibPlugin : public CompressionPlugin
{
public:
    virtual ~ZLibPlugin() {}

    virtual const wchar_t* GetGUID() { return L"Quazal-ZLib-Fake-GUID"; }

    virtual const wchar_t* GetName() { return L"Quazal ZLib Compression Plugin"; }

    virtual const wchar_t* GetAuthor() { return L"Quazal"; }

    virtual unsigned int GetVersion() { return 1; }

    virtual bool IsCertified() { return true; }

    virtual bool Activate() { return true; }

    virtual void Release() {}

    virtual PluginObject* CreatePluginObject();
    virtual void DeletePluginObject(PluginObject* pObject);

    static ZLibPlugin* GetInstance();

private:
    static ZLibPlugin s_instance;
};

}
}

#endif
