#ifndef GAME_WORLD_GENERATION_STRUCTUREFEATURE_HPP
#define GAME_WORLD_GENERATION_STRUCTUREFEATURE_HPP

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <string>
#include <vector>

#include "game/world/generation/LargeFeature.hpp"

class BlockPos;
class ChunkPos;
class ChunkPrimer;
class Level;
class Random;
class StructureFeatureSavedData;
class StructureStart;

class StructureFeature : public LargeFeature
{
public:
    enum EFeatureTypes
    {
        eFeatureType_Mineshaft = 0,
        eFeatureType_NetherBridge = 1,
        eFeatureType_ScatteredFeature = 2,
        eFeatureType_Stronghold = 3,
        eFeatureType_Village = 4,
        eFeatureType_OceanMonument = 5,
        eFeatureType_WoodlandMansion = 6,
        eFeatureType_BuriedTreasure = 7,
        eFeatureType_Shipwreck = 8,
        eFeatureType_OceanRuin = 9,
        eFeatureType_EndCity = 10
    };

    StructureFeature();
    virtual ~StructureFeature();

    virtual void addFeature(Level* level, int chunkX, int chunkZ, int originChunkX, int originChunkZ,
                            ChunkPrimer* primer);
    virtual std::wstring getFeatureName() = 0;
    virtual bool postProcess(Level* level, Random& random, ChunkPos* chunkPos);
    virtual bool isIntersection(int x, int z);
    virtual bool isInsideFeature(const BlockPos& pos);
    virtual BlockPos* getNearestGeneratedFeature(Level* level, const BlockPos& pos, bool skipExistingChunks) = 0;
    virtual std::vector<BlockPos>* getGuesstimatedFeaturePositions();
    virtual void restoreSavedData(Level* level);
    virtual void saveFeature(int chunkX, int chunkZ, StructureStart* start);
    virtual bool isFeatureChunk(int chunkX, int chunkZ, bool flatWorld) = 0;
    virtual StructureStart* createStructureStart(int chunkX, int chunkZ) = 0;

    StructureStart* getStructureAt(const BlockPos& pos);
    bool isInsideBoundingFeature(Level* level, const BlockPos& pos);
    static BlockPos* findNearestFeaturePositionBySpacing(Level* level, StructureFeature* feature, const BlockPos& pos,
                                                         int spacing, int separation, int salt,
                                                         bool skipKnownStructures, int maxDistance,
                                                         bool skipExistingChunks);
    bool canPlaceFeatureInChunk(bool defaultValue, EFeatureTypes type, int chunkX, int chunkZ);
    bool hasStructureAt(const BlockPos& pos);

protected:
    boost::shared_ptr<StructureFeatureSavedData> mSavedData;
    boost::unordered_map<long long, StructureStart*> mStructureMap;
};

#endif
