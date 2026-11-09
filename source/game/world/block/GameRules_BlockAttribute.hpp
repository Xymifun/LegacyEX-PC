#ifndef GAME_WORLD_BLOCK_GAMERULES_BLOCKATTRIBUTE_HPP
#define GAME_WORLD_BLOCK_GAMERULES_BLOCKATTRIBUTE_HPP

#include "game/core/IGameRuleAttribute.hpp"
#include "game/data/GameRuleSaveInterface.hpp"

#include <string>

class Block;

class GameRules_BlockAttribute : public IGameRuleAttribute
{
public:
    GameRules_BlockAttribute(Block** value);
    virtual ~GameRules_BlockAttribute();

    virtual void setValue(const std::wstring& value);
    virtual void save(ConsoleGameRules::EGameRuleAttr attribute, GameRuleSaveInterface* saveInterface);
    virtual void writeAsXml(ConsoleGameRules::EGameRuleAttr attribute, std::string& xml);

private:
    Block** mValue;
};

#endif
