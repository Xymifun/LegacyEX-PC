#ifndef GAME_WORLD_ONINITIALISEWORLD_HPP
#define GAME_WORLD_ONINITIALISEWORLD_HPP

#include "game/data/GameRuleSaveInterface.hpp"
#include "game/minigame/CompoundGameRuleDefinition.hpp"

class OnInitialiseWorld : public CompoundGameRuleDefinition
{
public:
    virtual ConsoleGameRules::EGameRuleType getActionType();
};

#endif
