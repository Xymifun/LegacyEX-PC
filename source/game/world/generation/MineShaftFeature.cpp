#include "game/world/generation/MineShaftFeature.hpp"

#include <algorithm>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/MineShaftStart.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/biome/MesaBiome.hpp"
#include "game/world/block/BlockPos.hpp"

MineShaftFeature::MineShaftFeature() : StructureFeature(), mChance(0.004)
{
}

MineShaftFeature::MineShaftFeature(SuperflatConfig* config)
    : StructureFeature(),
      mChance((float)config->maybeOption(0x23, 0, 1) / 1000.0f)
{
}

std::wstring MineShaftFeature::getFeatureName()
{
    return std::wstring(L"Mineshaft");
}

bool MineShaftFeature::isFeatureChunk(int chunkX, int chunkZ, bool flatWorld)
{
    bool forced = false;
    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
    if (options != NULL)
        forced = options->isFeatureChunk(chunkX, chunkZ, eFeatureType_Mineshaft, NULL);

    return forced || (random.nextDouble() < mChance && random.nextInt(80) < std::max(abs(chunkX), abs(chunkZ)));
}

BlockPos* MineShaftFeature::getNearestGeneratedFeature(Level* levelIn, const BlockPos& pos, bool skipExistingChunks)
{
    int chunkX = pos.x >> 4;
    int chunkZ = pos.z >> 4;

    for (int radius = 0; radius <= 1000; ++radius)
    {
        for (int offsetX = -radius; offsetX <= radius; ++offsetX)
        {
            bool edgeX = offsetX == -radius || offsetX == radius;
            for (int offsetZ = -radius; offsetZ <= radius; ++offsetZ)
            {
                bool edgeZ = offsetZ == -radius || offsetZ == radius;
                if (edgeX || edgeZ)
                {
                    int candidateX = chunkX + offsetX;
                    int candidateZ = chunkZ + offsetZ;
                    random.setSeed((long long)(candidateX ^ candidateZ) ^ levelIn->getSeed());
                    random.nextInt();
                    if (isFeatureChunk(candidateX, candidateZ, false))
                    {
                        if (!skipExistingChunks || !levelIn->isChunkGeneratedAt(candidateX, candidateZ))
                            return new BlockPos(candidateX * 16 + 8, 64, candidateZ * 16 + 8);
                    }
                }
            }
        }
    }

    return NULL;
}

StructureStart* MineShaftFeature::createStructureStart(int chunkX, int chunkZ)
{
    gApp.AddTerrainFeaturePosition((_eTerrainFeatureType)2, chunkX, chunkZ);
    Biome* biome = level->getBiome(BlockPos(chunkX * 16 + 8, 64, chunkZ * 16 + 8));
    bool mesa = dynamic_cast<MesaBiome*>(biome) != NULL;
    MineShaftType type = mesa ? eMineShaftType_Mesa : eMineShaftType_Normal;
    return new MineShaftStart(level, random, chunkX, chunkZ, type);
}
