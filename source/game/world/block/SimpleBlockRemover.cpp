#include "game/world/block/SimpleBlockRemover.hpp"

#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"

bool SimpleBlockRemover::RemoveBlocks(int areaIndex, int maxBlocks)
{
    Area& area = mAreas[areaIndex];
    int removed = 0;

    while (area.mY >= area.mBounds->mMinY && removed < maxBlocks)
    {
        for (area.mX = (int)area.mBounds->mMinX; area.mX < (int)area.mBounds->mMaxX; ++area.mX)
        {
            for (area.mZ = (int)area.mBounds->mMinZ; area.mZ < (int)area.mBounds->mMaxZ; ++area.mZ)
            {
                BlockPos pos(area.mX, area.mY, area.mZ);

                if (mLevel->getBlock(pos) != Blocks::air)
                {
                    mLevel->setBlock(pos, Blocks::air->defaultBlockState(), Block::UPDATE_ALL, false);

                    ++removed;

                    if (removed >= maxBlocks)
                    {
                        return true;
                    }
                }
            }
        }

        --area.mY;
    }

    return removed >= maxBlocks;
}
