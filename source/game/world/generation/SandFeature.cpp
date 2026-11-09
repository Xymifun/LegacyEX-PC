#include "game/world/generation/SandFeature.hpp"

#include "game/util/Random.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MutableBlockPos.hpp"


SandFeature::SandFeature(Block* block, int numberOfBlocks)
    : Feature(false),
      mBlock(block),
      mNumberOfBlocks(numberOfBlocks)
{
}

SandFeature::~SandFeature()
{
}

bool SandFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    if (level->getBlockState(origin)->getMaterial() != Material::WATER)
    {
        return false;
    }

    Level::setInstaTick(true);
    const int radius = random.nextInt(mNumberOfBlocks - 2) + 2;

    if (gApp.getLevelGenerationOptions() != NULL)
    {
        LevelGenerationOptions* levelGenerationOptions = gApp.getLevelGenerationOptions();
        if (levelGenerationOptions->checkIntersects(origin.offset(-radius, -2, -radius),
                                                    origin.offset(radius, 2, radius)))
        {
            Level::setInstaTick(false);
            return false;
        }
    }

    MutableBlockPos position(0, 0, 0);
    for (int x = origin.x - radius; x <= origin.x + radius; ++x)
    {
        for (int z = origin.z - radius; z <= origin.z + radius; ++z)
        {
            int xd = x - origin.x;
            int zd = z - origin.z;
            if (xd * xd + zd * zd <= radius * radius)
            {
                for (int y = origin.y - 2; y <= origin.y + 2; ++y)
                {
                    position.set(x, y, z);
                    Block* block = level->getBlockState(position)->getBlock();
                    if (block == Blocks::dirt || block == Blocks::grass)
                    {
                        level->setBlock(position, mBlock->defaultBlockState(), Block::UPDATE_CLIENTS, false);
                    }
                }
            }
        }
    }

    Level::setInstaTick(false);
    return true;
}
