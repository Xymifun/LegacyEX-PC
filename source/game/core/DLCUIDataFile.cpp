#include "game/core/DLCUIDataFile.hpp"

DLCUIDataFile::DLCUIDataFile(const std::wstring& path)
    : DLCFile(DLCManager::eDLCType_UIData, path),
      m_pData(NULL),
      m_dataSize(0),
      m_bTakeDataOwnership(false)
{
}

DLCUIDataFile::~DLCUIDataFile()
{
    if (m_bTakeDataOwnership && m_pData != NULL)
    {
        delete[] m_pData;
    }
}

unsigned char* DLCUIDataFile::getData(unsigned int& size)
{
    size = m_dataSize;
    return m_pData;
}

void DLCUIDataFile::addData(unsigned char* pData, unsigned int size, bool bTakeDataOwnership)
{
    m_pData = pData;
    m_dataSize = size;
    m_bTakeDataOwnership = bTakeDataOwnership;
}
