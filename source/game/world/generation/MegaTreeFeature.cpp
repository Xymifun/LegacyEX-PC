#include "game/world/generation/MegaTreeFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"

MegaTreeFeature::MegaTreeFeature(bool doBlockNotify, int baseHeight, int extraRandomHeight,
                                 const BlockState* trunkState, const BlockState* leafState)
    : AbstractTreeFeature(doBlockNotify, baseHeight, trunkState, leafState, false),
      mExtraRandomHeight(extraRandomHeight)
{
}

int MegaTreeFeature::calcTreeHeigth(Random& random)
{
    int height = random.nextInt(3) + mBaseHeight;
    if (mExtraRandomHeight > 1)
    {
        height += random.nextInt(mExtraRandomHeight);
    }
    return height;
}

bool MegaTreeFeature::checkIsFree(Level* level, const BlockPos& pos, int height)
{
    bool free = true;
    if (pos.y < 1 || pos.y + height + 1 > 256)
    {
        return false;
    }

    for (int y = 0; y <= height + 1; ++y)
    {
        int radius = 2;
        if (y == 0)
        {
            radius = 1;
        }
        else if (y >= height - 1)
        {
            radius = 2;
        }
        for (int x = -radius; x <= radius && free; ++x)
        {
            for (int z = -radius; z <= radius && free; ++z)
            {
                if (pos.y + y < 0 || pos.y + y >= 256 || !isFree(level->getBlockState(pos.offset(x, y, z))->getBlock()))
                {
                    free = false;
                }
            }
        }
    }

    return free;
}

bool MegaTreeFeature::makeDirtFloor(const BlockPos& pos, Level* level)
{
    const BlockPos& below = pos.below();
    Block* block = level->getBlockState(below)->getBlock();
    if ((block != Blocks::grass && block != Blocks::dirt) || pos.y < 2)
    {
        return false;
    }
    setDirtAt(level, below);
    setDirtAt(level, below.east());
    setDirtAt(level, below.south());
    setDirtAt(level, below.south().east());
    return true;
}

bool MegaTreeFeature::prepareTree(Level* level, Random&, const BlockPos& pos, int height)
{
    return checkIsFree(level, pos, height) && makeDirtFloor(pos, level);
}

void MegaTreeFeature::placeDoubleTrunkLeaves(Level* level, const BlockPos& pos, int radius)
{
    int radiusSquared = radius * radius;
    for (int x = -radius; x <= radius + 1; ++x)
    {
        int x1 = x - 1;
        int x1x1 = x1 * x1;
        int xx = x * x;
        for (int z = -radius; z <= radius + 1; ++z)
        {
            int z1 = z - 1;
            if (xx + z * z <= radiusSquared || x1x1 + z1 * z1 <= radiusSquared || xx + z1 * z1 <= radiusSquared
                || x1x1 + z * z <= radiusSquared)
            {
                const BlockPos& leafPos = pos.offset(x, 0, z);
                Material* material = level->getBlockState(leafPos)->getMaterial();
                if (material == Material::AIR || material == Material::LEAVES)
                {
                    setBlock(level, leafPos, mLeafState, Block::UPDATE_NONE);
                }
            }
        }
    }
}

void MegaTreeFeature::placeSingleTrunkLeaves(Level* level, const BlockPos& pos, int radius)
{
    int radiusSquared = radius * radius;
    for (int x = -radius; x <= radius; ++x)
    {
        for (int z = -radius; z <= radius; ++z)
        {
            if (x * x + z * z <= radiusSquared)
            {
                const BlockPos& leafPos = pos.offset(x, 0, z);
                Material* material = level->getBlockState(leafPos)->getMaterial();
                if (material == Material::AIR || material == Material::LEAVES)
                {
                    setBlock(level, leafPos, mLeafState, Block::UPDATE_NONE);
                }
            }
        }
    }
}
