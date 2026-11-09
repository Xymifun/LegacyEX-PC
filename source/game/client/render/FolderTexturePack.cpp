#include "game/client/render/FolderTexturePack.hpp"

#include <string>

#include "game/core/File.hpp"
#include "game/core/InputStream.hpp"

InputStream* FolderTexturePack::getResourceImplementation(const std::wstring& fileName)
{
    std::wstring strPath = L"";
    strPath = L"Common\\DummyTexturePack\\res";

    std::wstring strFullPath = strPath + fileName;
    return InputStream::getResourceAsStream(strFullPath);
}

bool FolderTexturePack::hasFile(const std::wstring& fileName)
{
    File file(getPath(false, NULL) + fileName);
    return file.exists() && file.isFile();
}

bool FolderTexturePack::isTerrainUpdateCompatible()
{
    return true;
}

std::wstring FolderTexturePack::getPath(bool bTitleUpdate, const char* pFileName)
{
    std::wstring path;
    path = L"Common\\" + m_pFile->getPath() + L"\\";
    return path;
}

void FolderTexturePack::loadUI()
{
}

void FolderTexturePack::unloadUI()
{
}
