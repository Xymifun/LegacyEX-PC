#ifndef GAME_WORLD_BLOCK_GAMERULES_BLOCKSTATEATTRIBUTE_HPP
#define GAME_WORLD_BLOCK_GAMERULES_BLOCKSTATEATTRIBUTE_HPP

#include "game/core/IGameRuleAttribute.hpp"
#include "game/data/GameRuleSaveInterface.hpp"

#include <string>

class BlockState;

class GameRules_BlockStateAttribute : public IGameRuleAttribute
{
public:
    GameRules_BlockStateAttribute(const BlockState** value);
    virtual ~GameRules_BlockStateAttribute();

    virtual void setValue(const std::wstring& value);
    virtual void save(ConsoleGameRules::EGameRuleAttr attribute, GameRuleSaveInterface* saveInterface);
    virtual void writeAsXml(ConsoleGameRules::EGameRuleAttr attribute, std::string& xml);

private:
    const BlockState** mValue;
};

#endif
