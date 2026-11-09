#ifndef GAME_CORE_GAMERULES_STRINGATTRIBUTE_HPP
#define GAME_CORE_GAMERULES_STRINGATTRIBUTE_HPP

#include "game/core/IGameRuleAttribute.hpp"
#include "game/data/GameRuleSaveInterface.hpp"

#include <string>

class GameRules_StringAttribute : public IGameRuleAttribute
{
public:
    GameRules_StringAttribute(std::wstring* value);
    virtual ~GameRules_StringAttribute();

    virtual void setValue(const std::wstring& value);
    virtual void save(ConsoleGameRules::EGameRuleAttr attribute, GameRuleSaveInterface* saveInterface);
    virtual void writeAsXml(ConsoleGameRules::EGameRuleAttr attribute, std::string& xml);

private:
    std::wstring* mValue;
};

#endif
