#include "game/world/generation/UnderwaterCanyonFeature.hpp"
#include "game/world/block/BlockId.hpp"

#include <cmath>

#include "game/core/MemSect.hpp"
#include "game/core/Mth.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/generation/UnderwaterLargeCaveFeature.hpp"

void UnderwaterCanyonFeature::apply(Level* level, int chunkX, int chunkZ, ChunkPrimer* primer)
{
    random.setSeed(level->getSeed());
    int xStep = random.nextInt() / 2 * 2 + 1;
    int zStep = random.nextInt() / 2 * 2 + 1;
    for (int x = chunkX - 8; x <= chunkX + 8; ++x)
    {
        for (int z = chunkZ - 8; z <= chunkZ + 8; ++z)
        {
            int seedOffset = x * xStep + z * zStep;
            random.setSeed(seedOffset ^ level->getSeed());
            addFeature(level, x, z, chunkX, chunkZ, primer);
        }
    }
}

void UnderwaterCanyonFeature::addTunnel(Level* level, long long seed, int chunkX, int chunkZ, ChunkPrimer* primer,
                                        double x, double y, double z, float thickness, float yaw, float pitch,
                                        int startIndex, int endIndex, double yScale)
{
    MemSect(49);
    Random random(seed);
    MemSect(0);
    float centerX = (float)(chunkX * 16 + 8);
    float centerZ = (float)(chunkZ * 16 + 8);
    float pitchDelta = 0.0F;
    float yawDelta = 0.0F;
    BlockPos origin(x, y, z);

    if (endIndex < 1)
        endIndex = 112 - random.nextInt(28);

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
        float radius = sinf((float)startIndex * 3.1415927F / (float)endIndex) * thickness + 1.5F;
        float horizontalRadius = radius * (random.nextFloat() * 0.25F + 0.75F);
        float verticalRadius = radius * (float)yScale * (random.nextFloat() * 0.25F + 0.75F);
        float cosPitch = cosf(pitch);
        float sinPitch = sinf(pitch);
        x += cosf(yaw) * cosPitch;
        y += sinPitch;
        z += sinf(yaw) * cosPitch;
        yaw += yawDelta * 0.05F;
        pitch = pitch * 0.7F + pitchDelta * 0.05F;
        float change = random.nextGaussianFloat();
        change *= random.nextFloat();
        pitchDelta = pitchDelta * 0.8F + change * 2.0F;
        change = random.nextGaussianFloat();
        change *= random.nextFloat();
        yawDelta = yawDelta * 0.5F + change * 4.0F;

        if (singleStep || random.nextInt(4) != 0)
        {
            float remaining = (float)(endIndex - startIndex);
            float maxDistance = thickness + 18.0F;
            float dx = (float)x - centerX;
            float dz = (float)z - centerZ;
            if (dx * dx + dz * dz - remaining * remaining > maxDistance * maxDistance)
                return;

            float doubleRadius = horizontalRadius + horizontalRadius;
            if (x >= centerX - 16.0F - doubleRadius && z >= centerZ - 16.0F - doubleRadius
                && x <= centerX + 16.0F + doubleRadius && z <= centerZ + 16.0F + doubleRadius)
            {
                int minX = (int)floor(x - horizontalRadius) - chunkX * 16 - 1;
                int maxX = (int)floor(x + horizontalRadius) - chunkX * 16 + 1;
                double bottom = floor(y - verticalRadius);
                double top = floor(y + verticalRadius);
                int minZ = (int)floor(z - horizontalRadius) - chunkZ * 16 - 1;
                int maxZ = (int)floor(z + horizontalRadius) - chunkZ * 16 + 1;
                if (minX < 0)
                    minX = 0;
                if (maxX > 16)
                    maxX = 16;
                int maxY = Mth::clamp((int)top + 1, 1, 120);
                int minY = Mth::clamp((int)bottom - 1, 1, maxY);
                if (minZ < 0)
                    minZ = 0;
                if (maxZ > 16)
                    maxZ = 16;

                int magmaData = Blocks::magma->convertBlockStateToLegacyData(Blocks::magma->defaultBlockState());
                int obsidianData = Blocks::obsidian->convertBlockStateToLegacyData(
                    Blocks::obsidian->defaultBlockState());
                int flowingWaterData = Blocks::flowing_water->convertBlockStateToLegacyData(
                    Blocks::flowing_water->defaultBlockState());
                int waterData = Blocks::water->convertBlockStateToLegacyData(Blocks::water->defaultBlockState());
                bool carved = false;
                for (int localX = minX; localX < maxX; ++localX)
                {
                    float xd = ((float)(localX + chunkX * 16) + 0.5F - (float)x) / horizontalRadius;
                    for (int localZ = minZ; localZ < maxZ; ++localZ)
                    {
                        float zd = ((float)(localZ + chunkZ * 16) + 0.5F - (float)z) / horizontalRadius;
                        float horizontalDistance = zd * zd + xd * xd;
                        if (horizontalDistance < 1.0F)
                        {
                            for (int localY = maxY - 1; localY >= minY; --localY)
                            {
                                float yd = ((float)localY + 0.5F - (float)y) / verticalRadius;
                                int index = (localX << 11) | (localZ << 7) | localY;
                                if (horizontalDistance * rs[localY - 1] + yd * yd / 6.0F < 1.0F && localY < 62)
                                {
                                    if (UnderwaterLargeCaveFeature::isDiggable(primer->getBlockId(index)))
                                    {
                                        if (!carved && level != NULL
                                            && level->getBiome(origin)->getBiomeType() != Biome::eBiomeType_Ocean)
                                            return;
                                        carved = true;

                                        if (localY < 10)
                                        {
                                            if (random.nextFloat() < 0.25F)
                                                primer->setBlockAndData(index, BlockId::MAGMA, magmaData);
                                            else
                                                primer->setBlockAndData(index, BlockId::OBSIDIAN, obsidianData);
                                        }
                                        else
                                        {
                                            bool water = false;
                                            if (localX < 15)
                                            {
                                                int neighbor = ((localX << 11) + 0x800) | (localZ << 7) | localY;
                                                if (primer->getBlockId(neighbor) == BlockId::AIR)
                                                {
                                                    primer->setBlockAndData(neighbor, BlockId::FLOWING_WATER,
                                                                            flowingWaterData);
                                                    water = true;
                                                }
                                            }
                                            if (localX > 0)
                                            {
                                                int neighbor = ((localX << 11) - 0x800) | (localZ << 7) | localY;
                                                if (primer->getBlockId(neighbor) == BlockId::AIR)
                                                {
                                                    primer->setBlockAndData(neighbor, BlockId::FLOWING_WATER,
                                                                            flowingWaterData);
                                                    water = true;
                                                }
                                            }
                                            if (localZ < 15)
                                            {
                                                int neighbor = (localX << 11) | ((localZ << 7) + 0x80) | localY;
                                                if (primer->getBlockId(neighbor) == BlockId::AIR)
                                                {
                                                    primer->setBlockAndData(neighbor, BlockId::FLOWING_WATER,
                                                                            flowingWaterData);
                                                    water = true;
                                                }
                                            }
                                            if (localZ > 0)
                                            {
                                                int neighbor = (localX << 11) | ((localZ << 7) - 0x80) | localY;
                                                if (primer->getBlockId(neighbor) == BlockId::AIR)
                                                {
                                                    primer->setBlockAndData(neighbor, BlockId::FLOWING_WATER,
                                                                            flowingWaterData);
                                                    water = true;
                                                }
                                            }
                                            if (!water)
                                                primer->setBlockAndData(index, BlockId::WATER, waterData);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                if (singleStep)
                    break;
            }
        }
    }
}

void UnderwaterCanyonFeature::addFeature(Level* level, int chunkX, int chunkZ, int originChunkX, int originChunkZ,
                                         ChunkPrimer* primer)
{
    if (random.nextInt(20) != 0)
        return;

    double x = chunkX * 16 + random.nextInt(16);
    double y = random.nextInt(random.nextInt(40) + 8) + 20;
    if ((float)random.nextInt() <= 0.2F)
        y += 15.0;
    double z = chunkZ * 16 + random.nextInt(16);
    int tunnels = 1;
    for (int i = 0; i < tunnels; ++i)
    {
        float yaw = random.nextFloat() * 3.1415927F * 2.0F;
        float pitch = (random.nextFloat() - 0.5F) * 2.0F / 8.0F;
        float firstWidth = random.nextFloat() * 3.0F;
        float thickness = random.nextFloat() * 3.0F + firstWidth;
        if (random.nextFloat() < 0.05F)
            thickness *= 2.0F;
        addTunnel(level, random.nextLong(), originChunkX, originChunkZ, primer, x, y, z, thickness, yaw, pitch, 0, 0,
                  3.0);
    }
}
