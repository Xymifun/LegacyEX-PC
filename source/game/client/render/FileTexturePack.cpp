#include "game/client/render/FileTexturePack.hpp"

void FileTexturePack::unload(Textures* textures)
{
}

InputStream* FileTexturePack::getResourceImplementation(const std::wstring& fileName)
{
    return NULL;
}

bool FileTexturePack::hasFile(const std::wstring& fileName)
{
    return false;
}

bool FileTexturePack::isTerrainUpdateCompatible()
{
    return false;
}
