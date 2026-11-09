#ifndef GAME_WORLD_OVERWORLDLEVELSOURCE_HPP
#define GAME_WORLD_OVERWORLDLEVELSOURCE_HPP

#include <string>
#include <vector>

#include "game/util/Random.hpp"
#include "game/world/GenericOverworldLevelSource.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/chunk/ChunkGenerator.hpp"

class BlockPos;
class BlockState;
class BuriedTreasureFeature;
class CanyonFeature;
class ChunkPrimer;
class CustomizableSourceSettings;
class LargeCaveFeature;
class Level;
class LevelChunk;
class LevelType;
class MineShaftFeature;
class MobCategory;
class OceanMonumentFeature;
class OceanRuinFeature;
class PerlinNoise;
class PerlinSimplexNoise;
class RandomScatteredLargeFeature;
class ShipwreckFeature;
class StrongholdFeature;
class SuperflatConfig;
class UnderwaterCanyonFeature;
class UnderwaterLargeCaveFeature;
class VillageFeature;
class WoodlandMansionFeature;

class OverworldLevelSource : public ChunkGenerator, public GenericOverworldLevelSource
{
public:
    OverworldLevelSource(Level* level, long long seed, bool generateStructures, SuperflatConfig* config);
    virtual ~OverworldLevelSource();

    float getHeightFalloff(int x, int z, int* distance);
    void getHeights(int x, int y, int z, arrayWithLength<Biome*>& biomes, arrayWithLength<double>& heights);
    virtual void prepareHeights(int chunkX, int chunkZ, ChunkPrimer* primer);
    void buildSurfaces(int chunkX, int chunkZ, ChunkPrimer* primer, arrayWithLength<Biome*> biomes);
    virtual LevelChunk* createChunk(int chunkX, int chunkZ);
    virtual void lightChunk(LevelChunk* chunk);
    bool isWater(const BlockPos& pos);
    void calcWaterDepths(int chunkX, int chunkZ);
    virtual void postProcess(int chunkX, int chunkZ);
    virtual bool postProcessLoadedChunk(LevelChunk* chunk, int chunkX, int chunkZ);
    virtual std::vector<Biome::MobSpawnerData*>* getMobsAt(MobCategory* category, const BlockPos& pos);
    virtual bool isPosInFeature(Level* level, const std::wstring& feature, const BlockPos& pos);
    virtual BlockPos* findNearestMapFeature(Level* level, const std::wstring& feature, const BlockPos& pos,
                                            bool takeBlock);
    virtual void recreateLogicStructuresForChunk(LevelChunk* chunk, int chunkX, int chunkZ);
    static void staticCtor();

private:
    Random mRandom;
    Random mPopulationRandom;
    PerlinNoise* mMinLimitNoise;
    PerlinNoise* mMaxLimitNoise;
    PerlinNoise* mMainNoise;
    PerlinSimplexNoise* mSurfaceNoise;
    PerlinNoise* mScaleNoise;
    PerlinNoise* mDepthNoise;
    PerlinNoise* mUnusedNoiseA;
    PerlinNoise* mUnusedNoiseB;
    PerlinNoise* mForestNoise;
    Level* mLevel;
    bool mGenerateStructures;
    LevelType* mGeneratorType;
    float mBiomeWeights[25];
    CustomizableSourceSettings* mSettings;
    const BlockState* mOceanBlock;
    arrayWithLength<double> mSurfaceDepths;
    LargeCaveFeature* mCaves;
    StrongholdFeature* mStronghold;
    VillageFeature* mVillage;
    MineShaftFeature* mMineshaft;
    RandomScatteredLargeFeature* mTemple;
    CanyonFeature* mRavines;
    UnderwaterLargeCaveFeature* mUnderwaterCaves;
    UnderwaterCanyonFeature* mUnderwaterRavines;
    OceanMonumentFeature* mMonument;
    WoodlandMansionFeature* mMansion;
    BuriedTreasureFeature* mBuriedTreasure;
    ShipwreckFeature* mShipwreck;
    OceanRuinFeature* mOceanRuin;
};

#endif
