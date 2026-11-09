#ifndef GAME_WORLD_BLOCK_USEBLOCKRULEDEFINITION_HPP
#define GAME_WORLD_BLOCK_USEBLOCKRULEDEFINITION_HPP

#include "game/minigame/GameRuleDefinition.hpp"

class GameRule;

class UseBlockRuleDefinition : public GameRuleDefinition
{
public:
    UseBlockRuleDefinition();


    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual bool onUseBlock(GameRule* gameRule, int blockId, int x, int y, int z);

private:
    int mBlockId;
    bool mUsePosition;
    int mX;
    int mY;
    int mZ;
};

#endif
