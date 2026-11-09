#include "game/world/generation/IcePatchFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/MutableBlockPos.hpp"

IcePatchFeature::IcePatchFeature(int radius) : Feature(false), mBlock(Blocks::packed_ice), mRadius(radius)
{
}

IcePatchFeature::~IcePatchFeature()
{
}

bool IcePatchFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    BlockPos position = origin;
    while (level->isEmptyBlock(position) && position.y > 2)
    {
        position = position.below();
    }

    if (level->getBlockState(position)->getBlock() != Blocks::snow)
    {
        return false;
    }

    const int radius = random.nextInt(mRadius - 2) + 2;
    MutableBlockPos target(0, 0, 0);
    for (int x = position.x - radius; x <= position.x + radius; ++x)
    {
        for (int z = position.z - radius; z <= position.z + radius; ++z)
        {
            if ((x - position.x) * (x - position.x) + (z - position.z) * (z - position.z) > radius * radius)
            {
                continue;
            }

            for (int y = position.y - 1; y <= position.y + 1; ++y)
            {
                target.set(x, y, z);
                Block* block = level->getBlockState(target)->getBlock();
                if (block == Blocks::dirt || block == Blocks::snow || block == Blocks::ice)
                {
                    level->setBlock(target, mBlock->defaultBlockState(), Block::UPDATE_CLIENTS, false);
                }
            }
        }
    }

    return true;
}
