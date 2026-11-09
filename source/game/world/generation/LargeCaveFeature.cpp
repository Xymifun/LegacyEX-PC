#include "game/world/generation/LargeCaveFeature.hpp"
#include "game/world/block/BlockId.hpp"

#include "game/core/Mth.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"

unsigned int LargeCaveFeature::canReplaceBlock(int blockId, int data)
{
    Block* block;
    if (blockId != BlockId::STONE && blockId != BlockId::DIRT && blockId != BlockId::GRASS
        && blockId != BlockId::HARDENED_CLAY && blockId != BlockId::STAINED_HARDENED_CLAY
        && blockId != BlockId::SANDSTONE && blockId != BlockId::RED_SANDSTONE && blockId != BlockId::MYCELIUM
        && blockId != BlockId::SNOW_LAYER
        && ((blockId != BlockId::SAND && blockId != BlockId::GRAVEL) || (block = Block::byId(data)) == NULL
            || block->getCraftingMaterial() == Material::WATER))
        return 0;
    return 1;
}

void LargeCaveFeature::setCaveBlocks(Level* level, int minX, int maxX, int minY, int maxY, int minZ, int maxZ,
                                     int chunkX, int chunkZ, double centerX, double centerY, double centerZ,
                                     bool hasWater, double radiusX, double radiusZ, ChunkPrimer* primer)
{
    BlockPos columnPos(0, 0, 0);
    int lavaData = Blocks::lava->convertBlockStateToLegacyData(Blocks::lava->defaultBlockState());
    int waterData = Blocks::water->convertBlockStateToLegacyData(Blocks::water->defaultBlockState());
    bool carved = false;
    for (int localX = minX; localX < maxX; ++localX)
    {
        int worldX = localX + chunkX * 16;
        double xd = ((double)worldX + 0.5 - centerX) / radiusX;
        for (int localZ = minZ; localZ < maxZ; ++localZ)
        {
            int worldZ = localZ + chunkZ * 16;
            columnPos.x = worldX;
            columnPos.y = 0;
            columnPos.z = worldZ;
            Biome* biome = level->getBiome(columnPos);
            bool frozen = biome->getDepth() < -0.9F && biome->isSnowCovered();
            if (hasWater && !frozen)
                return;

            double zd = ((double)worldZ + 0.5 - centerZ) / radiusX;
            bool foundTop = false;
            if (xd * xd + zd * zd >= 1.0)
                continue;

            for (int localY = maxY - 1; localY >= minY; --localY)
            {
                double yd = ((double)(localY - 1) + 0.5 - centerY) / radiusZ;
                int index = (localX << 11) | (localZ << 7) | localY;
                if (yd <= -0.7 || yd * yd + xd * xd + zd * zd >= 1.0)
                    continue;

                int blockId = primer->getBlockId(index);
                int aboveBlockId = primer->getBlockId(index + 1);
                if (blockId == BlockId::GRASS || blockId == BlockId::MYCELIUM)
                    foundTop = true;
                if ((hasWater && blockId != BlockId::PACKED_ICE) || !canReplaceBlock(blockId, aboveBlockId))
                    continue;

                if (!carved
                    && level->getBiome(BlockPos(centerX, centerY, centerZ))->getBiomeType() == Biome::eBiomeType_Ocean)
                    return;
                carved = true;

                if (localY - 1 < 10)
                {
                    primer->setBlockAndData(index, BlockId::LAVA, lavaData);
                }
                else
                {
                    if (frozen && localY < level->getSeaLevel() && blockId == BlockId::PACKED_ICE)
                        primer->setBlockAndData(index, BlockId::WATER, waterData);
                    else
                        primer->setBlockAndData(index, BlockId::AIR, 0);

                    if (foundTop && primer->getBlockId(index - 1) == BlockId::DIRT)
                    {
                        Biome* columnBiome = level->getBiome(columnPos);
                        Block* topBlock = Block::byId(columnBiome->mTopBlock.blockId);
                        primer->setBlockAndData(index - 1, columnBiome->mTopBlock.blockId,
                                                topBlock->convertBlockStateToLegacyData(topBlock->defaultBlockState()));
                    }
                }
            }
        }
    }
}

