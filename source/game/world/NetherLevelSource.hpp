#ifndef GAME_WORLD_NETHERLEVELSOURCE_HPP
#define GAME_WORLD_NETHERLEVELSOURCE_HPP

#include <string>
#include <vector>

#include "game/core/PerlinNoise.hpp"
#include "game/util/Random.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/chunk/ChunkGenerator.hpp"
#include "game/world/generation/NetherBridgeFeature.hpp"

class BlockPos;
class BushFeature;
class ChunkPrimer;
class HellFireFeature;
class HellPortalFeature;
class HellSpringFeature;
class LargeHellCaveFeature;
class Level;
class LevelChunk;
class LightGemFeature;
class MobCategory;
class OreFeature;
class StructureFeature;

class NetherLevelSource : public ChunkGenerator
{
public:
    NetherLevelSource(Level* level, bool generateStructures, long long seed);
    virtual ~NetherLevelSource();

    void prepareHeights(int chunkX, int chunkZ, ChunkPrimer* primer);
    void buildSurfaces(int chunkX, int chunkZ, ChunkPrimer* primer);
    virtual void lightChunk(LevelChunk* chunk);
    virtual arrayWithLength<double> getHeights(arrayWithLength<double> values, int x, int y, int z, int xSize,
                                               int ySize, int zSize);
    virtual LevelChunk* createChunk(int chunkX, int chunkZ);
    virtual void postProcess(int chunkX, int chunkZ);
    virtual bool postProcessLoadedChunk(LevelChunk* chunk, int chunkX, int chunkZ);
    virtual std::vector<Biome::MobSpawnerData*>* getMobsAt(MobCategory* category, const BlockPos& pos);
    virtual BlockPos* findNearestMapFeature(Level* level, const std::wstring& feature, const BlockPos& pos,
                                            bool findUnexplored);
    virtual void recreateLogicStructuresForChunk(LevelChunk* chunk, int chunkX, int chunkZ);
    virtual bool isPosInFeature(Level* level, const std::wstring& feature, const BlockPos& pos);

private:
    Level* mLevel;
    bool mGenerateStructures;
    Random mRandom;
    Random mPopulationRandom;
    PerlinNoise* mNoise1;
    PerlinNoise* mNoise2;
    PerlinNoise* mNoise3;
    PerlinNoise* mNoise4;
    PerlinNoise* mNoise5;
    PerlinNoise* mNoise6;
    PerlinNoise* mNoise7;
    HellFireFeature* mFireFeature;
    LightGemFeature* mLightGemFeature;
    HellPortalFeature* mHellPortalFeature;
    OreFeature* mQuartzOre;
    OreFeature* mMagmaOre;
    HellSpringFeature* mLavaSpring;
    HellSpringFeature* mLavaSpringOpen;
    BushFeature* mBrownMushroom;
    BushFeature* mRedMushroom;
    NetherBridgeFeature* mNetherBridgeFeature;
    LargeHellCaveFeature* mCaves;
};

#endif
