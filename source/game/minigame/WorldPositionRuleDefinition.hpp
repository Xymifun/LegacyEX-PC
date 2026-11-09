#ifndef GAME_MINIGAME_WORLDPOSITIONRULEDEFINITION_HPP
#define GAME_MINIGAME_WORLDPOSITIONRULEDEFINITION_HPP

#include "game/minigame/GameRuleDefinition.hpp"

class WorldPositionRuleDefinition : public GameRuleDefinition
{
    friend class MasterGameMode;

public:
    WorldPositionRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();

protected:
    int mX;
    int mY;
    int mZ;
};

#endif
