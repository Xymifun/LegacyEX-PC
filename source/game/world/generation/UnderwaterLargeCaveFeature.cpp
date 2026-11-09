#include "game/world/generation/UnderwaterLargeCaveFeature.hpp"
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

bool UnderwaterLargeCaveFeature::isDiggable(int blockId)
{
    return blockId == BlockId::AIR || blockId == BlockId::STONE || blockId == BlockId::GRASS || blockId == BlockId::DIRT
           || blockId == BlockId::FLOWING_WATER || blockId == BlockId::WATER || blockId == BlockId::FLOWING_LAVA
           || blockId == BlockId::LAVA || blockId == BlockId::SAND || blockId == BlockId::GRAVEL
           || blockId == BlockId::SANDSTONE || blockId == BlockId::OBSIDIAN || blockId == BlockId::MYCELIUM
           || blockId == BlockId::STAINED_HARDENED_CLAY || blockId == BlockId::HARDENED_CLAY
           || blockId == BlockId::RED_SANDSTONE;
}

void UnderwaterLargeCaveFeature::apply(Level* level, int chunkX, int chunkZ, ChunkPrimer* primer)
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

void UnderwaterLargeCaveFeature::addTunnel(Level* level, long long seed, int chunkX, int chunkZ, ChunkPrimer* primer,
                                           double x, double y, double z, float width, float yaw, float pitch,
                                           int originChunkX, int originChunkZ, double scale)
{
    const float originX = static_cast<float>(chunkX * 16 + 8);
    const float originZ = static_cast<float>(chunkZ * 16 + 8);
    float yawVariance = 0.0F;
    float pitchVariance = 0.0F;
    const BlockPos startPos(x, y, z);
    MemSect(49);
    Random tunnelRandom(seed);
    MemSect(0);
    int length = originChunkZ;
    if (length < 1)
        length = 0x70 - tunnelRandom.nextInt(0x1c);
    int start = originChunkX;
    bool room = false;
    if (start == -1)
    {
        start = length / 2;
        room = true;
    }
    const int branchPoint = tunnelRandom.nextInt(length / 2) + length / 4;

    const float pi = 3.1415927F;
    const float halfPi = 1.5707964F;
    const bool narrowing = tunnelRandom.nextInt(6) == 0;
    for (int step = start; step < length; ++step)
    {
        const float radius = std::sin(static_cast<float>(step) * pi / static_cast<float>(length)) * width + 1.5F;
        const float radiusY = radius * static_cast<float>(scale);
        const float cosPitch = std::cos(pitch);
        const float sinPitch = std::sin(pitch);
        x += std::cos(yaw) * cosPitch;
        y += sinPitch;
        z += std::sin(yaw) * cosPitch;
        pitch *= narrowing ? 0.92F : 0.7F;
        pitch += pitchVariance * 0.1F;
        yaw += yawVariance * 0.1F;
        float change = tunnelRandom.nextGaussianFloat();
        change *= tunnelRandom.nextFloat();
        pitchVariance = pitchVariance * 0.9F + change * 2.0F;
        change = tunnelRandom.nextGaussianFloat();
        change *= tunnelRandom.nextFloat();
        yawVariance = yawVariance * 0.75F + change * 4.0F;
        if (!room && step == branchPoint && width > 1.0F)
        {
            long long branchSeed = tunnelRandom.nextLong();
            float branchWidth = tunnelRandom.nextFloat() * 0.5F + 0.5F;
            addTunnel(level, branchSeed, chunkX, chunkZ, primer, x, y, z, branchWidth, yaw - halfPi, pitch / 3.0F, step,
                      length, 1.0);
            branchSeed = tunnelRandom.nextLong();
            branchWidth = tunnelRandom.nextFloat() * 0.5F + 0.5F;
            addTunnel(level, branchSeed, chunkX, chunkZ, primer, x, y, z, branchWidth, yaw + halfPi, pitch / 3.0F, step,
                      length, 1.0);
            return;
        }
        if (!room && tunnelRandom.nextInt(4) == 0)
            continue;
        const float dx = static_cast<float>(x) - originX;
        const float dz = static_cast<float>(z) - originZ;
        const float remaining = static_cast<float>(length - step);
        const float reach = width + 18.0F;
        if (dx * dx + dz * dz - remaining * remaining > reach * reach)
            return;
        if (x < originX - 16.0F - radius * 2.0F || z < originZ - 16.0F - radius * 2.0F
            || x > originX + 16.0F + radius * 2.0F || z > originZ + 16.0F + radius * 2.0F)
            continue;
        int minX = static_cast<int>(std::floor(x - radius)) - chunkX * 16 - 1;
        int maxX = static_cast<int>(std::floor(x + radius)) - chunkX * 16 + 1;
        const double bottom = std::floor(y - radiusY);
        const double top = std::floor(y + radiusY);
        int minZ = static_cast<int>(std::floor(z - radius)) - chunkZ * 16 - 1;
        int maxZ = static_cast<int>(std::floor(z + radius)) - chunkZ * 16 + 1;
        if (minX < 0)
            minX = 0;
        if (maxX > 16)
            maxX = 16;
        const int maxY = Mth::clamp(static_cast<int>(top) + 1, 1, 120);
        const int minY = Mth::clamp(static_cast<int>(bottom) - 1, 1, maxY);
        if (minZ < 0)
            minZ = 0;
        if (maxZ > 16)
            maxZ = 16;
        int magmaData = Blocks::magma->convertBlockStateToLegacyData(Blocks::magma->defaultBlockState());
        int obsidianData = Blocks::obsidian->convertBlockStateToLegacyData(Blocks::obsidian->defaultBlockState());
        int flowingWaterData = Blocks::flowing_water->convertBlockStateToLegacyData(
            Blocks::flowing_water->defaultBlockState());
        int waterData = Blocks::water->convertBlockStateToLegacyData(Blocks::water->defaultBlockState());
        int lavaData = Blocks::lava->convertBlockStateToLegacyData(Blocks::lava->defaultBlockState());
        bool found = false;
        for (int localX = minX; localX < maxX; ++localX)
        {
            const float xd = (localX + chunkX * 16 + 0.5F - static_cast<float>(x)) / radius;
            for (int localZ = minZ; localZ < maxZ; ++localZ)
            {
                const float zd = (localZ + chunkZ * 16 + 0.5F - static_cast<float>(z)) / radius;
                if (xd * xd + zd * zd >= 1.0F)
                    continue;
                for (int localY = maxY - 1; localY >= minY; --localY)
                {
                    const float yd = (localY + 0.5F - static_cast<float>(y)) / radiusY;
                    const int index = (localX << 11) | (localZ << 7) | localY;
                    const float yd2 = yd * yd;
                    if (yd <= -0.7 || xd * xd + yd2 + zd * zd >= 1.0F || localY >= 0x3e
                        || !isDiggable(primer->getBlockId(index)))
                        continue;
                    if (!found && level != NULL && level->getBiome(startPos)->getBiomeType() != Biome::eBiomeType_Ocean)
                        return;
                    found = true;
                    if (localY < 9)
                    {
                        primer->setBlockAndData(index, BlockId::LAVA, lavaData);
                    }
                    else if (localY < 10)
                    {
                        if (tunnelRandom.nextFloat() < 0.25F)
                            primer->setBlockAndData(index, BlockId::MAGMA, magmaData);
                        else
                            primer->setBlockAndData(index, BlockId::OBSIDIAN, obsidianData);
                    }
                    else
                    {
                        bool water = false;
                        if (localX < 15 && primer->getBlockId(index + 0x800) == BlockId::AIR)
                        {
                            primer->setBlockAndData(index + 0x800, BlockId::FLOWING_WATER, flowingWaterData);
                            water = true;
                        }
                        if (localX > 0 && primer->getBlockId(index - 0x800) == BlockId::AIR)
                        {
                            primer->setBlockAndData(index - 0x800, BlockId::FLOWING_WATER, flowingWaterData);
                            water = true;
                        }
                        if (localZ < 15 && primer->getBlockId(index + 0x80) == BlockId::AIR)
                        {
                            primer->setBlockAndData(index + 0x80, BlockId::FLOWING_WATER, flowingWaterData);
                            water = true;
                        }
                        if (localZ > 0 && primer->getBlockId(index - 0x80) == BlockId::AIR)
                        {
                            primer->setBlockAndData(index - 0x80, BlockId::FLOWING_WATER, flowingWaterData);
                            water = true;
                        }
                        if (!water)
                            primer->setBlockAndData(index, BlockId::WATER, waterData);
                    }
                }
            }
        }
        if (room)
            return;
    }
}

