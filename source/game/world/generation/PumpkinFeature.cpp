#include "game/world/generation/PumpkinFeature.hpp"

#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

PumpkinFeature::~PumpkinFeature()
{
}

bool PumpkinFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    for (int i = 0; i < 64; ++i)
    {
        BlockPos position = origin.offset(random.nextInt(8) - random.nextInt(8), random.nextInt(4) - random.nextInt(4),
                                          random.nextInt(8) - random.nextInt(8));
        if (level->isEmptyBlock(position) && level->getBlockState(position.below())->getBlock() == Blocks::grass
            && Blocks::pumpkin->mayPlace(level, position))
        {
            level->setBlock(position,
                            Blocks::pumpkin->defaultBlockState()->setValue(
                                HorizontalDirectionalBlock::FACING_PROPERTY,
                                Direction::Plane::HORIZONTAL->getRandomDirection(random)),
                            Block::UPDATE_CLIENTS, false);
        }
    }

    return true;
}
