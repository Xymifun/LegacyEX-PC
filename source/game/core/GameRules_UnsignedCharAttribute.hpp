#ifndef GAME_CORE_GAMERULES_UNSIGNEDCHARATTRIBUTE_HPP
#define GAME_CORE_GAMERULES_UNSIGNEDCHARATTRIBUTE_HPP

#include "game/core/IGameRuleAttribute.hpp"
#include "game/data/GameRuleSaveInterface.hpp"

#include <string>

class GameRules_UnsignedCharAttribute : public IGameRuleAttribute
{
public:
    GameRules_UnsignedCharAttribute(unsigned char* value);
    virtual ~GameRules_UnsignedCharAttribute();

    virtual void setValue(const std::wstring& value);
    virtual void save(ConsoleGameRules::EGameRuleAttr attribute, GameRuleSaveInterface* saveInterface);
    virtual void writeAsXml(ConsoleGameRules::EGameRuleAttr attribute, std::string& xml);

private:
    unsigned char* mValue;
};

#endif
