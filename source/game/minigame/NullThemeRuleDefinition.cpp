#include "game/minigame/NullThemeRuleDefinition.hpp"

#include "game/world/generation/VoidLayerTheme.hpp"

ConsoleGameRules::EGameRuleType NullThemeRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_NullTheme;
}

LayerTheme* NullThemeRuleDefinition::Generate() const
{
    return new VoidLayerTheme();
}
