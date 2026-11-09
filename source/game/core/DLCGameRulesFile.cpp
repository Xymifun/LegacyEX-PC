#include "game/core/DLCGameRulesFile.hpp"

DLCGameRulesFile::DLCGameRulesFile(const std::wstring& path)
    : DLCGameRules(DLCManager::eDLCType_GameRules, path),
      m_pData(NULL),
      m_dataSize(0),
      m_bFree(false)
{
}

void DLCGameRulesFile::addData(unsigned char* pData, unsigned int size)
{
    m_pData = pData;
    m_dataSize = size;
}

unsigned char* DLCGameRulesFile::getData(unsigned int& size)
{
    size = m_dataSize;
    return m_pData;
}

void DLCGameRulesFile::addParameter(DLCManager::EDLCParameterType parameter, const std::wstring& value)
{
    if (parameter == DLCManager::eDLCParameterType_Free)
    {
        m_bFree = true;
    }
}

bool DLCGameRulesFile::getParameterAsBool(DLCManager::EDLCParameterType parameter)
{
    if (parameter != DLCManager::eDLCParameterType_Free)
    {
        return false;
    }

    return m_bFree;
}
