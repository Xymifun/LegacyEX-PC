#include "game/world/generation/GroundBushFeature.hpp"

#include <cstdlib>

#include "game/core/PIXNamedEvents.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"

GroundBushFeature::GroundBushFeature(const BlockState* trunkState, const BlockState* leafState) : TreeFeature(false)
{
    mTrunkState = trunkState;
    mLeafState = leafState;
}

bool GroundBushFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    PIXBeginNamedEvent(0.0f, "Placing GroundBushFeature");

    BlockPos position = origin;
    const BlockState* state = level->getBlockState(position);
    while ((state->getMaterial() == Material::AIR || state->getMaterial() == Material::LEAVES) && position.y > 0)
    {
        position = position.below();
        state = level->getBlockState(position);
    }

    Block* ground = level->getBlockState(position)->getBlock();
    if (ground == Blocks::dirt || ground == Blocks::grass)
    {
        position = position.above();
        setBlock(level, position, mTrunkState, Block::UPDATE_NONE);

        for (int y = position.y; y <= position.y + 2; ++y)
        {
            const int radius = 2 - (y - position.y);
            for (int x = position.x - radius; x <= position.x + radius; ++x)
            {
                const int dx = std::abs(x - position.x);
                for (int z = position.z - radius; z <= position.z + radius; ++z)
                {
                    if (dx != radius || std::abs(z - position.z) != radius || random.nextInt(2) != 0)
                    {
                        const BlockPos& leafPos = BlockPos(x, y, z);
                        const Material* material = level->getBlockState(leafPos)->getMaterial();
                        if (material == Material::AIR || material == Material::LEAVES)
                        {
                            setBlock(level, leafPos, mLeafState, Block::UPDATE_NONE);
                        }
                    }
                }
            }
        }
    }

    PIXEndNamedEvent();
    return true;
}
