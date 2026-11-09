#include "game/client/render/DLCModelsFile.hpp"

DLCModelsFile::DLCModelsFile(const std::wstring& path) : DLCFile(DLCManager::eDLCType_Models, path)
{
}

DLCModelsFile::~DLCModelsFile()
{
}

void DLCModelsFile::addData(unsigned char* pData, unsigned int size)
{
    m_data.data = pData;
    m_data.length = size;
}

arrayWithLength<unsigned char> DLCModelsFile::getData()
{
    return arrayWithLength<unsigned char>(m_data.data, m_data.length);
}
