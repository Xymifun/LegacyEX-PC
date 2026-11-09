#include "game/world/generation/SimplePatchesLayerThemeRuleDefinition.hpp"

#include "game/core/GameRules_FloatAttribute.hpp"
#include "game/core/GameRules_IntAttribute.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/generation/SimplePatchesLayerTheme.hpp"

SimplePatchesLayerThemeRuleDefinition::SimplePatchesLayerThemeRuleDefinition()
    : mBlockId(-1),
      mBlockAux(0),
      mNoiseScale(5.0f),
      mPatchThreshold(0.0f)
{
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_blockId, &mBlockId)->setMin(0)->setMax(255);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_blockAux, &mBlockAux);
    addFloatAttribute(ConsoleGameRules::eGameRuleAttr_scale, &mNoiseScale)->setMin(0.0f)->setMax(100.0f);
    addFloatAttribute(ConsoleGameRules::eGameRuleAttr_freq, &mPatchThreshold)->setMin(-1.0f)->setMax(1.0f);
}

ConsoleGameRules::EGameRuleType SimplePatchesLayerThemeRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_SimplePatchesTheme;
}

LayerTheme* SimplePatchesLayerThemeRuleDefinition::Generate() const
{
    Block* block = Block::byId(mBlockId);
    const BlockState* blockState = block->getBlockState(mBlockAux);
    return new SimplePatchesLayerTheme(5, static_cast<double>(mNoiseScale), blockState, mPatchThreshold);
}
