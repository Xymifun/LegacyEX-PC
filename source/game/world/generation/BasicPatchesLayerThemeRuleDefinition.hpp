#ifndef GAME_WORLD_GENERATION_BASICPATCHESLAYERTHEMERULEDEFINITION_HPP
#define GAME_WORLD_GENERATION_BASICPATCHESLAYERTHEMERULEDEFINITION_HPP

#include "game/minigame/GameRuleDefinition.hpp"
#include "game/world/block/GenericMultiBlockThemeRuleDefinition.hpp"
#include "game/world/generation/LayerTheme.hpp"

class BasicPatchesLayerThemeRuleDefinition : public GenericMultiBlockThemeRuleDefinition
{
public:
    BasicPatchesLayerThemeRuleDefinition();


    virtual ConsoleGameRules::EGameRuleType getActionType();

    virtual LayerTheme* Generate() const;

private:
    float mPatchThreshold;
    double mNoiseScale;
    int mPatchDepth;
};

#endif
