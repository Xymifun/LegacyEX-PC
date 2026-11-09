#include "game/world/block/BlockLayerRuleDefinition.hpp"

#include "game/core/GameRules_IntAttribute.hpp"
#include "game/world/block/BlockPos.hpp"

BlockLayerRuleDefinition::BlockLayerRuleDefinition()
{
    mBlockId = 0;

    addIntAttribute(ConsoleGameRules::eGameRuleAttr_blockId, &mBlockId)->setMin(0)->setMax(255);
}

BlockLayerRuleDefinition::~BlockLayerRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType BlockLayerRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_BlockLayer;
}

bool BlockLayerRuleDefinition::containsBlock(int blockId, const BlockPos& pos)
{
    return mBlockId == blockId && NamedAreaRuleDefinition::containsBlock(blockId, pos);
}
