#include "game/core/DLCBehavioursFile.hpp"

DLCBehavioursFile::DLCBehavioursFile(const std::wstring& path) : DLCFile(DLCManager::eDLCType_Behaviours, path)
{
}

DLCBehavioursFile::~DLCBehavioursFile()
{
}

void DLCBehavioursFile::addData(unsigned char* pData, unsigned int size)
{
    m_data.data = pData;
    m_data.length = size;
}

arrayWithLength<unsigned char> DLCBehavioursFile::getData()
{
    return arrayWithLength<unsigned char>(m_data.data, m_data.length);
}
