#include "game/world/biome/VoidBiomeDecorator.hpp"

#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/Level.hpp"

void VoidBiomeDecorator::decorate(Level* level, Random* random, Biome* biome, const BlockPos& origin)
{
    BlockPos spawn = level->getSharedSpawnPos();
    BlockPos offsetOrigin = origin.offset(8, spawn.y, 8);

    if (spawn.distSqr(offsetOrigin) <= 1024.0)
    {
        BlockPos min(spawn.x - 16, spawn.y - 1, spawn.z - 16);
        BlockPos max(spawn.x + 16, spawn.y - 1, spawn.z + 16);
        MutableBlockPos position(spawn.x - 16, spawn.y - 1, spawn.z - 16);

        for (int z = origin.z; z < origin.z + 16; ++z)
        {
            for (int x = origin.x; x < origin.x + 16; ++x)
            {
                if (z >= min.z && z <= max.z && x >= min.x && x <= max.x)
                {
                    position.set(x, position.getY(), z);
                    if (spawn.x == x && spawn.z == z)
                    {
                        level->setBlock(position, Blocks::cobblestone->defaultBlockState(), Block::UPDATE_CLIENTS,
                                        false);
                    }
                    else
                    {
                        level->setBlock(position, Blocks::stone->defaultBlockState(), Block::UPDATE_CLIENTS, false);
                    }
                }
            }
        }
    }
}
