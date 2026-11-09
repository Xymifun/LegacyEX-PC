#ifndef GAME_WORLD_BLOCK_MUSHROOMBLOCKTHEMERULEDEFINITION_HPP
#define GAME_WORLD_BLOCK_MUSHROOMBLOCKTHEMERULEDEFINITION_HPP

#include "game/world/generation/LayerAttributeChildRuleDefinition.hpp"
#include "game/world/generation/LayerTheme.hpp"

class MushroomBlockThemeRuleDefinition : public LayerAttributeChildRuleDefinition<LayerTheme*>
{
public:
    MushroomBlockThemeRuleDefinition();


    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual LayerTheme* Generate() const;

private:
    Block* mBlock;
};

#endif
