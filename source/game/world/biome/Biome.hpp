#ifndef GAME_WORLD_BIOME_BIOME_HPP
#define GAME_WORLD_BIOME_BIOME_HPP

#include <string>
#include <vector>

#include "game/core/ColourTable.hpp"
#include "game/item/WeighedRandomItem.hpp"
#include "game/world/block/FlowerBlock.hpp"
#include "game/world/generation/OceanRuinFeature.hpp"

class AbstractTreeFeature;
class BiomeDecorator;
class BlockPos;
class ChunkPrimer;
class DoublePlantFeature;
class Feature;
class Level;
class MobCategory;
class PerlinSimplexNoise;
class Random;
class Vec3;

class Biome
{
public:
    enum EBiomeIDs
    {
        eBiomeIDs_Ocean = 0,
        eBiomeIDs_Plains = 1,
        eBiomeIDs_Desert = 2,
        eBiomeIDs_ExtremeHills = 3,
        eBiomeIDs_Forest = 4,
        eBiomeIDs_Taiga = 5,
        eBiomeIDs_Swampland = 6,
        eBiomeIDs_River = 7,
        eBiomeIDs_Hell = 8,
        eBiomeIDs_TheEnd = 9,
        eBiomeIDs_LegacyFrozenOcean = 10,
        eBiomeIDs_FrozenRiver = 11,
        eBiomeIDs_IcePlains = 12,
        eBiomeIDs_IceMountains = 13,
        eBiomeIDs_MushroomIsland = 14,
        eBiomeIDs_MushroomIslandShore = 15,
        eBiomeIDs_Beach = 16,
        eBiomeIDs_DesertHills = 17,
        eBiomeIDs_ForestHills = 18,
        eBiomeIDs_TaigaHills = 19,
        eBiomeIDs_ExtremeHillsEdge = 20,
        eBiomeIDs_Jungle = 21,
        eBiomeIDs_JungleHills = 22,
        eBiomeIDs_JungleEdge = 23,
        eBiomeIDs_DeepOcean = 24,
        eBiomeIDs_StoneBeach = 25,
        eBiomeIDs_ColdBeach = 26,
        eBiomeIDs_BirchForest = 27,
        eBiomeIDs_BirchForestHills = 28,
        eBiomeIDs_RoofedForest = 29,
        eBiomeIDs_ColdTaiga = 30,
        eBiomeIDs_ColdTaigaHills = 31,
        eBiomeIDs_MegaTaiga = 32,
        eBiomeIDs_MegaTaigaHills = 33,
        eBiomeIDs_ExtremeHillsPlusTrees = 34,
        eBiomeIDs_Savanna = 35,
        eBiomeIDs_SavannaPlateau = 36,
        eBiomeIDs_Mesa = 37,
        eBiomeIDs_MesaRock = 38,
        eBiomeIDs_MesaClearRock = 39,
        eBiomeIDs_WarmOcean = 40,
        eBiomeIDs_DeepWarmOcean = 41,
        eBiomeIDs_LukewarmOcean = 42,
        eBiomeIDs_DeepLukewarmOcean = 43,
        eBiomeIDs_ColdOcean = 44,
        eBiomeIDs_DeepColdOcean = 45,
        eBiomeIDs_FrozenOcean = 46,
        eBiomeIDs_DeepFrozenOcean = 47,
        eBiomeIDs_TheVoid = 127,
        eBiomeIDs_SunflowerPlains = 129,
        eBiomeIDs_DesertMutated = 130,
        eBiomeIDs_ExtremeHillsMutated = 131,
        eBiomeIDs_FlowerForest = 132,
        eBiomeIDs_TaigaMutated = 133,
        eBiomeIDs_SwamplandMutated = 134,
        eBiomeIDs_IcePlainsSpikes = 140,
        eBiomeIDs_JungleMutated = 149,
        eBiomeIDs_JungleEdgeMutated = 151,
        eBiomeIDs_BirchForestMutated = 155,
        eBiomeIDs_BirchForestHillsMutated = 156,
        eBiomeIDs_RoofedForestMutated = 157,
        eBiomeIDs_ColdTaigaMutated = 158,
        eBiomeIDs_MegaSpruceTaiga = 160,
        eBiomeIDs_RedwoodTaigaHillsMutated = 161,
        eBiomeIDs_ExtremeHillsPlusTreesMutated = 162,
        eBiomeIDs_SavannaMutated = 163,
        eBiomeIDs_SavannaPlateauMutated = 164,
        eBiomeIDs_MesaBryce = 165,
        eBiomeIDs_MesaRockMutated = 166,
        eBiomeIDs_MesaClearRockMutated = 167
    };

