#ifndef GAME_CLIENT_RENDER_FILETEXTUREPACK_HPP
#define GAME_CLIENT_RENDER_FILETEXTUREPACK_HPP

#include <string>

#include "game/client/render/AbstractTexturePack.hpp"

class File;

class FileTexturePack : public AbstractTexturePack
{
public:
    FileTexturePack(unsigned int id, File* file, TexturePack* parentPack);
    virtual ~FileTexturePack() {}

    virtual void unload(Textures* textures);
    virtual InputStream* getResourceImplementation(const std::wstring& fileName);
    virtual bool hasFile(const std::wstring& fileName);
    virtual bool isTerrainUpdateCompatible();

private:
    void loadZipFile();
};

#endif
