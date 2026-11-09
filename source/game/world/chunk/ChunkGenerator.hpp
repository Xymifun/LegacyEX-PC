#ifndef GAME_WORLD_CHUNK_CHUNKGENERATOR_HPP
#define GAME_WORLD_CHUNK_CHUNKGENERATOR_HPP

#include <string>
#include <vector>

#include "game/world/biome/Biome.hpp"

class BlockPos;
class Level;
class LevelChunk;
class MobCategory;

class ChunkGenerator
{
public:
    int mLevelSize;

    virtual ~ChunkGenerator() {}
    virtual LevelChunk* createChunk(int chunkX, int chunkZ) = 0;
    virtual void postProcess(int chunkX, int chunkZ) = 0;
    virtual bool postProcessLoadedChunk(LevelChunk* chunk, int chunkX, int chunkZ) = 0;
    virtual std::vector<Biome::MobSpawnerData*>* getMobsAt(MobCategory* category, const BlockPos& pos) = 0;
    virtual BlockPos* findNearestMapFeature(Level* level, const std::wstring& feature, const BlockPos& pos,
                                            bool takeBlock) = 0;
    virtual void recreateLogicStructuresForChunk(LevelChunk* chunk, int chunkX, int chunkZ) = 0;
    virtual bool isPosInFeature(Level* level, const std::wstring& feature, const BlockPos& pos) = 0;
    virtual void lightChunk(LevelChunk* chunk);
};

#endif
