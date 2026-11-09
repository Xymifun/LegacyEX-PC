#ifndef GAME_MINIGAME_NULLTHEMERULEDEFINITION_HPP
#define GAME_MINIGAME_NULLTHEMERULEDEFINITION_HPP

#include "game/world/generation/LayerAttributeChildRuleDefinition.hpp"
#include "game/world/generation/LayerTheme.hpp"

class NullThemeRuleDefinition : public LayerAttributeChildRuleDefinition<LayerTheme*>
{
public:
    virtual ConsoleGameRules::EGameRuleType getActionType();

    virtual LayerTheme* Generate() const;
};

#endif
