#include "game/world/block/BlockRotProcessor.hpp"

#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/StructurePlaceSettings.hpp"

StructureTemplate::StructureBlockInfo* BlockRotProcessor::processBlock(Level* level, const BlockPos& position,
                                                                       StructureTemplate::StructureBlockInfo* blockInfo)
{
    if (mIntegrity < 1.0f && mRandom->nextFloat() > mIntegrity)
        blockInfo = NULL;
    return blockInfo;
}
