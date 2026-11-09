#ifndef GAME_WORLD_THEENDLEVELSOURCE_HPP
#define GAME_WORLD_THEENDLEVELSOURCE_HPP

#include <string>
#include <vector>

#include "game/core/PerlinNoise.hpp"
#include "game/core/SimplexNoise.hpp"
#include "game/util/Random.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/ChunkGenerator.hpp"

class Biome;
class ChunkPrimer;
class EndCityFeature;
class EndIslandFeature;
class Level;
class LevelChunk;
class MobCategory;

class TheEndLevelSource : public ChunkGenerator
{
public:
    TheEndLevelSource(Level* level, bool generateStructures, long long seed, const BlockPos& origin);
    virtual ~TheEndLevelSource();

    double fjIslandProximity(int x, int z);
    bool fjIslandCheck(int x, int z);
    float getIslandHeightValue(int x, int z, int xOffset, int zOffset);
    arrayWithLength<double> getHeights(arrayWithLength<double> values, int x, int y, int z, int xSize, int ySize,
                                       int zSize);
    void prepareHeights(int chunkX, int chunkZ, ChunkPrimer* primer);
    void buildSurfaces(ChunkPrimer* primer);
    virtual LevelChunk* createChunk(int chunkX, int chunkZ);
    virtual void postProcess(int chunkX, int chunkZ);
    virtual bool postProcessLoadedChunk(LevelChunk* chunk, int chunkX, int chunkZ);
    virtual std::vector<Biome::MobSpawnerData*>* getMobsAt(MobCategory* category, const BlockPos& pos);
    virtual BlockPos* findNearestMapFeature(Level* level, const std::wstring& feature, const BlockPos& pos,
                                            bool findUnexplored);
    virtual void recreateLogicStructuresForChunk(LevelChunk* chunk, int chunkX, int chunkZ);
    virtual bool isPosInFeature(Level* level, const std::wstring& feature, const BlockPos& pos);
    bool isIslandChunk(int x, int z);

private:
    Random* mRandom;
    Random* mPopulationRandom;
    PerlinNoise* mNoise1;
    PerlinNoise* mNoise2;
    PerlinNoise* mNoise3;
    PerlinNoise* mNoise4;
    PerlinNoise* mNoise5;
    Level* mLevel;
    bool mGenerateStructures;
    BlockPos mOrigin;
    EndCityFeature* mEndCityFeature;
    SimplexNoise* mIslandNoise;
    arrayWithLength<Biome*> mBiomes;
    EndIslandFeature* mEndIslandFeature;
};

#endif
