#include "game/world/generation/ReedsFeature.hpp"

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

ReedsFeature::~ReedsFeature()
{
}

bool ReedsFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    for (int attempt = 0; attempt < 20; ++attempt)
    {
        const int xOffsetStart = random.nextInt(4);
        const int dx = xOffsetStart - random.nextInt(4);
        const int zOffsetStart = random.nextInt(4);
        const int dz = zOffsetStart - random.nextInt(4);
        const BlockPos position = origin.offset(dx, 0, dz);
        if (gApp.getLevelGenerationOptions() != NULL
            && gApp.getLevelGenerationOptions()->checkIntersects(position, position))
        {
            continue;
        }

        if (level->isEmptyBlock(position))
        {
            const BlockPos below = position.below();
            if (level->getBlockState(below.west())->getMaterial() != Material::WATER
                && level->getBlockState(below.east())->getMaterial() != Material::WATER
                && level->getBlockState(below.north())->getMaterial() != Material::WATER
                && level->getBlockState(below.south())->getMaterial() != Material::WATER)
            {
                continue;
            }

            const int height = random.nextInt(random.nextInt(3) + 1) + 2;
            for (int y = 0; y < height; ++y)
            {
                if (Blocks::reeds->canSurvive(level, position, NULL))
                {
                    level->setBlock(position.above(y), Blocks::reeds->defaultBlockState(), Block::UPDATE_CLIENTS,
                                    false);
                }
            }
        }
    }

    return true;
}
