#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PLUGIN_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PLUGIN_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class PluginObject;

class Plugin : public RootObject
{
public:
    Plugin();
    virtual ~Plugin();

    virtual const wchar_t* GetGUID() = 0;
    virtual const wchar_t* GetName() = 0;
    virtual const wchar_t* GetAuthor() = 0;
    virtual unsigned int GetVersion() = 0;
    virtual bool IsCertified() = 0;
    virtual unsigned int GetType() = 0;
    virtual bool Activate() = 0;
    virtual void Release() = 0;

    virtual bool Initialize() { return true; }

    virtual void ThreadAttach() {}

    virtual void ThreadDetach() {}

    virtual void Destroy() {}

    virtual PluginObject* CreatePluginObject() = 0;
    virtual void DeletePluginObject(PluginObject* pObject) = 0;

    void SetLibrary(void* pLibrary) { m_pLibrary = pLibrary; }

    void Unload();
    void Trace(unsigned long long uiFilter);

    static Plugin* GetInstance() { return s_pInstance; }

private:
    static Plugin* s_pInstance;

    void* m_pLibrary;
};

}
}

#endif
