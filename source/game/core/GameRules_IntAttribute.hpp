#ifndef GAME_CORE_GAMERULES_INTATTRIBUTE_HPP
#define GAME_CORE_GAMERULES_INTATTRIBUTE_HPP

#include "game/core/IGameRuleAttribute.hpp"
#include "game/data/GameRuleSaveInterface.hpp"

#include <string>

class GameRules_IntAttribute : public IGameRuleAttribute
{
public:
    GameRules_IntAttribute(int* value);
    virtual ~GameRules_IntAttribute();

    virtual void setValue(const std::wstring& value);
    virtual void save(ConsoleGameRules::EGameRuleAttr attribute, GameRuleSaveInterface* saveInterface);
    virtual void writeAsXml(ConsoleGameRules::EGameRuleAttr attribute, std::string& xml);

    GameRules_IntAttribute* setMin(int min);
    GameRules_IntAttribute* setMax(int max);

private:
    int* mValue;
    int mMin;
    int mMax;
};

#endif
