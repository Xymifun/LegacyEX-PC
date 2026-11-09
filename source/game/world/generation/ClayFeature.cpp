#include "game/world/generation/ClayFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"

ClayFeature::ClayFeature(int numberOfBlocks) : Feature(false), mClay(Blocks::clay), mNumberOfBlocks(numberOfBlocks)
{
}

ClayFeature::~ClayFeature()
{
}

bool ClayFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    if (level->getBlockState(origin)->getBlock()->mMaterial != Material::WATER)
    {
        return false;
    }

    const int radius = random.nextInt(mNumberOfBlocks - 2) + 2;
    const int yRadius = 1;

    for (int x = origin.x - radius; x <= origin.x + radius; ++x)
    {
        for (int z = origin.z - radius; z <= origin.z + radius; ++z)
        {
            const int xd = x - origin.x;
            const int zd = z - origin.z;
            if (xd * xd + zd * zd <= radius * radius)
            {
                for (int y = origin.y - yRadius; y <= origin.y + yRadius; ++y)
                {
                    const BlockPos& pos = BlockPos(x, y, z);
                    Block* block = level->getBlockState(pos)->getBlock();
                    if (block == Blocks::dirt || block == Blocks::clay)
                    {
                        level->setBlock(pos, mClay->defaultBlockState(), Block::UPDATE_CLIENTS, false);
                    }
                }
            }
        }
    }

    return true;
}
