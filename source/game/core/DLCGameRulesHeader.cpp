#include "game/core/DLCGameRulesHeader.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"

#include "game/core/GameRuleManager.hpp"


DLCGameRulesHeader::DLCGameRulesHeader(const std::wstring& path)
    : DLCGameRules(DLCManager::eDLCType_GameRulesHeader, path),
      m_data(NULL),
      m_dataSize(0),
      m_ready(false)
{
    m_grfPath = path.substr(0, path.size() - 4) + L".grf";
    m_levelGenerationOptions = NULL;
}

bool DLCGameRulesHeader::requiresTexturePack()
{
    return m_requiresTexturePack;
}

unsigned int DLCGameRulesHeader::getTexturePackId()
{
    return m_texturePackId;
}

std::wstring DLCGameRulesHeader::getDefaultSaveName()
{
    return m_defaultSaveName;
}

const wchar_t* DLCGameRulesHeader::getWorldName()
{
    return m_worldName.c_str();
}

const wchar_t* DLCGameRulesHeader::getDisplayName()
{
    return m_displayName.c_str();
}

std::wstring DLCGameRulesHeader::getGrfPath()
{
    return m_grfPath;
}

void DLCGameRulesHeader::setRequiresTexturePack(bool required)
{
    m_requiresTexturePack = required;
}

void DLCGameRulesHeader::setTexturePackId(unsigned int id)
{
    m_texturePackId = id;
}

void DLCGameRulesHeader::setDefaultSaveName(const std::wstring& name)
{
    m_defaultSaveName = name;
}

void DLCGameRulesHeader::setWorldName(const std::wstring& name)
{
    m_worldName = name;
}

void DLCGameRulesHeader::setDisplayName(const std::wstring& name)
{
    m_displayName = name;
}

void DLCGameRulesHeader::setGrfPath(const std::wstring& path)
{
    m_grfPath = path;
}

bool DLCGameRulesHeader::ready()
{
    return m_ready;
}

void DLCGameRulesHeader::addData(unsigned char* data, unsigned int size)
{
    m_data = data;
    m_dataSize = size;
}

unsigned char* DLCGameRulesHeader::getData(unsigned int& size)
{
    size = m_dataSize;
    return m_data;
}

void DLCGameRulesHeader::setGrfData(unsigned char* data, unsigned int size, StringTable* stringTable)
{
    if (m_ready)
        return;

    m_ready = true;
    gApp.mGameRuleManager.readRuleFile(m_levelGenerationOptions, data, size, stringTable);
}
