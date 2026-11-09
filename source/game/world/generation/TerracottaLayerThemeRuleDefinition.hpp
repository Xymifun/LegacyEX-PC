#ifndef GAME_WORLD_GENERATION_TERRACOTTALAYERTHEMERULEDEFINITION_HPP
#define GAME_WORLD_GENERATION_TERRACOTTALAYERTHEMERULEDEFINITION_HPP

#include <string>

#include "game/world/generation/LayerAttributeChildRuleDefinition.hpp"
#include "game/world/generation/LayerTheme.hpp"
#include "game/world/generation/TerracottaLayerTheme.hpp"

class TerracottaLayerThemeRuleDefinition : public LayerAttributeChildRuleDefinition<LayerTheme*>
{
public:
    TerracottaLayerThemeRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual LayerTheme* Generate() const;

private:
    std::wstring mPattern;
    std::wstring mColourFunction;
};

#endif
