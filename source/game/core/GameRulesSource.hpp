#ifndef GAME_CORE_GAMERULESSOURCE_HPP
#define GAME_CORE_GAMERULESSOURCE_HPP

#include <string>

class GameRulesSource
{
public:
    GameRulesSource();

    virtual bool requiresTexturePack();
    virtual unsigned int getTexturePackId();
    virtual std::wstring getDefaultSaveName();
    virtual const wchar_t* getWorldName();
    virtual const wchar_t* getDisplayName();
    virtual std::wstring getGrfPath();
    virtual bool requiresBaseSave();
    virtual std::wstring getBaseSavePath();
    virtual bool isGameModeLobby();
    virtual void setRequiresTexturePack(bool requiresTexturePack);
    virtual void setTexturePackId(unsigned int texturePackId);
    virtual void setDefaultSaveName(const std::wstring& defaultSaveName);
    virtual void setWorldName(const std::wstring& worldName);
    virtual void setDisplayName(const std::wstring& displayName);
    virtual void setGrfPath(const std::wstring& grfPath);
    virtual void setBaseSavePath(const std::wstring& baseSavePath);
    virtual void setIsGameModeLobby(bool isGameModeLobby);
    virtual bool ready();
    virtual ~GameRulesSource() {}

protected:
    std::wstring m_worldName;
    std::wstring m_displayName;
    std::wstring m_defaultSaveName;
    bool m_requiresTexturePack;
    unsigned int m_texturePackId;
    std::wstring m_grfPath;
    std::wstring m_baseSavePath;
    bool m_requiresBaseSave;
    bool m_isGameModeLobby;
};

#endif
