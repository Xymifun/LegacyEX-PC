#include "game/world/generation/CanyonFeature.hpp"
#include "game/world/block/BlockId.hpp"

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/Mth.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/generation/LargeCaveFeature.hpp"

void CanyonFeature::addTunnel(Level* level, long long seed, int chunkX, int chunkZ, ChunkPrimer* primer, double x,
                              double y, double z, float thickness, float yaw, float pitch, int startIndex, int endIndex,
                              double yScale)
{
    BlockPos origin(x, y, z);
    MemSect(49);
    Random random(seed);
    MemSect(0);
    double centerX = chunkX * 16 + 8;
    double centerZ = chunkZ * 16 + 8;
    float yawDelta = 0.0F;
    float pitchDelta = 0.0F;

    if (endIndex <= 0)
    {
        int blockRange = range * 16 - 16;
        endIndex = blockRange - random.nextInt(blockRange / 4);
    }

    bool singleStep = false;
    if (startIndex == -1)
    {
        startIndex = endIndex / 2;
        singleStep = true;
    }

    float widthScale = 1.0F;
    for (int i = 0; i < 128; ++i)
    {
        if (i == 0 || random.nextInt(3) == 0)
            widthScale = 1.0F + random.nextFloat() * random.nextFloat();
        rs[i] = widthScale * widthScale;
    }

    for (; startIndex < endIndex; ++startIndex)
    {
        double horizontalRadius = 1.5
                                  + (double)(Mth::sin((float)startIndex * 3.1415927F / (float)endIndex) * thickness);
        double verticalRadius = horizontalRadius * yScale;
        horizontalRadius *= random.nextFloat() * 0.25 + 0.75;
        verticalRadius *= random.nextFloat() * 0.25 + 0.75;
        float cosPitch = Mth::cos(pitch);
        float sinPitch = Mth::sin(pitch);
        x += (double)(Mth::cos(yaw) * cosPitch);
        y += (double)sinPitch;
        z += (double)(Mth::sin(yaw) * cosPitch);
        yaw += yawDelta * 0.05F;
        pitch = pitch * 0.7F + pitchDelta * 0.05F;
        pitchDelta = pitchDelta * 0.8F + (random.nextFloat() - random.nextFloat()) * random.nextFloat() * 2.0F;
        yawDelta = yawDelta * 0.5F + (random.nextFloat() - random.nextFloat()) * random.nextFloat() * 4.0F;

        if (!singleStep && random.nextInt(4) == 0)
            continue;

        double dx = x - centerX;
        double dz = z - centerZ;
        double remaining = endIndex - startIndex;
        double maxDistance = thickness + 18.0F;
        if (dx * dx + dz * dz - remaining * remaining > maxDistance * maxDistance)
            return;

        if (x < centerX - 16.0 - horizontalRadius * 2.0 || z < centerZ - 16.0 - horizontalRadius * 2.0
            || x > centerX + 16.0 + horizontalRadius * 2.0 || z > centerZ + 16.0 + horizontalRadius * 2.0)
            continue;

        int minX = Mth::floor(x - horizontalRadius) - chunkX * 16 - 1;
        int maxX = Mth::floor(x + horizontalRadius) - chunkX * 16 + 1;
        int minY = Mth::floor(y - verticalRadius) - 1;
        int maxY = Mth::floor(y + verticalRadius) + 1;
        int minZ = Mth::floor(z - horizontalRadius) - chunkZ * 16 - 1;
        int maxZ = Mth::floor(z + horizontalRadius) - chunkZ * 16 + 1;
        if (minX < 0)
            minX = 0;
        if (maxX > 16)
            maxX = 16;
        if (minY < 1)
            minY = 1;
        if (maxY > 120)
            maxY = 120;
        if (minZ < 0)
            minZ = 0;
        if (maxZ > 16)
            maxZ = 16;

        bool foundWater = false;
        for (int bx = minX; !foundWater && bx < maxX; ++bx)
        {
            for (int bz = minZ; !foundWater && bz < maxZ; ++bz)
            {
                for (int by = maxY + 1; !foundWater && by >= minY - 1; --by)
                {
                    if (by >= 0 && by < 128)
                    {
                        const BlockState* state = primer->getState(bx, by, bz);
                        if (state->getBlock() == Blocks::flowing_water || state->getBlock() == Blocks::water)
                            foundWater = true;
                        if (by != minY - 1 && bx != minX && bx != maxX - 1 && bz != minZ && bz != maxZ - 1)
                            by = minY;
                    }
                }
            }
        }
        if (foundWater)
            continue;

        BlockPos biomePos(0, 0, 0);
        int lavaData = Blocks::flowing_lava->convertBlockStateToLegacyData(Blocks::flowing_lava->defaultBlockState());
        bool carved = false;
        for (int bx = minX; bx < maxX; ++bx)
        {
            double xd = ((double)(bx + chunkX * 16) + 0.5 - x) / horizontalRadius;
            for (int bz = minZ; bz < maxZ; ++bz)
            {
                double zd = ((double)(bz + chunkZ * 16) + 0.5 - z) / horizontalRadius;
                double horizontalDistance = zd * zd + xd * xd;
                bool foundTop = false;
                if (horizontalDistance >= 1.0)
                    continue;

                for (int by = maxY - 1; by >= minY; --by)
                {
                    double yd = ((double)by + 0.5 - y) / verticalRadius;
                    int index = (bx << 11) | (bz << 7) | by;
                    if (horizontalDistance * rs[by] + yd * yd / 6.0 >= 1.0)
                        continue;

                    if (primer->getState(bx, by, bz)->getBlock() == Blocks::grass)
                        foundTop = true;
                    int aboveBlockId = primer->getBlockId((bx << 11) | (bz << 7) | (by + 1));
                    int blockId = primer->getBlockId(index);
                    if (!LargeCaveFeature::canReplaceBlock(blockId, aboveBlockId))
                        continue;

                    if (!carved && level != NULL && level->getBiome(origin)->getBiomeType() == Biome::eBiomeType_Ocean)
                        return;
                    carved = true;

                    if (by - 1 < 10)
                    {
                        primer->setBlockAndData(index, BlockId::FLOWING_LAVA, lavaData);
                    }
                    else
                    {
                        primer->setBlockAndData(index, BlockId::AIR, 0);
                        if (foundTop && primer->getState(bx, by - 1, bz)->getBlock() == Blocks::dirt)
                        {
                            biomePos.y = 0;
                            biomePos.x = bx + chunkX * 16;
                            biomePos.z = bz + chunkZ * 16;
                            Biome* biome = level->getBiome(biomePos);
                            primer->setBlockAndData(index - 1, biome->mTopBlock.blockId, biome->mTopBlock.blockData);
                        }
                    }
                }
            }
        }

        if (singleStep)
            break;
    }
}

void CanyonFeature::addFeature(Level* level, int chunkX, int chunkZ, int originChunkX, int originChunkZ,
                               ChunkPrimer* primer)
{
    if (random.nextInt(50) != 0)
        return;

    double x = chunkX * 16 + random.nextInt(16);
    double y = random.nextInt(random.nextInt(40) + 8) + 20;
    double z = chunkZ * 16 + random.nextInt(16);
    int tunnels = 1;
    for (int i = 0; i < tunnels; ++i)
    {
        float yaw = random.nextFloat() * 3.1415927F * 2.0F;
        float pitch = (random.nextFloat() - 0.5F) * 2.0F / 8.0F;
        float thickness = (random.nextFloat() * 2.0F + random.nextFloat()) * 2.0F;
        addTunnel(level, random.nextLong(), originChunkX, originChunkZ, primer, x, y, z, thickness, yaw, pitch, 0, 0,
                  3.0);
    }
    gApp.AddTerrainFeaturePosition((_eTerrainFeatureType)4, x / 16, y / 16);
}
