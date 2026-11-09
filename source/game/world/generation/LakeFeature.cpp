#include "game/world/generation/LakeFeature.hpp"
#include "game/world/block/BlockId.hpp"

#include <cstring>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/util/Random.hpp"

LakeFeature::LakeFeature(Block* block) : Feature(false), mBlock(block)
{
}

bool LakeFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    BlockPos pos = origin;
    pos = pos.offset(-8, 0, -8);
    while (pos.y > 5 && level->isEmptyBlock(pos))
        pos = pos.below();
    if (pos.y <= 4)
        return false;

    pos = pos.below(4);
    bool fill[2048];
    memset(fill, 0, sizeof(fill));

    if (gApp.getLevelGenerationOptions() != NULL)
    {
        LevelGenerationOptions* levelGenerationOptions = gApp.getLevelGenerationOptions();
        if (levelGenerationOptions->checkIntersects(BlockPos(pos.x, pos.y, pos.z),
                                                    BlockPos(pos.x + 16, pos.y + 8, pos.z + 16)))
            return false;
    }

    int count = random.nextInt(4) + 4;
    for (int i = 0; i < count; ++i)
    {
        double xSize = random.nextDouble() * 6.0 + 3.0;
        double ySize = random.nextDouble() * 4.0 + 2.0;
        double zSize = random.nextDouble() * 6.0 + 3.0;
        double xCenter = random.nextDouble() * (14.0 - xSize) + 1.0 + xSize * 0.5;
        double yCenter = random.nextDouble() * (4.0 - ySize) + 2.0 + ySize * 0.5;
        double zCenter = random.nextDouble() * (14.0 - zSize) + 1.0 + zSize * 0.5;

        for (int x = 1; x < 15; ++x)
        {
            for (int z = 1; z < 15; ++z)
            {
                for (int y = 1; y < 7; ++y)
                {
                    double dx = ((double)x - xCenter) / (xSize * 0.5);
                    double dy = ((double)y - yCenter) / (ySize * 0.5);
                    double dz = ((double)z - zCenter) / (zSize * 0.5);
                    double distSq = dx * dx + dy * dy + dz * dz;
                    if (distSq < 1.0)
                        fill[(x * 16 + z) * 8 + y] = true;
                }
            }
        }
    }

    for (int x = 0; x < 16; ++x)
    {
        for (int z = 0; z < 16; ++z)
        {
            for (int y = 0; y < 8; ++y)
            {
                bool edge =
                    !fill[(x * 16 + z) * 8 + y]
                    && ((x < 15 && fill[((x + 1) * 16 + z) * 8 + y]) || (x > 0 && fill[((x - 1) * 16 + z) * 8 + y])
                        || (z < 15 && fill[(x * 16 + z + 1) * 8 + y]) || (z > 0 && fill[(x * 16 + (z - 1)) * 8 + y])
                        || (y < 7 && fill[(x * 16 + z) * 8 + y + 1]) || (y > 0 && fill[(x * 16 + z) * 8 + (y - 1)]));
                if (!edge)
                    continue;

                Material* material = level->getBlockState(pos.offset(x, y, z))->getMaterial();
                if (y >= 4 && material->isLiquid())
                    return false;
                if (y < 4 && !material->isSolid() && level->getBlockState(pos.offset(x, y, z))->getBlock() != mBlock)
                    return false;
            }
        }
    }

    for (int x = 0; x < 16; ++x)
    {
        for (int z = 0; z < 16; ++z)
        {
            for (int y = 0; y < 8; ++y)
            {
                if (fill[(x * 16 + z) * 8 + y])
                    level->setBlock(pos.offset(x, y, z),
                                    y >= 4 ? Blocks::air->defaultBlockState() : mBlock->defaultBlockState(),
                                    Block::UPDATE_CLIENTS, false);
            }
        }
    }

    for (int x = 0; x < 16; ++x)
    {
        for (int z = 0; z < 16; ++z)
        {
            for (int y = 4; y < 8; ++y)
            {
                if (!fill[(x * 16 + z) * 8 + y])
                    continue;

                BlockPos below = pos.offset(x, y - 1, z);
                if (level->getBlockState(below)->getBlock() == Blocks::dirt
                    && level->getBrightness(LightLayer::SKY, pos.offset(x, y, z)) > 0)
                {
                    Biome* biome = level->getBiome(below);
                    if (biome->mTopBlock.blockId == BlockId::MYCELIUM)
                        level->setBlock(below, Blocks::mycelium->defaultBlockState(), Block::UPDATE_CLIENTS, false);
                    else
                        level->setBlock(below, Blocks::grass->defaultBlockState(), Block::UPDATE_CLIENTS, false);
                }
            }
        }
    }

    if (mBlock->defaultBlockState()->getMaterial() == Material::LAVA)
    {
        for (int x = 0; x < 16; ++x)
        {
            for (int z = 0; z < 16; ++z)
            {
                for (int y = 0; y < 8; ++y)
                {
                    bool edge =
                        !fill[(x * 16 + z) * 8 + y]
                        && ((x < 15 && fill[((x + 1) * 16 + z) * 8 + y]) || (x > 0 && fill[((x - 1) * 16 + z) * 8 + y])
                            || (z < 15 && fill[(x * 16 + z + 1) * 8 + y]) || (z > 0 && fill[(x * 16 + (z - 1)) * 8 + y])
                            || (y < 7 && fill[(x * 16 + z) * 8 + y + 1])
                            || (y > 0 && fill[(x * 16 + z) * 8 + (y - 1)]));
                    if (edge && (y < 4 || random.nextInt(2) != 0)
                        && level->getBlockState(pos.offset(x, y, z))->getMaterial()->isSolid())
                        level->setBlock(pos.offset(x, y, z), Blocks::stone->defaultBlockState(), Block::UPDATE_CLIENTS,
                                        false);
                }
            }
        }
    }

    if (mBlock->defaultBlockState()->getMaterial() == Material::WATER)
    {
        for (int x = 0; x < 16; ++x)
        {
            for (int z = 0; z < 16; ++z)
            {
                if (level->shouldFreezeIgnoreNeighbors(pos.offset(x, 4, z)))
                    level->setBlock(pos.offset(x, 4, z), Blocks::ice->defaultBlockState(), Block::UPDATE_CLIENTS,
                                    false);
            }
        }
    }

    return true;
}
