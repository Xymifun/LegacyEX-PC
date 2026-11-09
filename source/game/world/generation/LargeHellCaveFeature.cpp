#include "game/world/generation/LargeHellCaveFeature.hpp"

#include "game/core/Mth.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"

LargeHellCaveFeature::~LargeHellCaveFeature()
{
}

void LargeHellCaveFeature::addRoom(long long seed, int chunkX, int chunkZ, ChunkPrimer* primer, double x, double y,
                                   double z)
{
    addTunnel(seed, chunkX, chunkZ, primer, x, y, z, 1.0f + random.nextFloat() * 6.0f, 0.0f, 0.0f, -1, -1, 0.5);
}

void LargeHellCaveFeature::addFeature(Level*, int chunkX, int chunkZ, int originChunkX, int originChunkZ,
                                      ChunkPrimer* primer)
{
    int count = random.nextInt(random.nextInt(random.nextInt(10) + 1) + 1);
    if (random.nextInt(5) != 0)
    {
        count = 0;
    }

    for (int i = 0; i < count; ++i)
    {
        const double x = (double)(chunkX * 16 + random.nextInt(16));
        const double y = (double)random.nextInt(128);
        const double z = (double)(chunkZ * 16 + random.nextInt(16));

        int branches = 1;
        if (random.nextInt(4) == 0)
        {
            addRoom(random.nextLong(), originChunkX, originChunkZ, primer, x, y, z);
            branches += random.nextInt(4);
        }

        for (int branch = 0; branch < branches; ++branch)
        {
            const float yaw = random.nextFloat() * 3.1415927f * 2.0f;
            const float pitch = (random.nextFloat() - 0.5f) * 2.0f / 8.0f;
            float width = random.nextFloat() * 2.0f + random.nextFloat();
            addTunnel(random.nextLong(), originChunkX, originChunkZ, primer, x, y, z, width * 2.0f, yaw, pitch, 0, 0,
                      0.5);
        }
    }
}

