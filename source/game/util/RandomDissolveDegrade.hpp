#ifndef GAME_UTIL_RANDOMDISSOLVEDEGRADE_HPP
#define GAME_UTIL_RANDOMDISSOLVEDEGRADE_HPP

#include "game/minigame/DegradationRuleDefinition.hpp"

class RandomDissolveDegrade : public DegradationRuleDefinition
{
public:
    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual void NextBlocks(BlockDegradeRoutine::LayerCache* layerCache, arrayWithLength<BlockPos>& blocks);
};

#endif
