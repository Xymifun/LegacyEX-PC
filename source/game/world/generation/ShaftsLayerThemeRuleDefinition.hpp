#ifndef GAME_WORLD_GENERATION_SHAFTSLAYERTHEMERULEDEFINITION_HPP
#define GAME_WORLD_GENERATION_SHAFTSLAYERTHEMERULEDEFINITION_HPP

#include "game/world/block/GenericMultiBlockThemeRuleDefinition.hpp"

class ShaftsLayerThemeRuleDefinition : public GenericMultiBlockThemeRuleDefinition
{
public:
    ShaftsLayerThemeRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual LayerTheme* Generate() const;

protected:
    int mHeightOffset;
};

#endif
