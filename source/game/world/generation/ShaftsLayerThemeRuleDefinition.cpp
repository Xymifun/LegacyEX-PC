#include "game/world/generation/ShaftsLayerThemeRuleDefinition.hpp"

#include <vector>

#include "game/core/GameRules_IntAttribute.hpp"
#include "game/world/generation/ShaftsLayerTheme.hpp"

ShaftsLayerThemeRuleDefinition::ShaftsLayerThemeRuleDefinition()
    : GenericMultiBlockThemeRuleDefinition(),
      mHeightOffset(0)
{
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_startHeight, &mHeightOffset)->setMin(-9)->setMax(9);
}

ConsoleGameRules::EGameRuleType ShaftsLayerThemeRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_ShaftsTheme;
}

LayerTheme* ShaftsLayerThemeRuleDefinition::Generate() const
{
    std::vector<const BlockState*> blocks;
    fillStack(blocks);

    return new ShaftsLayerTheme(mHeightOffset, blocks);
}
