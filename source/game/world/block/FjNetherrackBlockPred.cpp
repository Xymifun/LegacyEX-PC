#include "game/world/block/FjNetherrackBlockPred.hpp"

#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"

bool FjNetherrackBlockPred::apply(const Biome::BlockPair& blockPair) const
{
    static int netherrackId = Block::getId(Blocks::netherrack);

    bool result = blockPair.blockId == netherrackId;
    return result;
}

FjNetherrackBlockPred::~FjNetherrackBlockPred()
{
}
