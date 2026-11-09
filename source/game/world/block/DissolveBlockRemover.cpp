#include "game/world/block/DissolveBlockRemover.hpp"

#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/MaterialColor.hpp"

DissolveBlockRemover::DissolveBlockRemover(Level* level, std::vector<AABB*>& areas, int passCount, int ticksPerRemoval)
    : BlockRemover(level, areas)
{
    mPassCount = passCount;
    mBlocksPerRemoval = 1;
    mTicksPerRemoval = ticksPerRemoval;
}

void DissolveBlockRemover::Tick()
{
    BlockRemover::Tick();

    if (mRemovalCount >= mPassCount)
    {
        mFinished = true;
    }
}

bool DissolveBlockRemover::RemoveBlocks(int areaIndex, int maxBlocks)
{
    Area& area = mAreas[areaIndex];
    int phase = -mRemovalCount;

    for (int y = (int)area.mBounds->mMinY; y < area.mBounds->mMaxY; ++y)
    {
        for (int z = (int)area.mBounds->mMinZ; z < area.mBounds->mMaxZ; ++z)
        {
            for (int x = (int)area.mBounds->mMinX; x < area.mBounds->mMaxX; ++x)
            {
                if (phase >= 0 && phase % mPassCount == 0)
                {
                    BlockPos pos(x, y, z);
                    const BlockState* state = mLevel->getBlockState(pos);
                    int colour = const_cast<BlockState*>(state)->getMapColor(mLevel, pos)->calculateRGBColor(2);
                    mLevel->setBlock(pos, Blocks::air->defaultBlockState(), Block::UPDATE_ALL, false);
                    CreateParticles(pos, colour);
                }

                ++phase;
            }
        }
    }

    return true;
}
