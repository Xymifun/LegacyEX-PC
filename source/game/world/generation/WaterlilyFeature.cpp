#include "game/world/generation/WaterlilyFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"

WaterlilyFeature::~WaterlilyFeature()
{
}

bool WaterlilyFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    for (int attempt = 0; attempt < 10; ++attempt)
    {
        int x = origin.x + random.nextInt(8) - random.nextInt(8);
        int y = origin.y + random.nextInt(4) - random.nextInt(4);
        int z = origin.z + random.nextInt(8) - random.nextInt(8);
        if (level->isEmptyBlock(BlockPos(x, y, z)) && Blocks::waterlily->mayPlace(level, BlockPos(x, y, z)))
        {
            level->setBlock(BlockPos(x, y, z), Blocks::waterlily->defaultBlockState(), Block::UPDATE_CLIENTS, false);
        }
    }

    return true;
}
