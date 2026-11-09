#include "game/world/generation/BasicPatchesLayerThemeRuleDefinition.hpp"

#include <vector>

#include "game/core/GameRules_DoubleAttribute.hpp"
#include "game/core/GameRules_FloatAttribute.hpp"
#include "game/core/GameRules_IntAttribute.hpp"
#include "game/world/generation/BasicPatchesLayerTheme.hpp"

BasicPatchesLayerThemeRuleDefinition::BasicPatchesLayerThemeRuleDefinition()
    : GenericMultiBlockThemeRuleDefinition(),
      mPatchThreshold(0.0f),
      mNoiseScale(5.0),
      mPatchDepth(0)
{
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_startHeight, &mPatchDepth)->setMax(9);
    addFloatAttribute(ConsoleGameRules::eGameRuleAttr_freq, &mPatchThreshold)->setMin(-1.1f)->setMax(1.1f);
    addDoubleAttribute(ConsoleGameRules::eGameRuleAttr_scale, &mNoiseScale)->setMin(1.0)->setMax(1000.0);
}

ConsoleGameRules::EGameRuleType BasicPatchesLayerThemeRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_BasicPatchesTheme;
}

LayerTheme* BasicPatchesLayerThemeRuleDefinition::Generate() const
{
    std::vector<const BlockState*> blocks;
    fillStack(blocks);

    return new BasicPatchesLayerTheme(5, mNoiseScale, mPatchDepth, blocks, mPatchThreshold);
}