    enum BiomeType
    {
        eBiomeType_Beach = 0,
        eBiomeType_Desert = 1,
        eBiomeType_ExtremeHills = 2,
        eBiomeType_ExtremeHillsEdge = 3,
        eBiomeType_Forest = 4,
        eBiomeType_Hell = 5,
        eBiomeType_Ice = 6,
        eBiomeType_Jungle = 7,
        eBiomeType_Mesa = 8,
        eBiomeType_MushroomIsland = 9,
        eBiomeType_Ocean = 10,
        eBiomeType_Plains = 11,
        eBiomeType_River = 12,
        eBiomeType_Savanna = 13,
        eBiomeType_StoneBeach = 14,
        eBiomeType_Swamp = 15,
        eBiomeType_Taiga = 16,
        eBiomeType_TheEnd = 17,
        eBiomeType_Void = 18
    };

    enum EBiomeBaseClass
    {
        eBiomeBaseClass_Beach = 0,
        eBiomeBaseClass_Desert = 1,
        eBiomeBaseClass_ExtremeHills = 2,
        eBiomeBaseClass_Forest = 3,
        eBiomeBaseClass_Hell = 4,
        eBiomeBaseClass_Ice = 5,
        eBiomeBaseClass_Jungle = 6,
        eBiomeBaseClass_Mesa = 7,
        eBiomeBaseClass_MushroomIsland = 8,
        eBiomeBaseClass_Ocean = 10,
        eBiomeBaseClass_Plains = 11,
        eBiomeBaseClass_River = 13,
        eBiomeBaseClass_Savanna = 14,
        eBiomeBaseClass_StoneBeach = 15,
        eBiomeBaseClass_Swamp = 16,
        eBiomeBaseClass_Taiga = 17,
        eBiomeBaseClass_TheEnd = 18,
        eBiomeBaseClass_OceanCold = 19,
        eBiomeBaseClass_OceanFrozen = 20,
        eBiomeBaseClass_OceanLukewarm = 21,
        eBiomeBaseClass_OceanWarm = 22,
        eBiomeBaseClass_Void = 23
    };

    enum TemperatureCategory
    {
        eTemperatureCategory_Ocean = 0,
        eTemperatureCategory_Cold = 1,
        eTemperatureCategory_Medium = 2,
        eTemperatureCategory_Warm = 3
    };

    class BlockPair
    {
    public:
        int blockId;
        int blockData;

        BlockPair(int blockId = 0, int blockData = 0) : blockId(blockId), blockData(blockData) {}
    };

    class BiomeHeight
    {
    public:
        float depth;
        float scale;

        BiomeHeight(float depth, float scale) : depth(depth), scale(scale) {}
    };

    class BiomeProperties
    {
    public:
        BiomeProperties(const std::wstring& name);

        BiomeProperties* setDepthAndScale(const BiomeHeight& height);
        BiomeProperties* setTemperatureAndDownfall(float temperature, float downfall);
        BiomeProperties* scale(float scale);
        BiomeProperties* depth(float depth);
        BiomeProperties* dry();
        BiomeProperties* waterColor(int waterColour);
        BiomeProperties* snow();
        BiomeProperties* mutated(int baseBiomeId);

        std::wstring mName;
        float mDepth;
        float mScale;
        float mTemperature;
        float mDownfall;
        int mWaterColour;
        bool mSnowEnabled;
        bool mRainEnabled;
        int mBaseBiomeId;
    };

    class MobSpawnerData : public WeighedRandomItem
    {
    public:
        MobSpawnerData(int weight, unsigned long long entityBaseClass, int minCount, int maxCount)
            : WeighedRandomItem(weight),
              mEntityBaseClass(entityBaseClass),
              mMinCount(minCount),
              mMaxCount(maxCount)
        {
        }

        virtual ~MobSpawnerData();

        unsigned long long mEntityBaseClass;
        int mMinCount;
        int mMaxCount;
    };

    static const BlockPair STONE_BLOCK;
    static const BlockPair BEDROCK_BLOCK;
    static const BlockPair WATER_BLOCK;

    static PerlinSimplexNoise* TEMPERATURE_NOISE;
    static PerlinSimplexNoise* BIOME_INFO_NOISE;
    static DoublePlantFeature* DOUBLE_PLANT_FEATURE;

