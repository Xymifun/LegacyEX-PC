#include "game/core/DLCColourTableFile.hpp"

#include "game/client/render/TexturePackRepository.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/ColourTable.hpp"

DLCColourTableFile::DLCColourTableFile(const std::wstring& path)
    : DLCFile(DLCManager::eDLCType_ColourTable, path),
      m_pColourTable(NULL)
{
}

DLCColourTableFile::~DLCColourTableFile()
{
    if (m_pColourTable != NULL)
    {
        delete m_pColourTable;
    }
}

void DLCColourTableFile::addData(unsigned char* pData, unsigned int size)
{
    ColourTable* defaultColours =
        Minecraft::GetInstance()->mTexturePackRepository->defaultTexturePack->getColourTable();
    ColourTable* table = new ColourTable(defaultColours, pData, size);
    m_pColourTable = table;
}
