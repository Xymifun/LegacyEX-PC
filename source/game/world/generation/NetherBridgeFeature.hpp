#ifndef GAME_WORLD_GENERATION_NETHERBRIDGEFEATURE_HPP
#define GAME_WORLD_GENERATION_NETHERBRIDGEFEATURE_HPP

#include <string>
#include <vector>

#include "game/world/generation/StructureFeature.hpp"
#include "game/world/generation/StructureStart.hpp"
#include "game/world/biome/Biome.hpp"

class BlockPos;
class ChunkPos;
class Level;
class Random;

class NetherBridgeFeature : public StructureFeature
{
public:
    class NetherBridgeStart : public StructureStart
    {
    public:
        NetherBridgeStart(Level* level, Random& random, int chunkX, int chunkZ);
        NetherBridgeStart();
        virtual EStructureStart GetType();
        static StructureStart* Create();
    };

    NetherBridgeFeature();
    virtual ~NetherBridgeFeature();
    virtual std::wstring getFeatureName();
    std::vector<Biome::MobSpawnerData*>* getBridgeEnemies();
    virtual bool isFeatureChunk(int chunkX, int chunkZ, bool flatWorld);
    virtual StructureStart* createStructureStart(int chunkX, int chunkZ);
    virtual BlockPos* getNearestGeneratedFeature(Level* level, const BlockPos& pos, bool skipExistingChunks);
    void clearCachedBuildings();

private:
    std::vector<Biome::MobSpawnerData*> mBridgeEnemies;
    bool mCachedBuildings;
    ChunkPos* mCachedChunk;
#ifdef MINECRAFT_PC
    void PlaceGridFortresses();
    std::vector<ChunkPos*> mGridChunks;
#endif
};

#endif
