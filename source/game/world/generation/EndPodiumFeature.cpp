#include "game/world/generation/EndPodiumFeature.hpp"

#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Iterator.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/TorchBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/DirectionProperty.hpp"

const BlockPos EndPodiumFeature::END_PODIUM_LOCATION(0, 62, 0);

EndPodiumFeature::EndPodiumFeature(bool active) : Feature(false), mActive(active)
{
}

bool EndPodiumFeature::place(Level* level, Random&, const BlockPos& origin)
{
    Iterator<MutableBlockPos*>* positions = BlockPos::mutablesBetween(
        BlockPos(origin.x - 4, origin.y - 1, origin.z - 4), BlockPos(origin.x + 4, origin.y + 32, origin.z + 4));

    while (positions->hasNext())
    {
        MutableBlockPos* position = positions->next();
        double distance = position->dist(origin.x, position->y, origin.z);
        if (distance <= 3.5)
        {
            if (position->y < origin.y)
            {
                if (distance <= 2.5)
                    setBlock(level, *position, Blocks::bedrock->defaultBlockState(), Block::UPDATE_NONE);
                else if (position->y < origin.y)
                    setBlock(level, *position, Blocks::end_stone->defaultBlockState(), Block::UPDATE_NONE);
            }
            else if (position->y > origin.y)
            {
                setBlock(level, *position, Blocks::air->defaultBlockState(), Block::UPDATE_NONE);
            }
            else if (distance > 2.5)
            {
                setBlock(level, *position, Blocks::bedrock->defaultBlockState(), Block::UPDATE_NONE);
            }
            else if (mActive)
            {
                setBlock(level, *position, Blocks::end_portal->defaultBlockState(), Block::UPDATE_NONE);
            }
            else
            {
                setBlock(level, *position, Blocks::air->defaultBlockState(), Block::UPDATE_NONE);
            }
        }
    }

    for (int height = 0; height < 4; ++height)
        setBlock(level, origin.above(height), Blocks::bedrock->defaultBlockState(), Block::UPDATE_NONE);

    BlockPos torchCenter = origin.above(2);
    arrayWithLength<const Direction*> faces = Direction::Plane::HORIZONTAL->getFaces();
    for (unsigned int i = 0; i < faces.length; ++i)
    {
        const Direction* direction = faces.data[i];
        setBlock(level, torchCenter.relative(direction),
                 Blocks::torch->defaultBlockState()->setValue<const Direction*>(TorchBlock::FACING, direction),
                 Block::UPDATE_NONE);
    }

    return true;
}
