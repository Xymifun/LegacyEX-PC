#include "game/world/generation/EndGatewayFeature.hpp"

#include <cstdlib>

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EndGatewayBlock.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/TypedBoxed.hpp"

EndGatewayFeature::EndGatewayFeature(int portalNumber) : Feature(false), mPortalNumber(portalNumber)
{
}

bool EndGatewayFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    MutableBlockPos position(0, 0, 0);
    for (int x = origin.x - 1; x <= origin.x + 1; ++x)
    {
        for (int z = origin.z - 1; z <= origin.z + 1; ++z)
        {
            for (int y = origin.y - 2; y <= origin.y + 2; ++y)
            {
                position.set(x, y, z);
                const bool atX = x == origin.x;
                const bool atY = y == origin.y;
                const bool atZ = z == origin.z;
                const bool edgeY = std::abs(y - origin.y) == 2;
                if (atX && atY && atZ)
                {
                    const BlockState* state = Blocks::end_gateway->defaultBlockState()->setValue(
                        EndGatewayBlock::PORTAL_NUMBER, mPortalNumber);
                    setBlock(level, BlockPos(position), state, Block::UPDATE_NONE);
                }
                else if (atY)
                {
                    setBlock(level, position, Blocks::air->defaultBlockState(), Block::UPDATE_NONE);
                }
                else if (edgeY && atX && atZ)
                {
                    setBlock(level, position, Blocks::bedrock->defaultBlockState(), Block::UPDATE_NONE);
                }
                else if ((!atX && !atZ) || edgeY)
                {
                    setBlock(level, position, Blocks::air->defaultBlockState(), Block::UPDATE_NONE);
                }
                else
                {
                    setBlock(level, position, Blocks::bedrock->defaultBlockState(), Block::UPDATE_NONE);
                }
            }
        }
    }

    return true;
}
