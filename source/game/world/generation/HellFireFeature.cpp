#include "game/world/generation/HellFireFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"

bool HellFireFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    for (int i = 0; i < 64; ++i)
    {
        BlockPos position = origin.offset(random.nextInt(8) - random.nextInt(8), random.nextInt(4) - random.nextInt(4),
                                          random.nextInt(8) - random.nextInt(8));
        if (level->isEmptyBlock(position))
        {
            if (level->getBlockState(position.below())->getBlock() == Blocks::netherrack)
            {
                level->setBlock(position, Blocks::fire->defaultBlockState(), Block::UPDATE_CLIENTS, false);
            }
        }
    }
    return true;
}
