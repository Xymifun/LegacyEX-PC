#include "game/world/block/BlockStackLayerThemeRuleDefinition.hpp"

#include <vector>

#include "game/world/block/BlockStackLayerTheme.hpp"

ConsoleGameRules::EGameRuleType BlockStackLayerThemeRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_BlockStackTheme;
}

LayerTheme* BlockStackLayerThemeRuleDefinition::Generate() const
{
    std::vector<const BlockState*> states;
    fillStack(states);

    return new BlockStackLayerTheme(states);
}
