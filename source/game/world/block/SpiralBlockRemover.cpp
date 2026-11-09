#include "game/world/block/SpiralBlockRemover.hpp"

#include "game/core/Direction.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"

void SpiralBlockRemover::Tick()
{
    BlockRemover::Tick();

    if (mTicks % mBlocksPerRemoval == 0)
    {
        ++mBlocksPerRemoval;
    }
}

bool SpiralBlockRemover::RemoveBlocks(int areaIndex, int maxBlocks)
{
    Area& area = mAreas[areaIndex];
    Spiral& spiral = mSpirals[areaIndex];
    const Direction* axis = area.mBounds->mMaxX - area.mBounds->mMinX == 1.0 ? Direction::EAST : Direction::NORTH;
    int removed = 0;
    int x;
    int y;
    BlockPos pos;

    while (removed < maxBlocks && spiral.GetNext(x, y))
    {
        if (axis == Direction::EAST)
        {
            pos = BlockPos((int)area.mBounds->mMinX, y, x);
        }
        else
        {
            pos = BlockPos(x, y, (int)area.mBounds->mMinZ);
        }

        if (!area.mBounds->containsIncludingLowerBound(Vec3::newTemp(&pos)))
        {
            continue;
        }

        mLevel->setBlock(pos, Blocks::air->defaultBlockState(), Block::UPDATE_ALL, false);
        ++removed;
    }

    if (removed > 0)
    {
        CreateParticles(pos, 0);
    }

    return removed >= maxBlocks;
}
