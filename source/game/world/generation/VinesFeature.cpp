#include "game/world/generation/VinesFeature.hpp"

#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/VineBlock.hpp"

VinesFeature::VinesFeature() : Feature(false)
{
}

bool VinesFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    for (BlockPos position(origin); position.y < 128; position = position.above())
    {
        if (level->isEmptyBlock(position))
        {
            arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();

            for (unsigned int i = 0; i < directions.length; ++i)
            {
                const Direction* direction = directions.data[i];

                if (Blocks::vine->mayPlace(level, position, direction))
                {
                    const BlockState* state = Blocks::vine->defaultBlockState()
                                                  ->setValue(VineBlock::NORTH, direction == Direction::SOUTH)
                                                  ->setValue(VineBlock::EAST, direction == Direction::WEST)
                                                  ->setValue(VineBlock::SOUTH, direction == Direction::NORTH)
                                                  ->setValue(VineBlock::WEST, direction == Direction::EAST);
                    level->setBlock(position, state, Block::UPDATE_CLIENTS, false);
                    break;
                }
            }
        }
        else
        {
            position = position.offset(random.nextInt(4) - random.nextInt(4), 0, random.nextInt(4) - random.nextInt(4));
        }
    }

    return true;
}
