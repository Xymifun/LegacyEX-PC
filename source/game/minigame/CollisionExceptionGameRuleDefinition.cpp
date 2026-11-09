#include "game/minigame/CollisionExceptionGameRuleDefinition.hpp"

#include "game/world/block/Block.hpp"

CollisionExceptionGameRuleDefinition::CollisionExceptionGameRuleDefinition() : mBlockId(0)
{
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_blockId, &mBlockId);
}

ConsoleGameRules::EGameRuleType CollisionExceptionGameRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_BlockCollisionException;
}

bool CollisionExceptionGameRuleDefinition::ShouldCollideWith(Block* block)
{
    return Block::getId(block) != mBlockId;
}