    static const BiomeHeight HEIGHT_DEFAULT;
    static const BiomeHeight HEIGHT_SHALLOW_WATERS;
    static const BiomeHeight HEIGHT_OCEANS;
    static const BiomeHeight HEIGHT_DEEP_OCEANS;
    static const BiomeHeight HEIGHT_LOW_PLAINS;
    static const BiomeHeight HEIGHT_MID_PLAINS;
    static const BiomeHeight HEIGHT_LOW_HILLS;
    static const BiomeHeight HEIGHT_HIGH_PLATEAUS;
    static const BiomeHeight HEIGHT_MID_HILLS;
    static const BiomeHeight HEIGHT_SHORES;
    static const BiomeHeight HEIGHT_ROCKY_WATERS;
    static const BiomeHeight HEIGHT_LOW_ISLANDS;
    static const BiomeHeight HEIGHT_PARTIALLY_SUBMERGED;

    static Biome* PLAINS;
    static Biome* DESERT;
    static Biome* EXTREME_HILLS;
    static Biome* FOREST;
    static Biome* TAIGA;
    static Biome* SWAMPLAND;
    static Biome* RIVER;
    static Biome* HELL;
    static Biome* THE_END;
    static Biome* LEGACY_FROZEN_OCEAN;
    static Biome* FROZEN_RIVER;
    static Biome* ICE_PLAINS;
    static Biome* ICE_MOUNTAINS;
    static Biome* MUSHROOM_ISLAND;
    static Biome* MUSHROOM_ISLAND_SHORE;
    static Biome* BEACH;
    static Biome* DESERT_HILLS;
    static Biome* FOREST_HILLS;
    static Biome* TAIGA_HILLS;
    static Biome* EXTREME_HILLS_EDGE;
    static Biome* JUNGLE;
    static Biome* JUNGLE_HILLS;
    static Biome* JUNGLE_EDGE;
    static Biome* STONE_BEACH;
    static Biome* COLD_BEACH;
    static Biome* BIRCH_FOREST;
    static Biome* BIRCH_FOREST_HILLS;
    static Biome* ROOFED_FOREST;
    static Biome* COLD_TAIGA;
    static Biome* COLD_TAIGA_HILLS;
    static Biome* MEGA_TAIGA;
    static Biome* MEGA_TAIGA_HILLS;
    static Biome* EXTREME_HILLS_PLUS_TREES;
    static Biome* SAVANNA;
    static Biome* SAVANNA_PLATEAU;
    static Biome* MESA;
    static Biome* MESA_ROCK;
    static Biome* MESA_CLEAR_ROCK;
    static Biome* OCEAN;
    static Biome* DEEP_OCEAN;
    static Biome* COLD_OCEAN;
    static Biome* DEEP_COLD_OCEAN;
    static Biome* FROZEN_OCEAN;
    static Biome* DEEP_FROZEN_OCEAN;
    static Biome* LUKEWARM_OCEAN;
    static Biome* DEEP_LUKEWARM_OCEAN;
    static Biome* WARM_OCEAN;
    static Biome* DEEP_WARM_OCEAN;
    static Biome* THE_VOID;
    static Biome* SUNFLOWER_PLAINS;
    static Biome* DESERT_MUTATED;
    static Biome* EXTREME_HILLS_MUTATED;
    static Biome* FLOWER_FOREST;
    static Biome* TAIGA_MUTATED;
    static Biome* SWAMPLAND_MUTATED;
    static Biome* ICE_PLAINS_SPIKES;
    static Biome* JUNGLE_MUTATED;
    static Biome* JUNGLE_EDGE_MUTATED;
    static Biome* BIRCH_FOREST_MUTATED;
    static Biome* BIRCH_FOREST_HILLS_MUTATED;
    static Biome* ROOFED_FOREST_MUTATED;
    static Biome* COLD_TAIGA_MUTATED;
    static Biome* MEGA_SPRUCE_TAIGA;
    static Biome* REDWOOD_TAIGA_HILLS_MUTATED;
    static Biome* EXTREME_HILLS_PLUS_TREES_MUTATED;
    static Biome* SAVANNA_MUTATED;
    static Biome* SAVANNA_PLATEAU_MUTATED;
    static Biome* MESA_BRYCE;
    static Biome* MESA_ROCK_MUTATED;
    static Biome* MESA_CLEAR_ROCK_MUTATED;
    static Biome* DEFAULT;

    static const int MAX_BIOME_ID = 256;
    static Biome* biomes[MAX_BIOME_ID + 1];

    Biome(EBiomeIDs biomeId, BiomeType biomeType, BiomeProperties* properties);
    virtual ~Biome();

    static void staticCtor();

