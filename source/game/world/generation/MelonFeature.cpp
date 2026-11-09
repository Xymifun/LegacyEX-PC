#include "game/world/generation/MelonFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"

MelonFeature::MelonFeature() : Feature(false)
{
}

MelonFeature::~MelonFeature()
{
}

bool MelonFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    for (int i = 0; i < 64; ++i)
    {
        BlockPos position = origin.offset(random.nextInt(8) - random.nextInt(8), random.nextInt(4) - random.nextInt(4),
                                          random.nextInt(8) - random.nextInt(8));
        if (Blocks::melon_block->mayPlace(level, position)
            && level->getBlockState(position.below())->getBlock() == Blocks::grass)
        {
            level->setBlock(position, Blocks::melon_block->defaultBlockState(), Block::UPDATE_CLIENTS, false);
        }
    }

    return true;
}
