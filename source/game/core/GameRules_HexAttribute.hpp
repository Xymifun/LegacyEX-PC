#ifndef GAME_CORE_GAMERULES_HEXATTRIBUTE_HPP
#define GAME_CORE_GAMERULES_HEXATTRIBUTE_HPP

#include "game/core/IGameRuleAttribute.hpp"
#include "game/data/GameRuleSaveInterface.hpp"

#include <string>

class GameRules_HexAttribute : public IGameRuleAttribute
{
public:
    GameRules_HexAttribute(int* value);
    virtual ~GameRules_HexAttribute();

    virtual void setValue(const std::wstring& value);
    virtual void save(ConsoleGameRules::EGameRuleAttr attribute, GameRuleSaveInterface* saveInterface);
    virtual void writeAsXml(ConsoleGameRules::EGameRuleAttr attribute, std::string& xml);

private:
    int* mValue;
};

#endif