void UnderwaterLargeCaveFeature::addRoom(Level* level, long long seed, int chunkX, int chunkZ, ChunkPrimer* primer,
                                         double x, double y, double z)
{
    addTunnel(level, seed, chunkX, chunkZ, primer, x, y, z, random.nextFloat() * 6.0F + 1.0F, 0.0F, 0.0F, -1, -1, 0.5);
}

void UnderwaterLargeCaveFeature::addFeature(Level* level, int chunkX, int chunkZ, int originChunkX, int originChunkZ,
                                            ChunkPrimer* primer)
{
    int count = random.nextInt(40);
    count = random.nextInt(count + 1);
    count = random.nextInt(count + 1);
    if (random.nextInt(15) != 0)
        count = 0;
    for (int i = 0; i < count; ++i)
    {
        const double z = chunkZ * 16 + random.nextInt(16);
        const double y = random.nextInt(random.nextInt(120) + 8);
        const double x = chunkX * 16 + random.nextInt(16);
        int branches = 1;
        if (random.nextInt(4) == 0)
        {
            addRoom(level, random.nextLong(), originChunkX, originChunkZ, primer, x, y, z);
            branches = random.nextInt(4) + 1;
        }
        for (int branch = 0; branch < branches; ++branch)
        {
            const float yaw = random.nextFloat() * 3.1415927f * 2.0f;
            const float pitch = (random.nextFloat() - 0.5f) * 2.0f * 0.125f;
            const float firstWidth = random.nextFloat();
            float tunnelWidth = firstWidth * 2.0f + random.nextFloat();
            addTunnel(level, random.nextLong(), originChunkX, originChunkZ, primer, x, y, z, tunnelWidth, yaw, pitch, 0,
                      0, 1.0);
        }
    }
}
