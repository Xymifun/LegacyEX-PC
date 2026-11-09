#include "game/client/render/TexturePack.hpp"

bool TexturePack::hasAudio()
{
    return m_bHasAudio;
}

void TexturePack::setHasAudio(bool bHasAudio)
{
    m_bHasAudio = bHasAudio;
}

void TexturePack::loadData()
{
}

std::wstring TexturePack::getResource(const std::wstring& fileName)
{
    return fileName;
}

DLCPack* TexturePack::getDLCPack()
{
    return NULL;
}

std::wstring TexturePack::getPath(bool bTitleUpdate, const char* pFileName)
{
    std::wstring path;

    if (bTitleUpdate)
        path = L"Common\\res\\TitleUpdate\\";
    else
        path = L"Common/";

    return path;
}
