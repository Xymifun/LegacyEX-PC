#include "game/core/GameRulesSource.hpp"

GameRulesSource::GameRulesSource()
{
    m_displayName = L"Default_DisplayName";
    m_worldName = L"Default_WorldName";
    m_defaultSaveName = L"Default_DefaultSaveName";
    m_requiresTexturePack = false;
    m_texturePackId = 0;
    m_grfPath = L"__NO_GRF_PATH__";
    m_requiresBaseSave = false;
    m_isGameModeLobby = false;
}

bool GameRulesSource::requiresTexturePack()
{
    return m_requiresTexturePack;
}

unsigned int GameRulesSource::getTexturePackId()
{
    return m_texturePackId;
}

std::wstring GameRulesSource::getDefaultSaveName()
{
    return m_defaultSaveName;
}

const wchar_t* GameRulesSource::getWorldName()
{
    return m_worldName.c_str();
}

const wchar_t* GameRulesSource::getDisplayName()
{
    return m_displayName.c_str();
}

std::wstring GameRulesSource::getGrfPath()
{
    return m_grfPath;
}

bool GameRulesSource::requiresBaseSave()
{
    return m_requiresBaseSave;
}

std::wstring GameRulesSource::getBaseSavePath()
{
    return m_baseSavePath;
}

bool GameRulesSource::isGameModeLobby()
{
    return m_isGameModeLobby;
}

void GameRulesSource::setRequiresTexturePack(bool requiresTexturePack)
{
    m_requiresTexturePack = requiresTexturePack;
}

void GameRulesSource::setTexturePackId(unsigned int texturePackId)
{
    m_texturePackId = texturePackId;
}

void GameRulesSource::setDefaultSaveName(const std::wstring& defaultSaveName)
{
    m_defaultSaveName = defaultSaveName;
}

void GameRulesSource::setWorldName(const std::wstring& worldName)
{
    m_worldName = worldName;
}

void GameRulesSource::setDisplayName(const std::wstring& displayName)
{
    m_displayName = displayName;
}

void GameRulesSource::setGrfPath(const std::wstring& grfPath)
{
    m_grfPath = grfPath;
}

void GameRulesSource::setBaseSavePath(const std::wstring& baseSavePath)
{
    m_baseSavePath = baseSavePath;
    m_requiresBaseSave = true;
}

void GameRulesSource::setIsGameModeLobby(bool isGameModeLobby)
{
    m_isGameModeLobby = isGameModeLobby;
}

bool GameRulesSource::ready()
{
    return true;
}
