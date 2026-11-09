#include "game/world/generation/CactusFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"

CactusFeature::~CactusFeature()
{
}

bool CactusFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    for (int attempt = 0; attempt < 10; ++attempt)
    {
        const BlockPos position = origin.offset(random.nextInt(8) - random.nextInt(8),
                                                random.nextInt(4) - random.nextInt(4),
                                                random.nextInt(8) - random.nextInt(8));
        if (level->isEmptyBlock(position))
        {
            const int height = random.nextInt(random.nextInt(3) + 1) + 1;
            for (int y = 0; y < height; ++y)
            {
                if (Blocks::cactus->canSurvive(level, position, NULL))
                    level->setBlock(position.above(y), Blocks::cactus->defaultBlockState(), Block::UPDATE_CLIENTS,
                                    false);
            }
        }
    }

    return true;
}
