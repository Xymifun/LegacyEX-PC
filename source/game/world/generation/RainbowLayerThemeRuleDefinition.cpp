#include "game/world/generation/RainbowLayerThemeRuleDefinition.hpp"

#include <vector>

#include "game/world/generation/RainbowLayerTheme.hpp"

ConsoleGameRules::EGameRuleType RainbowLayerThemeRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_RainbowTheme;
}

LayerTheme* RainbowLayerThemeRuleDefinition::Generate() const
{
    std::vector<const BlockState*> blocks;
    fillStack(blocks);

    return new RainbowLayerTheme(5, 16.0, blocks);
}
