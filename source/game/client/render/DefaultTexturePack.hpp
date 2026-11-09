#ifndef GAME_CLIENT_RENDER_DEFAULTTEXTUREPACK_HPP
#define GAME_CLIENT_RENDER_DEFAULTTEXTUREPACK_HPP

#include <string>

#include "game/client/render/AbstractTexturePack.hpp"

class DefaultTexturePack : public AbstractTexturePack
{
public:
    DefaultTexturePack();

    virtual bool hasData();
    virtual bool hasValidData();
    virtual bool hasAudio();
    virtual bool isLoadingData();
    virtual std::wstring getName();
    virtual std::wstring getDesc1();
    virtual bool isTerrainUpdateCompatible();
    virtual DLCPack* getDLCPack();
    virtual void loadUI();
    virtual void unloadUI();
    virtual void loadCustomModels();
    virtual void loadIcon();
    virtual void loadDescription();
    virtual void loadName();
    virtual InputStream* getResourceImplementation(const std::wstring& fileName);
    virtual bool hasFile(const std::wstring& fileName);
};

#endif
