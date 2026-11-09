#ifndef GAME_WORLD_GENERATION_PLAYERBOUNDSVOLUMERULEDEFINITION_HPP
#define GAME_WORLD_GENERATION_PLAYERBOUNDSVOLUMERULEDEFINITION_HPP

#include "game/minigame/NamedAreaRuleDefinition.hpp"

class PlayerBoundsVolumeRuleDefinition : public NamedAreaRuleDefinition
{
public:
    PlayerBoundsVolumeRuleDefinition();
    PlayerBoundsVolumeRuleDefinition(int x0, int y0, int z0, int x1, int y1, int z1);

    virtual ~PlayerBoundsVolumeRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();
};

#endif
