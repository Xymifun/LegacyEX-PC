#include "game/world/generation/EndIslandFeature.hpp"

#include <cmath>

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"

bool EndIslandFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    float radius = (float)random.nextInt(3) + 4.0f;
    int y = 0;

    for (; radius > 0.5f; radius -= (float)random.nextInt(2) + 0.5f, --y)
    {
        for (int x = (int)floorf(-radius); x <= (int)ceilf(radius); ++x)
        {
            for (int z = (int)floorf(-radius); z <= (int)ceilf(radius); ++z)
            {
                if ((float)(x * x + z * z) <= (radius + 1.0f) * (radius + 1.0f))
                {
                    setBlock(level, origin.offset(x, y, z), Blocks::end_stone->defaultBlockState(), Block::UPDATE_NONE);
                }
            }
        }
    }

    return true;
}
