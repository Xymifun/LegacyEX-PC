#include "game/core/DLCLocalisationFile.hpp"

#include "game/data/StringTable.hpp"

DLCLocalisationFile::DLCLocalisationFile(const std::wstring& path)
    : DLCFile(DLCManager::eDLCType_Localisation, path),
      m_pStringTable(NULL)
{
}

DLCLocalisationFile::~DLCLocalisationFile()
{
    delete m_pStringTable;
}

void DLCLocalisationFile::addData(unsigned char* pData, unsigned int size)
{
    delete m_pStringTable;

    arrayWithLength<unsigned char> data;
    data.data = pData;
    data.length = size;
    m_pStringTable = new StringTable(data, false, true);
}

StringTable* DLCLocalisationFile::getStringTable()
{
    return m_pStringTable;
}
