#ifndef GAME_WORLD_CUSTOMLEVELSOURCE_HPP
#define GAME_WORLD_CUSTOMLEVELSOURCE_HPP

#include <string>
#include <vector>

#include "game/world/GenericOverworldLevelSource.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/chunk/ChunkGenerator.hpp"

class BlockPos;
class ChunkPrimer;
class Level;
class LevelChunk;
class MobCategory;
class StructureFeature;

class CustomLevelSource : public ChunkGenerator, public GenericOverworldLevelSource
{
public:
    CustomLevelSource(Level* level, long long seed, bool generateStructures);
    virtual ~CustomLevelSource();

    virtual void prepareHeights(int chunkX, int chunkZ, ChunkPrimer* primer);
    virtual LevelChunk* createChunk(int chunkX, int chunkZ);
    virtual void lightChunk(LevelChunk* chunk);
    virtual void postProcess(int chunkX, int chunkZ);
    virtual bool postProcessLoadedChunk(LevelChunk* chunk, int chunkX, int chunkZ);
    virtual std::vector<Biome::MobSpawnerData*>* getMobsAt(MobCategory* category, const BlockPos& pos);
    virtual bool isPosInFeature(Level* level, const std::wstring& feature, const BlockPos& pos);
    virtual BlockPos* findNearestMapFeature(Level* level, const std::wstring& feature, const BlockPos& pos,
                                            bool findUnexplored);
    virtual void recreateLogicStructuresForChunk(LevelChunk* chunk, int chunkX, int chunkZ);

    int getHeightFalloff(int x, int z, int* heights);
    void buildSurfaces(int x, int z, ChunkPrimer* primer, arrayWithLength<Biome*> biomes);
    void getHeights(int x, int y, int z, arrayWithLength<Biome*>& biomes, arrayWithLength<double>& heights);
    int calcWaterDepths(int x, int z);
    bool isWater(const BlockPos& pos);

private:
    unsigned char mReservedBeforeFlag[56];
    bool mGenerateStructures;
    unsigned char mReservedBeforeFeatures[31];
    StructureFeature* mStronghold;
    StructureFeature* mVillage;
    StructureFeature* mMineshaft;
    StructureFeature* mTemple;
    unsigned char mReservedBeforeMonument[12];
    StructureFeature* mMonument;
    StructureFeature* mMansion;
    unsigned char mReservedBeforeShipwreck[4];
    StructureFeature* mShipwreck;
    StructureFeature* mOceanRuin;
};

#endif