    static int getId(Biome* biome);
    static Biome* byId(int biomeId);
    static Biome* byNameId(std::wstring name);
    static Biome** getBiomes();
    static Biome* getBiome(int biomeId, Biome* fallback);
    static Biome* getBiome(int biomeId);
    static Biome* getMutatedVariant(Biome* biome);
    static void generateColoursDebugOutput();

    BiomeDecorator* createDecorator();

    Biome* setWaterSkyColor(eMinecraftColour waterColour, eMinecraftColour skyColour);
    Biome* setNameAndDescription(int nameId, int descriptionId);
    Biome* setPreviewColor(eMinecraftColour previewColour);
    Biome* setRunDepthAndFloorBlock(unsigned int runDepth, unsigned int floorDepth, BlockPair floorBlock);
    Biome* setSnowAccumulation(float minAccumulation, float maxAccumulation);
    Biome* setSnowCovered();

    BiomeType getBiomeType() const;
    eMinecraftColour getPreviewColor();
    void clearDefaultFriendlies();
    int getMinSnowLayers() const;
    int getMaxSnowLayers() const;
    int getSnowAccumulationLayers() const;
    std::vector<MobSpawnerData*>* getMobs(MobCategory* category);
    bool isSnowCovered();
    bool isMutated();
    float getDownfall();
    float getDepth();
    float getScale();
    float getDefaultBiomeTemperature() const;
    std::wstring getName(bool useShortName);
    std::wstring getDescription();
    std::wstring getIdName();
    OceanRuinConfiguration getOceanRuinConfig() const;

    virtual AbstractTreeFeature* getTreeFeature(Random& random);
    virtual Feature* getGrassFeature(Random& random);
    virtual FlowerBlock::Flower* getRandomFlower(Random& random, const BlockPos& pos);
    virtual unsigned int getSkyColor(float temperature);
    virtual bool hasSnow();
    virtual bool hasRain();
    virtual bool isHumid();
    virtual float getCreatureProbability();
    virtual float getTemperature(const BlockPos& pos);
    float getTemperature() const;
    virtual void decorate(Level* level, Random& random, const BlockPos& pos);
    virtual unsigned int getGrassColor(const BlockPos& pos);
    virtual unsigned int getFoliageColor(const BlockPos& pos);
    virtual void setGrassColor(const BlockPos& pos, unsigned int colour);
    virtual void setFoliageColor(const BlockPos& pos, unsigned int colour);
    virtual void buildSurfaceAt(Level* level, Random& random, ChunkPrimer* primer, int x, int z, double noise);
    virtual void buildSurfaceAtDefault(Level* level, Random& random, ChunkPrimer* primer, int x, int z, double noise);
    virtual EBiomeBaseClass getBaseClass() = 0;
    virtual TemperatureCategory getTemperatureCategory() const;
    virtual bool isAlwaysValidSpawn();
    virtual unsigned int getWaterColor();
    virtual unsigned int getUnderWaterFogColor();
    virtual float getFogDistance();
    virtual Vec3* getFogColor(float celestialAngle, float partialTicks) const;
    virtual bool isFoggy();
    virtual bool isNatural();

    int mNameId;
    int mDescriptionId;
    float mDepth;
    float mScale;
    float mTemperature;
    float mDownfall;
    bool mSnowEnabled;
    bool mRainEnabled;
    int mBaseBiomeId;
    std::wstring mBiomeName;
    BlockPair mTopBlock;
    BlockPair mFillerBlock;
    BiomeDecorator* mDecorator;
    int id;
    std::vector<MobSpawnerData*> mMonsters;
    std::vector<MobSpawnerData*> mCreatures;
    std::vector<MobSpawnerData*> mWaterCreatures;
    std::vector<MobSpawnerData*> mChickens;
    std::vector<MobSpawnerData*> mWolves;
    std::vector<MobSpawnerData*> mSquid;
    std::vector<MobSpawnerData*> mMushroomCows;
    std::vector<MobSpawnerData*> mSlimes;
    std::vector<MobSpawnerData*> mAmbientCreatures;
    eMinecraftColour mWaterColour;
    eMinecraftColour mSkyColour;
    eMinecraftColour mPreviewColour;
    OceanRuinConfiguration mOceanRuinConfig;
    BiomeType mBiomeType;
    float mSnowAccumulation;
    float mSnowMeltRate;
    float mMinSnowAccumulation;
    float mMaxSnowAccumulation;
    unsigned int mRunDepth;
    unsigned int mFloorDepth;
    BlockPair mFloorBlock;
};

#endif
