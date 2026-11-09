#include "game/world/block/DLCMaterialFile.hpp"

#include "game/client/render/TexturePackRepository.hpp"
#include "game/core/Minecraft.hpp"

DLCMaterialFile::DLCMaterialFile(const std::wstring& path)
    : DLCFile(DLCManager::eDLCType_Material, path),
      m_pMaterialTable(NULL)
{
}

DLCMaterialFile::~DLCMaterialFile()
{
    if (m_pMaterialTable)
    {
        delete m_pMaterialTable;
    }
}

void DLCMaterialFile::addData(unsigned char* pData, unsigned int size)
{
    MaterialTable* defaultMaterials =
        Minecraft::GetInstance()->mTexturePackRepository->defaultTexturePack->getMaterialTable();
    MaterialTable* table = new MaterialTable(*defaultMaterials, pData, size);
    m_pMaterialTable = table;
}