void LargeHellCaveFeature::addTunnel(long long seed, int chunkX, int chunkZ, ChunkPrimer* primer, double x, double y,
                                     double z, float width, float yaw, float pitch, int start, int end,
                                     double verticalScale)
{
    const double centerX = (double)(chunkX * 16 + 8);
    const double centerZ = (double)(chunkZ * 16 + 8);
    float yawDelta = 0.0f;
    float pitchDelta = 0.0f;
    Random tunnelRandom(seed);

    if (end <= 0)
    {
        const int limit = range * 16 - 16;
        end = limit - tunnelRandom.nextInt(limit / 4);
    }

    bool room = false;
    if (start == -1)
    {
        start = end / 2;
        room = true;
    }

    const int branchPoint = tunnelRandom.nextInt(end / 2) + end / 4;
    const bool shallow = tunnelRandom.nextInt(6) == 0;

    for (; start < end; ++start)
    {
        const double radius = 1.5 + (double)(Mth::sin((float)start * 3.1415927f / (float)end) * width * 1.0f);
        const double heightRadius = radius * verticalScale;
        const float cosPitch = Mth::cos(pitch);
        const float sinPitch = Mth::sin(pitch);
        const float cosYaw = Mth::cos(yaw);
        y += (double)sinPitch;
        x += (double)(cosYaw * cosPitch);
        z += (double)(Mth::sin(yaw) * cosPitch);

        if (shallow)
        {
            pitch = pitch * 0.92f;
        }
        else
        {
            pitch = pitch * 0.7f;
        }

        pitch = pitch + pitchDelta * 0.1f;
        yaw += yawDelta * 0.1f;
        pitchDelta = pitchDelta * 0.9f;
        yawDelta = yawDelta * 0.75f;
        pitchDelta = pitchDelta
                     + (tunnelRandom.nextFloat() - tunnelRandom.nextFloat()) * tunnelRandom.nextFloat() * 2.0f;
        yawDelta = yawDelta + (tunnelRandom.nextFloat() - tunnelRandom.nextFloat()) * tunnelRandom.nextFloat() * 4.0f;

        if (!room && start == branchPoint && width > 1.0f)
        {
            long long leftSeed = tunnelRandom.nextLong();
            float leftWidth = tunnelRandom.nextFloat() * 0.5f + 0.5f;
            pitch = pitch / 3.0f;
            addTunnel(leftSeed, chunkX, chunkZ, primer, x, y, z, leftWidth, yaw - 1.5707964f, pitch, start, end, 1.0);
            long long rightSeed = tunnelRandom.nextLong();
            float rightWidth = tunnelRandom.nextFloat() * 0.5f + 0.5f;
            addTunnel(rightSeed, chunkX, chunkZ, primer, x, y, z, rightWidth, yaw + 1.5707964f, pitch, start, end, 1.0);
            return;
        }

        if (room || tunnelRandom.nextInt(4) != 0)
        {
            const double offsetX = x - centerX;
            const double offsetZ = z - centerZ;
            const double remaining = (double)(end - start);
            const double reach = (double)(width + 18.0f);
            if (offsetX * offsetX + offsetZ * offsetZ - remaining * remaining > reach * reach)
            {
                return;
            }

            if (x >= centerX - 16.0 - radius * 2.0 && z >= centerZ - 16.0 - radius * 2.0
                && x <= centerX + 16.0 + radius * 2.0 && z <= centerZ + 16.0 + radius * 2.0)
            {
                int minX = Mth::floor(x - radius) - chunkX * 16 - 1;
                int maxX = Mth::floor(x + radius) - chunkX * 16 + 1;
                int minY = Mth::floor(y - heightRadius) - 1;
                int maxY = Mth::floor(y + heightRadius) + 1;
                int minZ = Mth::floor(z - radius) - chunkZ * 16 - 1;
                int maxZ = Mth::floor(z + radius) - chunkZ * 16 + 1;

                if (minX < 0)
                {
                    minX = 0;
                }
                if (maxX > 16)
                {
                    maxX = 16;
                }
                if (minY < 1)
                {
                    minY = 1;
                }
                if (maxY > 120)
                {
                    maxY = 120;
                }
                if (minZ < 0)
                {
                    minZ = 0;
                }
                if (maxZ > 16)
                {
                    maxZ = 16;
                }

                bool lava = false;
                for (int localX = minX; !lava && localX < maxX; ++localX)
                {
                    for (int localZ = minZ; !lava && localZ < maxZ; ++localZ)
                    {
                        for (int localY = maxY + 1; !lava && localY >= minY - 1; --localY)
                        {
                            if (localY >= 0 && localY < 128)
                            {
                                const BlockState* state = primer->getState(localX, localY, localZ);
                                if (state->getBlock() == Blocks::flowing_lava || state->getBlock() == Blocks::lava)
                                {
                                    lava = true;
                                }

                                if (localY != minY - 1 && localX != minX && localX != maxX - 1 && localZ != minZ
                                    && localZ != maxZ - 1)
                                {
                                    localY = minY;
                                }
                            }
                        }
                    }
                }

                if (lava)
                {
                    continue;
                }

                for (int localX = minX; localX < maxX; ++localX)
                {
                    const double dx = ((double)(localX + chunkX * 16) + 0.5 - x) / radius;
                    for (int localZ = minZ; localZ < maxZ; ++localZ)
                    {
                        const double dz = ((double)(localZ + chunkZ * 16) + 0.5 - z) / radius;
                        for (int localY = maxY - 1; localY >= minY; --localY)
                        {
                            const double dy = ((double)localY + 0.5 - y) / heightRadius;
                            if (dy > -0.7 && dx * dx + dy * dy + dz * dz < 1.0)
                            {
                                const BlockState* state = primer->getState(localX, localY, localZ);
                                if (state->getBlock() == Blocks::netherrack || state->getBlock() == Blocks::dirt
                                    || state->getBlock() == Blocks::grass)
                                {
                                    primer->setState(localX, localY, localZ, Blocks::air->defaultBlockState());
                                }
                            }
                        }
                    }
                }

                if (room)
                {
                    break;
                }
            }
        }
    }
}
