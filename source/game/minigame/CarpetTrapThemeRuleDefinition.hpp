#ifndef GAME_MINIGAME_CARPETTRAPTHEMERULEDEFINITION_HPP
#define GAME_MINIGAME_CARPETTRAPTHEMERULEDEFINITION_HPP

#include "game/world/generation/LayerAttributeChildRuleDefinition.hpp"
#include "game/world/generation/LayerTheme.hpp"

class CarpetTrapThemeRuleDefinition : public LayerAttributeChildRuleDefinition<LayerTheme*>
{
public:
    CarpetTrapThemeRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual LayerTheme* Generate() const;

private:
    int mCarpetColour;
    float mNoiseScale;
};

#endif
