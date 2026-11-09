#include "game/world/generation/LightGemFeature.hpp"

#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"

bool LightGemFeature::place(Level* level, Random& random, const BlockPos& origin)
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
        const int dx = random.nextInt(8) - random.nextInt(8);
        const int dy = -random.nextInt(12);
        const int dz = random.nextInt(8) - random.nextInt(8);
        const BlockPos position = origin.offset(dx, dy, dz);
        if (level->getBlockState(position)->getMaterial() == Material::AIR)
        {
            int glowstoneNeighbours = 0;
            std::vector<const Direction*>::const_iterator it = Direction::VALUES.begin();
            while (it != Direction::VALUES.end())
            {
                if (level->getBlockState(position.relative(*it))->getBlock() == Blocks::glowstone)
                {
                    ++glowstoneNeighbours;
                    if (glowstoneNeighbours > 1)
                    {
                        break;
                    }
                }
                ++it;
            }

            if (glowstoneNeighbours == 1)
            {
                level->setBlock(position, Blocks::glowstone->defaultBlockState(), Block::UPDATE_CLIENTS, false);
            }
        }
    }

    return true;
}