void LargeCaveFeature::addTunnel(Level* level, long long seed, int chunkX, int chunkZ, ChunkPrimer* primer, double x,
                                 double y, double z, float width, float yaw, float pitch, int start, int length,
                                 double scale)
{
    const double originX = chunkX * 16 + 8;
    const double originZ = chunkZ * 16 + 8;
    Random tunnelRandom(seed);
    const BlockPos startPos(x, y, z);
    if (length < 1)
    {
        const int maxLength = range * 16 - 16;
        length = maxLength - tunnelRandom.nextInt(maxLength / 4);
    }
    bool room = false;
    if (start == -1)
    {
        room = true;
        start = length / 2;
    }
    const int branchPoint = tunnelRandom.nextInt(length / 2) + length / 4;
    const bool gentlePitch = tunnelRandom.nextInt(6) == 0;
    float yawVariance = 0.0f;
    float pitchVariance = 0.0f;
    for (int step = start; step < length; ++step)
    {
        const double radius = 1.5 + Mth::sin(step * 3.1415927f / length) * width * 1.0f;
        const double verticalRadius = radius * scale;
        const float horizontal = Mth::cos(pitch);
        const float vertical = Mth::sin(pitch);
        x += Mth::cos(yaw) * horizontal;
        y += vertical;
        z += Mth::sin(yaw) * horizontal;
        if (gentlePitch)
            pitch *= 0.92f;
        else
            pitch *= 0.7f;
        pitch += pitchVariance * 0.1f;
        yaw += yawVariance * 0.1f;
        pitchVariance *= 0.9f;
        yawVariance *= 0.75f;
        float first = tunnelRandom.nextFloat();
        float second = tunnelRandom.nextFloat();
        float strength = tunnelRandom.nextFloat();
        pitchVariance += (first - second) * strength * 2.0f;
        first = tunnelRandom.nextFloat();
        second = tunnelRandom.nextFloat();
        strength = tunnelRandom.nextFloat();
        yawVariance += (first - second) * strength * 4.0f;
        if (!room && step == branchPoint && width > 1.0f)
        {
            long long branchSeed = tunnelRandom.nextLong();
            float branchWidth = tunnelRandom.nextFloat() * 0.5f + 0.5f;
            addTunnel(level, branchSeed, chunkX, chunkZ, primer, x, y, z, branchWidth, yaw - 1.5707964f, pitch / 3.0f,
                      step, length, 1.0);
            branchSeed = tunnelRandom.nextLong();
            branchWidth = tunnelRandom.nextFloat() * 0.5f + 0.5f;
            addTunnel(level, branchSeed, chunkX, chunkZ, primer, x, y, z, branchWidth, yaw + 1.5707964f, pitch / 3.0f,
                      step, length, 1.0);
            return;
        }
        if (!room && tunnelRandom.nextInt(4) == 0)
            continue;
        const double dx = x - originX;
        const double dz = z - originZ;
        const double remaining = length - step;
        const double reach = width + 18.0f;
        if (dx * dx + dz * dz - remaining * remaining > reach * reach)
            return;
        if (x < originX - 16.0 - radius * 2.0 || z < originZ - 16.0 - radius * 2.0 || x > originX + 16.0 + radius * 2.0
            || z > originZ + 16.0 + radius * 2.0)
            continue;
        int minX = Mth::floor(x - radius) - chunkX * 16 - 1;
        int maxX = Mth::floor(x + radius) - chunkX * 16 + 1;
        int minY = Mth::floor(y - verticalRadius) - 1;
        int maxY = Mth::floor(y + verticalRadius) + 1;
        int minZ = Mth::floor(z - radius) - chunkZ * 16 - 1;
        int maxZ = Mth::floor(z + radius) - chunkZ * 16 + 1;
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
        bool hasWater = false;
        for (int localX = minX; !hasWater && localX < maxX; ++localX)
        {
            for (int localZ = minZ; !hasWater && localZ < maxZ; ++localZ)
            {
                for (int localY = maxY + 1; !hasWater && localY >= minY - 1; --localY)
                {
                    if (localY >= 0 && localY < 128)
                    {
                        const int blockId = primer->getBlockId((localX << 11) | (localZ << 7) | localY);
                        if (blockId == BlockId::FLOWING_WATER || blockId == BlockId::WATER)
                            hasWater = true;
                        if (localY != minY - 1 && localX != minX && localX != maxX - 1 && localZ != minZ
                            && localZ != maxZ - 1)
                            localY = minY;
                    }
                }
            }
        }
        Biome* biome = level->getBiome(BlockPos(originX, 0.0, originZ));
        if (!hasWater || (biome->getDepth() < -0.9f && biome->isSnowCovered()))
        {
            if (level->getBiome(startPos)->getBiomeType() == Biome::eBiomeType_Ocean)
                return;
            setCaveBlocks(level, minX, maxX, minY, maxY, minZ, maxZ, chunkX, chunkZ, x, y, z, hasWater, radius,
                          verticalRadius, primer);
            if (room)
                return;
        }
    }
}

void LargeCaveFeature::addRoom(Level* level, long long seed, int chunkX, int chunkZ, ChunkPrimer* primer, double x,
                               double y, double z)
{
    addTunnel(level, seed, chunkX, chunkZ, primer, x, y, z, random.nextFloat() * 6.0f + 1.0f, 0.0f, 0.0f, -1, -1, 0.5);
}

void LargeCaveFeature::addFeature(Level* level, int chunkX, int chunkZ, int originChunkX, int originChunkZ,
                                  ChunkPrimer* primer)
{
    int count = random.nextInt(40);
    count = random.nextInt(count + 1);
    count = random.nextInt(count + 1);
    if (random.nextInt(15) != 0)
        count = 0;
    for (int i = 0; i < count; ++i)
    {
        const double x = chunkX * 16 + random.nextInt(16);
        const double y = random.nextInt(random.nextInt(120) + 8);
        const double z = chunkZ * 16 + random.nextInt(16);
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
            if (random.nextInt(10) == 0)
            {
                const float firstScale = random.nextFloat();
                tunnelWidth *= firstScale * random.nextFloat() * 3.0f + 1.0f;
            }
            addTunnel(level, random.nextLong(), originChunkX, originChunkZ, primer, x, y, z, tunnelWidth, yaw, pitch, 0,
                      0, 1.0);
        }
    }
}
