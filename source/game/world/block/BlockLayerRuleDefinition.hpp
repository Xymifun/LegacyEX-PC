#ifndef GAME_WORLD_BLOCK_BLOCKLAYERRULEDEFINITION_HPP
#define GAME_WORLD_BLOCK_BLOCKLAYERRULEDEFINITION_HPP

#include "game/minigame/NamedAreaRuleDefinition.hpp"

class BlockPos;

class BlockLayerRuleDefinition : public NamedAreaRuleDefinition
{
public:
    BlockLayerRuleDefinition();

    virtual ~BlockLayerRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual bool containsBlock(int blockId, const BlockPos& pos);

private:
    int mBlockId;
};

#endif
