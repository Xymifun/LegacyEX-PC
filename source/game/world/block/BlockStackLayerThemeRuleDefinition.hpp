#ifndef GAME_WORLD_BLOCK_BLOCKSTACKLAYERTHEMERULEDEFINITION_HPP
#define GAME_WORLD_BLOCK_BLOCKSTACKLAYERTHEMERULEDEFINITION_HPP

#include "game/world/block/GenericMultiBlockThemeRuleDefinition.hpp"
#include "game/world/generation/LayerTheme.hpp"

class BlockStackLayerThemeRuleDefinition : public GenericMultiBlockThemeRuleDefinition
{
public:
    virtual ConsoleGameRules::EGameRuleType getActionType();

    virtual LayerTheme* Generate() const;
};

#endif
