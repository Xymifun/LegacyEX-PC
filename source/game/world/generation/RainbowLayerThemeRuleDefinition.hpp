#ifndef GAME_WORLD_GENERATION_RAINBOWLAYERTHEMERULEDEFINITION_HPP
#define GAME_WORLD_GENERATION_RAINBOWLAYERTHEMERULEDEFINITION_HPP

#include "game/world/block/GenericMultiBlockThemeRuleDefinition.hpp"
#include "game/world/generation/LayerTheme.hpp"

class RainbowLayerThemeRuleDefinition : public GenericMultiBlockThemeRuleDefinition
{
public:
    virtual ConsoleGameRules::EGameRuleType getActionType();

    virtual LayerTheme* Generate() const;
};

#endif
