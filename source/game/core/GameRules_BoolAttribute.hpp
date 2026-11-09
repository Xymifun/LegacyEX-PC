#ifndef GAME_CORE_GAMERULES_BOOLATTRIBUTE_HPP
#define GAME_CORE_GAMERULES_BOOLATTRIBUTE_HPP

#include "game/core/IGameRuleAttribute.hpp"
#include "game/data/GameRuleSaveInterface.hpp"

#include <string>

class GameRules_BoolAttribute : public IGameRuleAttribute
{
public:
    GameRules_BoolAttribute(bool* value);
    virtual ~GameRules_BoolAttribute();

    virtual void setValue(const std::wstring& value);
    virtual void save(ConsoleGameRules::EGameRuleAttr attribute, GameRuleSaveInterface* saveInterface);
    virtual void writeAsXml(ConsoleGameRules::EGameRuleAttr attribute, std::string& xml);

private:
    bool* mValue;
};

#endif
