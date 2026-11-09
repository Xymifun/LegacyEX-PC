#ifndef GAME_WORLD_NETHERFLATLEVELSOURCE_HPP
#define GAME_WORLD_NETHERFLATLEVELSOURCE_HPP

#include <string>
#include <vector>

#include "game/world/chunk/ChunkGenerator.hpp"

class BlockPos;
class ChunkPrimer;
class Level;
class LevelChunk;
class MobCategory;
class Random;

class NetherFlatLevelSource : public ChunkGenerator
{
public:
    NetherFlatLevelSource(Level* level, bool generateStructures, long long seed);
    virtual ~NetherFlatLevelSource();

    void prepareHeights(int chunkX, int chunkZ, ChunkPrimer* primer);
    void buildSurfaces(int chunkX, int chunkZ, ChunkPrimer* primer);
    virtual LevelChunk* createChunk(int chunkX, int chunkZ);
    virtual void postProcess(int chunkX, int chunkZ);
    virtual bool postProcessLoadedChunk(LevelChunk* chunk, int chunkX, int chunkZ);
    virtual std::vector<Biome::MobSpawnerData*>* getMobsAt(MobCategory* category, const BlockPos& pos);
    virtual BlockPos* findNearestMapFeature(Level* level, const std::wstring& feature, const BlockPos& pos,
                                            bool takeBlock);
    virtual void recreateLogicStructuresForChunk(LevelChunk* chunk, int chunkX, int chunkZ);
    virtual bool isPosInFeature(Level* level, const std::wstring& feature, const BlockPos& pos);
    virtual void lightChunk(LevelChunk* chunk);

private:
    Random* mRandom;
    Random* mPopulationRandom;
    bool mGenerateStructures;
    Level* mLevel;
};

#endif
