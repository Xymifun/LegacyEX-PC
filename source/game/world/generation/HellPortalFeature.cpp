#include "game/world/generation/HellPortalFeature.hpp"

#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"

HellPortalFeature::~HellPortalFeature()
{
}

bool HellPortalFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    if (!level->isEmptyBlock(origin))
    {
        return false;
    }

    if (level->getBlockState(origin.above())->getBlock() != Blocks::netherrack)
    {
        return false;
    }

    level->setBlock(origin, Blocks::glowstone->defaultBlockState(), Block::UPDATE_CLIENTS, false);

    for (int i = 0; i < 1500; ++i)
    {
        const BlockPos position = origin.offset(random.nextInt(8) - random.nextInt(8), -random.nextInt(12),
                                                random.nextInt(8) - random.nextInt(8));
        if (level->getBlockState(position)->getBlock()->mMaterial == Material::AIR)
        {
            int glowstoneNeighbours = 0;
            for (std::vector<const Direction*>::const_iterator it = Direction::VALUES.begin();
                 it != Direction::VALUES.end(); ++it)
            {
                if (level->getBlockState(position.relative(*it))->getBlock() == Blocks::glowstone)
                {
                    ++glowstoneNeighbours;
                    if (glowstoneNeighbours > 1)
                    {
                        break;
                    }
                }
            }

            if (glowstoneNeighbours == 1)
            {
                level->setBlock(position, Blocks::glowstone->defaultBlockState(), Block::UPDATE_CLIENTS, false);
            }
        }
    }

    return true;
}
