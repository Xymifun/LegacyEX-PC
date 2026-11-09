#ifndef GAME_MINIGAME_COLLISIONEXCEPTIONGAMERULEDEFINITION_HPP
#define GAME_MINIGAME_COLLISIONEXCEPTIONGAMERULEDEFINITION_HPP

#include "game/minigame/GameRuleDefinition.hpp"

class Block;

class CollisionExceptionGameRuleDefinition : public GameRuleDefinition
{
public:
    CollisionExceptionGameRuleDefinition();


    virtual ConsoleGameRules::EGameRuleType getActionType();

    bool ShouldCollideWith(Block* block);

private:
    int mBlockId;
};

#endif
