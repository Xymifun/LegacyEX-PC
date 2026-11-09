#ifndef GAME_WORLD_FLATLEVELSOURCE_HPP
#define GAME_WORLD_FLATLEVELSOURCE_HPP

#include <string>
#include <vector>

#include <boost/unordered_map.hpp>

#include "game/util/Random.hpp"
#include "game/world/GenericOverworldLevelSource.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/chunk/ChunkGenerator.hpp"

class BlockPos;
class BlockState;
class ChunkPrimer;
class Feature;
class LargeFeature;
class Level;
class LevelChunk;
class MobCategory;
class OceanMonumentFeature;
class StructureFeature;
class SuperflatConfig;

class FlatLevelSource : public ChunkGenerator, public GenericOverworldLevelSource
{
public:
    typedef boost::unordered_map<std::wstring, StructureFeature*> StructureMap;

    FlatLevelSource(Level* level, long long seed, bool generateStructures, SuperflatConfig* config);
    virtual ~FlatLevelSource();

    virtual LevelChunk* createChunk(int chunkX, int chunkZ);
    virtual void postProcess(int chunkX, int chunkZ);
    virtual bool postProcessLoadedChunk(LevelChunk* chunk, int chunkX, int chunkZ);
    virtual std::vector<Biome::MobSpawnerData*>* getMobsAt(MobCategory* category, const BlockPos& pos);
    virtual BlockPos* findNearestMapFeature(Level* level, const std::wstring& feature, const BlockPos& pos,
                                            bool findUnexplored);
    virtual void recreateLogicStructuresForChunk(LevelChunk* chunk, int chunkX, int chunkZ);
    virtual bool isPosInFeature(Level* level, const std::wstring& feature, const BlockPos& pos);
    virtual void prepareHeights(int chunkX, int chunkZ, ChunkPrimer* primer);
    virtual bool isFlat();

private:
    Level* mLevel;
    Random mRandom;
    Random mPopulationRandom;
    const BlockState* mLayers[256];
    StructureMap mStructureFeatures;
    LargeFeature* mCaves;
    LargeFeature* mCanyons;
    bool mDecorations;
    bool mDungeons;
    Feature* mLakes;
    Feature* mLavaLakes;
    OceanMonumentFeature* mOceanMonument;
    arrayWithLength<unsigned short> mElementIds;
};

#endif
