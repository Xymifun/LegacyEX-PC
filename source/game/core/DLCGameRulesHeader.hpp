#ifndef GAME_CORE_DLCGAMERULESHEADER_HPP
#define GAME_CORE_DLCGAMERULESHEADER_HPP

#include <string>

#include "game/core/DLCGameRules.hpp"
#include "game/core/GameRulesSource.hpp"

class LevelGenerationOptions;
class StringTable;

class DLCGameRulesHeader : public DLCGameRules, public GameRulesSource
{
public:
    DLCGameRulesHeader(const std::wstring& path);

    virtual bool requiresTexturePack();
    virtual unsigned int getTexturePackId();
    virtual std::wstring getDefaultSaveName();
    virtual const wchar_t* getWorldName();
    virtual const wchar_t* getDisplayName();
    virtual std::wstring getGrfPath();
    virtual void setRequiresTexturePack(bool required);
    virtual void setTexturePackId(unsigned int id);
    virtual void setDefaultSaveName(const std::wstring& name);
    virtual void setWorldName(const std::wstring& name);
    virtual void setDisplayName(const std::wstring& name);
    virtual void setGrfPath(const std::wstring& path);
    virtual bool ready();

    virtual void addData(unsigned char* data, unsigned int size);
    virtual unsigned char* getData(unsigned int& size);
    void setGrfData(unsigned char* data, unsigned int size, StringTable* stringTable);

private:
    friend class GameRuleManager;

    unsigned char* m_data;
    unsigned int m_dataSize;
    bool m_ready;

public:
    LevelGenerationOptions* m_levelGenerationOptions;
};

#endif
