#include "game/world/biome/Biome.hpp"
#include "game/core/StringIds.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/FoliageColor.hpp"
#include "game/core/GrassColor.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Mth.hpp"
#include "game/core/PerlinSimplexNoise.hpp"
#include "game/entity/mob/MobCategory.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/Minecraft.hpp"
#include "game/util/Random.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/biome/BeachBiome.hpp"
#include "game/world/biome/BiomeDecorator.hpp"
#include "game/world/biome/DesertBiome.hpp"
#include "game/world/biome/ExtremeHillsBiome.hpp"
#include "game/world/biome/ForestBiome.hpp"
#include "game/world/biome/HellBiome.hpp"
#include "game/world/biome/IceBiome.hpp"
#include "game/world/biome/JungleBiome.hpp"
#include "game/world/biome/MesaBiome.hpp"
#include "game/world/biome/MushroomIslandBiome.hpp"
#include "game/world/biome/MutatedForestBiome.hpp"
#include "game/world/biome/MutatedSavannaBiome.hpp"
#include "game/world/biome/OceanBiome.hpp"
#include "game/world/biome/OceanColdBiome.hpp"
#include "game/world/biome/OceanFrozenBiome.hpp"
#include "game/world/biome/OceanLukewarmBiome.hpp"
#include "game/world/biome/OceanWarmBiome.hpp"
#include "game/world/biome/PlainsBiome.hpp"
#include "game/world/biome/RiverBiome.hpp"
#include "game/world/biome/SavannaBiome.hpp"
#include "game/world/biome/StoneBeachBiome.hpp"
#include "game/world/biome/SwampBiome.hpp"
#include "game/world/biome/TaigaBiome.hpp"
#include "game/world/biome/TheEndBiome.hpp"
#include "game/world/biome/VoidBiome.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/FlowerBlock.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/SandBlock.hpp"
#include "game/world/block/TallGrassBlock.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/generation/BigTreeFeature.hpp"
#include "game/world/generation/DoublePlantFeature.hpp"
#include "game/world/generation/TallgrassFeature.hpp"
#include "game/world/generation/TreeFeature.hpp"
#include "game/core/PIXNamedEvents.hpp"

#include <boost/unordered_map.hpp>

typedef boost::unordered_map<std::wstring, Biome*> BiomeNameMap;

const Biome::BlockPair Biome::STONE_BLOCK(1, 0);
static const Biome::BlockPair AIR_BLOCK(0, 0);
const Biome::BlockPair Biome::BEDROCK_BLOCK(7, 0);
static const Biome::BlockPair GRAVEL_BLOCK(13, 0);
static const Biome::BlockPair RED_SANDSTONE_BLOCK(179, 0);
static const Biome::BlockPair SANDSTONE_BLOCK(24, 0);
static const Biome::BlockPair ICE_BLOCK(79, 0);
const Biome::BlockPair Biome::WATER_BLOCK(9, 0);

Biome* Biome::biomes[Biome::MAX_BIOME_ID + 1];
static BiomeNameMap sBiomesByName;
PerlinSimplexNoise* Biome::TEMPERATURE_NOISE = new PerlinSimplexNoise(new Random(1234LL), 1);
PerlinSimplexNoise* Biome::BIOME_INFO_NOISE = new PerlinSimplexNoise(new Random(2345LL), 1);
DoublePlantFeature* Biome::DOUBLE_PLANT_FEATURE = new DoublePlantFeature();

const Biome::BiomeHeight Biome::HEIGHT_DEFAULT(0.1F, 0.2F);
const Biome::BiomeHeight Biome::HEIGHT_SHALLOW_WATERS(-0.5F, 0.0F);
const Biome::BiomeHeight Biome::HEIGHT_OCEANS(-1.0F, 0.1F);
const Biome::BiomeHeight Biome::HEIGHT_DEEP_OCEANS(-1.8F, 0.1F);
const Biome::BiomeHeight Biome::HEIGHT_LOW_PLAINS(0.125F, 0.05F);
const Biome::BiomeHeight Biome::HEIGHT_MID_PLAINS(0.2F, 0.2F);
const Biome::BiomeHeight Biome::HEIGHT_LOW_HILLS(0.45F, 0.3F);
const Biome::BiomeHeight Biome::HEIGHT_HIGH_PLATEAUS(1.5F, 0.025F);
const Biome::BiomeHeight Biome::HEIGHT_MID_HILLS(1.0F, 0.5F);
const Biome::BiomeHeight Biome::HEIGHT_SHORES(0.0F, 0.025F);
const Biome::BiomeHeight Biome::HEIGHT_ROCKY_WATERS(0.1F, 0.8F);
const Biome::BiomeHeight Biome::HEIGHT_LOW_ISLANDS(0.2F, 0.3F);
const Biome::BiomeHeight Biome::HEIGHT_PARTIALLY_SUBMERGED(-0.2F, 0.1F);

Biome* Biome::PLAINS;
Biome* Biome::DESERT;
Biome* Biome::EXTREME_HILLS;
Biome* Biome::FOREST;
Biome* Biome::TAIGA;
Biome* Biome::SWAMPLAND;
Biome* Biome::RIVER;
Biome* Biome::HELL;
Biome* Biome::THE_END;
Biome* Biome::LEGACY_FROZEN_OCEAN;
Biome* Biome::FROZEN_RIVER;
Biome* Biome::ICE_PLAINS;
Biome* Biome::ICE_MOUNTAINS;
Biome* Biome::MUSHROOM_ISLAND;
Biome* Biome::MUSHROOM_ISLAND_SHORE;
Biome* Biome::BEACH;
Biome* Biome::DESERT_HILLS;
Biome* Biome::FOREST_HILLS;
Biome* Biome::TAIGA_HILLS;
Biome* Biome::EXTREME_HILLS_EDGE;
Biome* Biome::JUNGLE;
Biome* Biome::JUNGLE_HILLS;
Biome* Biome::JUNGLE_EDGE;
Biome* Biome::STONE_BEACH;
Biome* Biome::COLD_BEACH;
Biome* Biome::BIRCH_FOREST;
Biome* Biome::BIRCH_FOREST_HILLS;
Biome* Biome::ROOFED_FOREST;
Biome* Biome::COLD_TAIGA;
Biome* Biome::COLD_TAIGA_HILLS;
Biome* Biome::MEGA_TAIGA;
Biome* Biome::MEGA_TAIGA_HILLS;
Biome* Biome::EXTREME_HILLS_PLUS_TREES;
Biome* Biome::SAVANNA;
Biome* Biome::SAVANNA_PLATEAU;
Biome* Biome::MESA;
Biome* Biome::MESA_ROCK;
Biome* Biome::MESA_CLEAR_ROCK;
Biome* Biome::OCEAN;
Biome* Biome::DEEP_OCEAN;
Biome* Biome::COLD_OCEAN;
Biome* Biome::DEEP_COLD_OCEAN;
Biome* Biome::FROZEN_OCEAN;
Biome* Biome::DEEP_FROZEN_OCEAN;
Biome* Biome::LUKEWARM_OCEAN;
Biome* Biome::DEEP_LUKEWARM_OCEAN;
Biome* Biome::WARM_OCEAN;
Biome* Biome::DEEP_WARM_OCEAN;
Biome* Biome::THE_VOID;
Biome* Biome::SUNFLOWER_PLAINS;
Biome* Biome::DESERT_MUTATED;
Biome* Biome::EXTREME_HILLS_MUTATED;
Biome* Biome::FLOWER_FOREST;
Biome* Biome::TAIGA_MUTATED;
Biome* Biome::SWAMPLAND_MUTATED;
Biome* Biome::ICE_PLAINS_SPIKES;
Biome* Biome::JUNGLE_MUTATED;
Biome* Biome::JUNGLE_EDGE_MUTATED;
Biome* Biome::BIRCH_FOREST_MUTATED;
Biome* Biome::BIRCH_FOREST_HILLS_MUTATED;
Biome* Biome::ROOFED_FOREST_MUTATED;
Biome* Biome::COLD_TAIGA_MUTATED;
Biome* Biome::MEGA_SPRUCE_TAIGA;
Biome* Biome::REDWOOD_TAIGA_HILLS_MUTATED;
Biome* Biome::EXTREME_HILLS_PLUS_TREES_MUTATED;
Biome* Biome::SAVANNA_MUTATED;
Biome* Biome::SAVANNA_PLATEAU_MUTATED;
Biome* Biome::MESA_BRYCE;
Biome* Biome::MESA_ROCK_MUTATED;
Biome* Biome::MESA_CLEAR_ROCK_MUTATED;
Biome* Biome::DEFAULT;

Biome::BiomeProperties::BiomeProperties(const std::wstring& name)
{
    mName = name;
    mSnowEnabled = false;
    mWaterColour = 0xFFFFFF;
    mDownfall = 0.5F;
    mDepth = 0.1F;
    mTemperature = 0.5F;
    mRainEnabled = true;
    mScale = 0.2F;
    mBaseBiomeId = -1;
}

Biome::BiomeProperties* Biome::BiomeProperties::setDepthAndScale(const BiomeHeight& height)
{
    mDepth = height.depth;
    mScale = height.scale;

    return this;
}

Biome::BiomeProperties* Biome::BiomeProperties::setTemperatureAndDownfall(float temperature, float downfall)
{
    mTemperature = temperature;
    mDownfall = downfall;

    return this;
}

Biome::BiomeProperties* Biome::BiomeProperties::scale(float scale)
{
    mScale = scale;

    return this;
}

Biome::BiomeProperties* Biome::BiomeProperties::depth(float depth)
{
    mDepth = depth;

    return this;
}

Biome::BiomeProperties* Biome::BiomeProperties::dry()
{
    mRainEnabled = false;

    return this;
}

Biome::BiomeProperties* Biome::BiomeProperties::waterColor(int waterColour)
{
    mWaterColour = waterColour;

    return this;
}

Biome::BiomeProperties* Biome::BiomeProperties::snow()
{
    mSnowEnabled = true;

    return this;
}

Biome::BiomeProperties* Biome::BiomeProperties::mutated(int baseBiomeId)
{
    mBaseBiomeId = baseBiomeId;

    return this;
}

Biome::MobSpawnerData::~MobSpawnerData()
{
}

Biome::Biome(EBiomeIDs biomeId, BiomeType biomeType, BiomeProperties* properties)
    : mTopBlock(),
      mFillerBlock(),
      id(biomeId),
      mOceanRuinConfig(),
      mBiomeType(biomeType),
      mFloorBlock()
{
    mPreviewColour = eMinecraftColour_Preview_Plains;
    mTopBlock.blockId = Block::getId(Blocks::grass);
    mTopBlock.blockData = Blocks::grass->convertBlockStateToLegacyData(Blocks::grass->defaultBlockState());
    mFillerBlock.blockId = Block::getId(Blocks::dirt);
    mFillerBlock.blockData = Blocks::dirt->convertBlockStateToLegacyData(Blocks::dirt->defaultBlockState());

    mSnowAccumulation = 0.0F;
    mSnowMeltRate = 0.0F;
    mMinSnowAccumulation = 0.0F;
    mMaxSnowAccumulation = 0.0F;
    mRunDepth = 7;
    mFloorDepth = 1;
    mFloorBlock = GRAVEL_BLOCK;

    mBiomeName = properties->mName;
    mDepth = properties->mDepth;
    mScale = properties->mScale;
    mTemperature = properties->mTemperature;
    mDownfall = properties->mDownfall;
    mSnowEnabled = properties->mSnowEnabled;
    mRainEnabled = properties->mRainEnabled;
    mBaseBiomeId = properties->mBaseBiomeId;

    delete properties;

    biomes[biomeId] = this;
    mWaterColour = eMinecraftColour_NOTSET;
    sBiomesByName[mBiomeName] = this;

    mDecorator = createDecorator();

    mCreatures.push_back(new MobSpawnerData(12, EntityType::SHEEP, 4, 4));
    mCreatures.push_back(new MobSpawnerData(10, EntityType::PIG, 4, 4));
    mChickens.push_back(new MobSpawnerData(10, EntityType::CHICKEN, 4, 4));
    mCreatures.push_back(new MobSpawnerData(8, EntityType::COW, 4, 4));

    mMonsters.push_back(new MobSpawnerData(100, EntityType::SPIDER, 4, 4));
    mMonsters.push_back(new MobSpawnerData(95, EntityType::ZOMBIE, 4, 4));
    mMonsters.push_back(new MobSpawnerData(5, EntityType::ZOMBIE_VILLAGER, 1, 1));
    mMonsters.push_back(new MobSpawnerData(100, EntityType::SKELETON, 4, 4));
    mMonsters.push_back(new MobSpawnerData(100, EntityType::CREEPER, 4, 4));
    mMonsters.push_back(new MobSpawnerData(100, EntityType::PHANTOM, 1, 1));

    if (biomeId != eBiomeIDs_Hell && biomeId != eBiomeIDs_TheEnd)
    {
        mSlimes.push_back(new MobSpawnerData(100, EntityType::SLIME, 4, 4));
    }

    mMonsters.push_back(new MobSpawnerData(10, EntityType::ENDER_MAN, 1, 4));
    mMonsters.push_back(new MobSpawnerData(5, EntityType::WITCH, 1, 1));

    mAmbientCreatures.push_back(new MobSpawnerData(10, EntityType::BAT, 8, 8));

    mNameId = -1;
}

Biome::~Biome()
{
    delete mDecorator;
}

void Biome::staticCtor()
{
    OCEAN = new OceanBiome(eBiomeIDs_Ocean, false, (new BiomeProperties(L"ocean"))->setDepthAndScale(HEIGHT_OCEANS));
    OCEAN->setPreviewColor(eMinecraftColour_Preview_Ocean)
        ->setNameAndDescription(IDS_BIOME_OCEAN, IDS_BIOME_DESC_OCEAN)
        ->setWaterSkyColor(eMinecraftColour_Water_Ocean, eMinecraftColour_Sky_Ocean);

    DEEP_OCEAN = new OceanBiome(eBiomeIDs_DeepOcean, true,
                                (new BiomeProperties(L"deep_ocean"))->setDepthAndScale(HEIGHT_DEEP_OCEANS));
    DEEP_OCEAN->setPreviewColor(eMinecraftColour_Preview_DeepOcean)
        ->setNameAndDescription(IDS_BIOME_OCEAN, IDS_BIOME_DESC_OCEAN)
        ->setWaterSkyColor(eMinecraftColour_Water_DeepOcean, eMinecraftColour_Sky_DeepOcean);

    WARM_OCEAN = new OceanWarmBiome(
        eBiomeIDs_WarmOcean, false,
        (new BiomeProperties(L"warm_ocean"))->setDepthAndScale(HEIGHT_OCEANS)->setTemperatureAndDownfall(0.5F, 0.5F));
    WARM_OCEAN->setRunDepthAndFloorBlock(3, 1, BlockPair(12, 0));
    WARM_OCEAN->setPreviewColor(eMinecraftColour_Preview_Ocean)
        ->setNameAndDescription(IDS_BIOME_WARM_OCEAN, IDS_BIOME_DESC_WARM_OCEAN)
        ->setWaterSkyColor(eMinecraftColour_Water_Ocean, eMinecraftColour_Sky_Ocean);

    DEEP_WARM_OCEAN = new OceanWarmBiome(eBiomeIDs_DeepWarmOcean, true,
                                         (new BiomeProperties(L"deep_warm_ocean"))
                                             ->setDepthAndScale(HEIGHT_DEEP_OCEANS)
                                             ->setTemperatureAndDownfall(0.5F, 0.5F));
    DEEP_WARM_OCEAN->setRunDepthAndFloorBlock(3, 1, BlockPair(12, 0));
    DEEP_WARM_OCEAN->setPreviewColor(eMinecraftColour_Preview_DeepOcean)
        ->setNameAndDescription(IDS_BIOME_WARM_OCEAN, IDS_BIOME_DESC_WARM_OCEAN)
        ->setWaterSkyColor(eMinecraftColour_Water_DeepOcean, eMinecraftColour_Sky_DeepOcean);

    LUKEWARM_OCEAN = new OceanLukewarmBiome(eBiomeIDs_LukewarmOcean, false,
                                            (new BiomeProperties(L"lukewarm_ocean"))
                                                ->setDepthAndScale(HEIGHT_OCEANS)
                                                ->setTemperatureAndDownfall(0.5F, 0.5F));
    LUKEWARM_OCEAN->setRunDepthAndFloorBlock(3, 1, BlockPair(12, 0));
    LUKEWARM_OCEAN->setPreviewColor(eMinecraftColour_Preview_Ocean)
        ->setNameAndDescription(IDS_BIOME_LUKEWARM_OCEAN, IDS_BIOME_DESC_LUKEWARM_OCEAN)
        ->setWaterSkyColor(eMinecraftColour_Water_Ocean, eMinecraftColour_Sky_Ocean);

    DEEP_LUKEWARM_OCEAN = new OceanLukewarmBiome(eBiomeIDs_DeepLukewarmOcean, true,
                                                 (new BiomeProperties(L"deep_lukewarm_ocean"))
                                                     ->setDepthAndScale(HEIGHT_DEEP_OCEANS)
                                                     ->setTemperatureAndDownfall(0.5F, 0.5F));
    DEEP_LUKEWARM_OCEAN->setRunDepthAndFloorBlock(7, 1, BlockPair(12, 0));
    DEEP_LUKEWARM_OCEAN->setPreviewColor(eMinecraftColour_Preview_DeepOcean)
        ->setNameAndDescription(IDS_BIOME_LUKEWARM_OCEAN, IDS_BIOME_DESC_LUKEWARM_OCEAN)
        ->setWaterSkyColor(eMinecraftColour_Water_DeepOcean, eMinecraftColour_Sky_DeepOcean);

    COLD_OCEAN = new OceanColdBiome(
        eBiomeIDs_ColdOcean, false,
        (new BiomeProperties(L"cold_ocean"))->setDepthAndScale(HEIGHT_OCEANS)->setTemperatureAndDownfall(0.5F, 0.5F));
    COLD_OCEAN->setPreviewColor(eMinecraftColour_Preview_Ocean)
        ->setNameAndDescription(IDS_BIOME_COLD_OCEAN, IDS_BIOME_DESC_COLD_OCEAN)
        ->setWaterSkyColor(eMinecraftColour_Water_Ocean, eMinecraftColour_Sky_Ocean);

    DEEP_COLD_OCEAN = new OceanColdBiome(eBiomeIDs_DeepColdOcean, true,
                                         (new BiomeProperties(L"deep_cold_ocean"))
                                             ->setDepthAndScale(HEIGHT_DEEP_OCEANS)
                                             ->setTemperatureAndDownfall(0.5F, 0.5F));
    DEEP_COLD_OCEAN->setPreviewColor(eMinecraftColour_Preview_DeepOcean)
        ->setNameAndDescription(IDS_BIOME_COLD_OCEAN, IDS_BIOME_DESC_COLD_OCEAN)
        ->setWaterSkyColor(eMinecraftColour_Water_DeepOcean, eMinecraftColour_Sky_DeepOcean);

    FROZEN_OCEAN = new OceanFrozenBiome(
        eBiomeIDs_FrozenOcean, false,
        (new BiomeProperties(L"frozen_ocean"))->setDepthAndScale(HEIGHT_OCEANS)->setTemperatureAndDownfall(0.0F, 0.5F));
    FROZEN_OCEAN->setSnowAccumulation(0.125F, 0.25F);
    FROZEN_OCEAN->setPreviewColor(eMinecraftColour_Preview_Ocean)
        ->setNameAndDescription(IDS_BIOME_FROZEN_OCEAN, IDS_BIOME_DESC_FROZEN_OCEAN)
        ->setWaterSkyColor(eMinecraftColour_Water_Ocean, eMinecraftColour_Sky_Ocean);

    DEEP_FROZEN_OCEAN = new OceanFrozenBiome(eBiomeIDs_DeepFrozenOcean, true,
                                             (new BiomeProperties(L"deep_frozen_ocean"))
                                                 ->setDepthAndScale(HEIGHT_DEEP_OCEANS)
                                                 ->setTemperatureAndDownfall(0.0F, 0.5F));
    DEEP_FROZEN_OCEAN->setSnowAccumulation(0.125F, 0.25F);
    DEEP_FROZEN_OCEAN->setPreviewColor(eMinecraftColour_Preview_DeepOcean)
        ->setNameAndDescription(IDS_BIOME_FROZEN_OCEAN, IDS_BIOME_DESC_FROZEN_OCEAN)
        ->setWaterSkyColor(eMinecraftColour_Water_DeepOcean, eMinecraftColour_Sky_DeepOcean);

    PLAINS = new PlainsBiome(
        eBiomeIDs_Plains, false,
        (new BiomeProperties(L"plains"))->depth(0.125F)->scale(0.05F)->setTemperatureAndDownfall(0.8F, 0.4F));
    PLAINS->setPreviewColor(eMinecraftColour_Preview_Plains)
        ->setNameAndDescription(IDS_BIOME_PLAINS, IDS_BIOME_DESC_PLAINS)
        ->setWaterSkyColor(eMinecraftColour_Water_Plains, eMinecraftColour_Sky_Plains);

    DESERT = new DesertBiome(
        eBiomeIDs_Desert,
        (new BiomeProperties(L"desert"))->depth(0.125F)->scale(0.05F)->setTemperatureAndDownfall(2.0F, 0.0F)->dry());
    DESERT->setPreviewColor(eMinecraftColour_Preview_Desert)
        ->setNameAndDescription(IDS_BIOME_DESERT, IDS_BIOME_DESC_DESERT)
        ->setWaterSkyColor(eMinecraftColour_Water_Desert, eMinecraftColour_Sky_Desert);

    EXTREME_HILLS = new ExtremeHillsBiome(
        eBiomeIDs_ExtremeHills, ExtremeHillsBiome::eType_Normal,
        (new BiomeProperties(L"extreme_hills"))->depth(1.0F)->scale(0.5F)->setTemperatureAndDownfall(0.2F, 0.3F));
    EXTREME_HILLS->setPreviewColor(eMinecraftColour_Preview_ExtremeHills)
        ->setNameAndDescription(IDS_BIOME_EXTREME_HILLS, IDS_BIOME_DESC_EXTREME_HILLS)
        ->setWaterSkyColor(eMinecraftColour_Water_ExtremeHills, eMinecraftColour_Sky_ExtremeHills);

    FOREST = new ForestBiome(eBiomeIDs_Forest, ForestBiome::eType_Normal,
                             (new BiomeProperties(L"forest"))->setTemperatureAndDownfall(0.7F, 0.8F));
    FOREST->setPreviewColor(eMinecraftColour_Preview_Forest)
        ->setNameAndDescription(IDS_BIOME_FOREST, IDS_BIOME_DESC_FOREST)
        ->setWaterSkyColor(eMinecraftColour_Water_Forest, eMinecraftColour_Sky_Forest);

    TAIGA = new TaigaBiome(
        eBiomeIDs_Taiga, TaigaBiome::eType_Normal,
        (new BiomeProperties(L"taiga"))->depth(0.2F)->scale(0.2F)->setTemperatureAndDownfall(0.25F, 0.8F));
    TAIGA->setPreviewColor(eMinecraftColour_Preview_Taiga)
        ->setNameAndDescription(IDS_BIOME_TAIGA, IDS_BIOME_DESC_TAIGA)
        ->setWaterSkyColor(eMinecraftColour_Water_Taiga, eMinecraftColour_Sky_Taiga);

    SWAMPLAND = new SwampBiome(eBiomeIDs_Swampland, (new BiomeProperties(L"swampland"))
                                                        ->depth(-0.2F)
                                                        ->scale(0.1F)
                                                        ->setTemperatureAndDownfall(0.8F, 0.9F)
                                                        ->waterColor(0xE0FFAE));
    SWAMPLAND->setPreviewColor(eMinecraftColour_Preview_Swampland)
        ->setNameAndDescription(IDS_BIOME_SWAMPLAND, IDS_BIOME_DESC_SWAMPLAND)
        ->setWaterSkyColor(eMinecraftColour_Water_Swampland, eMinecraftColour_Sky_Swampland);

    RIVER = new RiverBiome(eBiomeIDs_River, (new BiomeProperties(L"river"))->depth(-0.5F)->scale(0.0F));
    RIVER->setPreviewColor(eMinecraftColour_Preview_River)
        ->setNameAndDescription(IDS_BIOME_RIVER, IDS_BIOME_DESC_RIVER)
        ->setWaterSkyColor(eMinecraftColour_Water_River, eMinecraftColour_Sky_River);

    HELL = new HellBiome(eBiomeIDs_Hell, (new BiomeProperties(L"hell"))->setTemperatureAndDownfall(2.0F, 0.0F)->dry());
    HELL->setPreviewColor(eMinecraftColour_Preview_Hell)
        ->setNameAndDescription(IDS_BIOME_NETHER, IDS_BIOME_DESC_NETHER)
        ->setWaterSkyColor(eMinecraftColour_Water_Hell, eMinecraftColour_Sky_Hell);

    THE_END = new TheEndBiome(eBiomeIDs_TheEnd, (new BiomeProperties(L"the_end"))->dry());
    THE_END->setPreviewColor(eMinecraftColour_Preview_Sky)
        ->setNameAndDescription(IDS_BIOME_END, IDS_BIOME_DESC_END)
        ->setWaterSkyColor(eMinecraftColour_Water_Sky, eMinecraftColour_Sky_Sky);

    LEGACY_FROZEN_OCEAN = new OceanBiome(eBiomeIDs_LegacyFrozenOcean, false,
                                         (new BiomeProperties(L"legacy_frozen_ocean"))
                                             ->depth(-1.0F)
                                             ->scale(0.1F)
                                             ->setTemperatureAndDownfall(0.0F, 0.5F)
                                             ->snow());
    LEGACY_FROZEN_OCEAN->setPreviewColor(eMinecraftColour_Preview_FrozenOcean)
        ->setNameAndDescription(IDS_BIOME_FROZEN_OCEAN, IDS_BIOME_DESC_OCEAN)
        ->setWaterSkyColor(eMinecraftColour_Water_FrozenOcean, eMinecraftColour_Sky_FrozenOcean);

    FROZEN_RIVER = new RiverBiome(eBiomeIDs_FrozenRiver, (new BiomeProperties(L"frozen_river"))
                                                             ->depth(-0.5F)
                                                             ->scale(0.0F)
                                                             ->setTemperatureAndDownfall(0.0F, 0.5F)
                                                             ->snow());
    FROZEN_RIVER->setPreviewColor(eMinecraftColour_Preview_FrozenRiver)
        ->setNameAndDescription(IDS_BIOME_FROZEN_RIVER, IDS_BIOME_DESC_FROZEN_RIVER)
        ->setWaterSkyColor(eMinecraftColour_Water_FrozenRiver, eMinecraftColour_Sky_FrozenRiver);

    ICE_PLAINS = new IceBiome(eBiomeIDs_IcePlains, false,
                              (new BiomeProperties(L"ice_plains"))
                                  ->depth(0.125F)
                                  ->scale(0.05F)
                                  ->setTemperatureAndDownfall(0.0F, 0.5F)
                                  ->snow());
    ICE_PLAINS->setPreviewColor(eMinecraftColour_Preview_IcePlains)
        ->setNameAndDescription(IDS_BIOME_ICE_FLATS, IDS_BIOME_DESC_ICE_FLATS)
        ->setWaterSkyColor(eMinecraftColour_Water_IcePlains, eMinecraftColour_Sky_IcePlains);

    ICE_MOUNTAINS = new IceBiome(eBiomeIDs_IceMountains, false,
                                 (new BiomeProperties(L"ice_mountains"))
                                     ->depth(0.45F)
                                     ->scale(0.3F)
                                     ->setTemperatureAndDownfall(0.0F, 0.5F)
                                     ->snow());
    ICE_MOUNTAINS->setPreviewColor(eMinecraftColour_Preview_IceMountains)
        ->setNameAndDescription(IDS_BIOME_ICE_MOUNTAINS, IDS_BIOME_DESC_ICE_MOUNTAINS)
        ->setWaterSkyColor(eMinecraftColour_Water_IceMountains, eMinecraftColour_Sky_IceMountains);

    MUSHROOM_ISLAND = new MushroomIslandBiome(
        eBiomeIDs_MushroomIsland,
        (new BiomeProperties(L"mushroom_island"))->depth(0.2F)->scale(0.3F)->setTemperatureAndDownfall(0.9F, 1.0F));
    MUSHROOM_ISLAND->setPreviewColor(eMinecraftColour_Preview_MushroomIsland)
        ->setNameAndDescription(IDS_BIOME_MUSHROOM_ISLAND, IDS_BIOME_DESC_MUSHROOM_ISLAND)
        ->setWaterSkyColor(eMinecraftColour_Water_MushroomIsland, eMinecraftColour_Sky_MushroomIsland);

    MUSHROOM_ISLAND_SHORE = new MushroomIslandBiome(eBiomeIDs_MushroomIslandShore,
                                                    (new BiomeProperties(L"mushroom_island_shore"))
                                                        ->depth(0.0F)
                                                        ->scale(0.025F)
                                                        ->setTemperatureAndDownfall(0.9F, 1.0F));
    MUSHROOM_ISLAND_SHORE->setPreviewColor(eMinecraftColour_Preview_MushroomIslandShore)
        ->setNameAndDescription(IDS_BIOME_MUSHROOM_ISLAND, IDS_BIOME_DESC_MUSHROOM_ISLAND)
        ->setWaterSkyColor(eMinecraftColour_Water_MushroomIslandShore, eMinecraftColour_Sky_MushroomIslandShore);

    BEACH = new BeachBiome(
        eBiomeIDs_Beach,
        (new BiomeProperties(L"beach"))->depth(0.0F)->scale(0.025F)->setTemperatureAndDownfall(0.8F, 0.4F));
    BEACH->setPreviewColor(eMinecraftColour_Preview_Beach)
        ->setNameAndDescription(IDS_BIOME_BEACH, IDS_BIOME_DESC_BEACH)
        ->setWaterSkyColor(eMinecraftColour_Water_Beach, eMinecraftColour_Sky_Beach);

    DESERT_HILLS = new DesertBiome(eBiomeIDs_DesertHills, (new BiomeProperties(L"desert_hills"))
                                                              ->depth(0.45F)
                                                              ->scale(0.3F)
                                                              ->setTemperatureAndDownfall(2.0F, 0.0F)
                                                              ->dry());
    DESERT_HILLS->setPreviewColor(eMinecraftColour_Preview_DesertHills)
        ->setNameAndDescription(IDS_BIOME_DESERT, IDS_BIOME_DESC_DESERT)
        ->setWaterSkyColor(eMinecraftColour_Water_DesertHills, eMinecraftColour_Sky_DesertHills);

    FOREST_HILLS = new ForestBiome(
        eBiomeIDs_ForestHills, ForestBiome::eType_Normal,
        (new BiomeProperties(L"forest_hills"))->depth(0.45F)->scale(0.3F)->setTemperatureAndDownfall(0.7F, 0.8F));
    FOREST_HILLS->setPreviewColor(eMinecraftColour_Preview_ForestHills)
        ->setNameAndDescription(IDS_BIOME_FOREST, IDS_BIOME_DESC_FOREST)
        ->setWaterSkyColor(eMinecraftColour_Water_ForestHills, eMinecraftColour_Sky_ForestHills);

    TAIGA_HILLS = new TaigaBiome(
        eBiomeIDs_TaigaHills, TaigaBiome::eType_Normal,
        (new BiomeProperties(L"taiga_hills"))->setTemperatureAndDownfall(0.25F, 0.8F)->depth(0.45F)->scale(0.3F));
    TAIGA_HILLS->setPreviewColor(eMinecraftColour_Preview_TaigaHills)
        ->setNameAndDescription(IDS_BIOME_TAIGA, IDS_BIOME_DESC_TAIGA)
        ->setWaterSkyColor(eMinecraftColour_Water_TaigaHills, eMinecraftColour_Sky_TaigaHills);

    EXTREME_HILLS_EDGE = new ExtremeHillsBiome(
        eBiomeIDs_ExtremeHillsEdge, ExtremeHillsBiome::eType_ExtraTrees,
        (new BiomeProperties(L"extreme_hills_edge"))->depth(0.8F)->scale(0.3F)->setTemperatureAndDownfall(0.2F, 0.3F));
    EXTREME_HILLS_EDGE->setPreviewColor(eMinecraftColour_Preview_ExtremeHillsEdge)
        ->setNameAndDescription(IDS_BIOME_EXTREME_HILLS, IDS_BIOME_DESC_EXTREME_HILLS)
        ->setWaterSkyColor(eMinecraftColour_Water_ExtremeHillsEdge, eMinecraftColour_Sky_ExtremeHillsEdge);

    JUNGLE = new JungleBiome(eBiomeIDs_Jungle, false,
                             (new BiomeProperties(L"jungle"))->setTemperatureAndDownfall(0.95F, 0.9F));
    JUNGLE->setPreviewColor(eMinecraftColour_Preview_Jungle)
        ->setNameAndDescription(IDS_BIOME_JUNGLE, IDS_BIOME_DESC_JUNGLE)
        ->setWaterSkyColor(eMinecraftColour_Water_Jungle, eMinecraftColour_Sky_Jungle);

    JUNGLE_HILLS = new JungleBiome(
        eBiomeIDs_JungleHills, false,
        (new BiomeProperties(L"jungle_hills"))->depth(0.45F)->scale(0.3F)->setTemperatureAndDownfall(0.95F, 0.9F));
    JUNGLE_HILLS->setPreviewColor(eMinecraftColour_Preview_JungleHills)
        ->setNameAndDescription(IDS_BIOME_JUNGLE, IDS_BIOME_DESC_JUNGLE)
        ->setWaterSkyColor(eMinecraftColour_Water_JungleHills, eMinecraftColour_Sky_JungleHills);

    JUNGLE_EDGE = new JungleBiome(eBiomeIDs_JungleEdge, true,
                                  (new BiomeProperties(L"jungle_edge"))->setTemperatureAndDownfall(0.95F, 0.8F));
    JUNGLE_EDGE->setPreviewColor(eMinecraftColour_Preview_JungleEdge)
        ->setNameAndDescription(IDS_BIOME_JUNGLE, IDS_BIOME_DESC_JUNGLE)
        ->setWaterSkyColor(eMinecraftColour_Water_JungleEdge, eMinecraftColour_Sky_JungleEdge);

    STONE_BEACH = new StoneBeachBiome(
        eBiomeIDs_StoneBeach,
        (new BiomeProperties(L"stone_beach"))->depth(0.1F)->scale(0.8F)->setTemperatureAndDownfall(0.2F, 0.3F));
    STONE_BEACH->setPreviewColor(eMinecraftColour_Preview_StoneBeach)
        ->setNameAndDescription(IDS_BIOME_STONE_BEACH, IDS_BIOME_DESC_STONE_BEACH)
        ->setWaterSkyColor(eMinecraftColour_Water_StoneBeach, eMinecraftColour_Sky_StoneBeach);

    COLD_BEACH = new BeachBiome(eBiomeIDs_ColdBeach, (new BiomeProperties(L"cold_beach"))
                                                         ->depth(0.0F)
                                                         ->scale(0.025F)
                                                         ->setTemperatureAndDownfall(0.05F, 0.3F)
                                                         ->snow());
    COLD_BEACH->setPreviewColor(eMinecraftColour_Preview_ColdBeach)
        ->setNameAndDescription(IDS_BIOME_COLD_BEACH, IDS_BIOME_DESC_COLD_BEACH)
        ->setWaterSkyColor(eMinecraftColour_Water_ColdBeach, eMinecraftColour_Sky_ColdBeach);

    BIRCH_FOREST = new ForestBiome(eBiomeIDs_BirchForest, ForestBiome::eType_Birch,
                                   (new BiomeProperties(L"birch_forest"))->setTemperatureAndDownfall(0.6F, 0.6F));
    BIRCH_FOREST->setPreviewColor(eMinecraftColour_Preview_BirchForest)
        ->setNameAndDescription(IDS_BIOME_BIRCH_FOREST, IDS_BIOME_DESC_BIRCH_FOREST)
        ->setWaterSkyColor(eMinecraftColour_Water_BirchForest, eMinecraftColour_Sky_BirchForest);

    BIRCH_FOREST_HILLS = new ForestBiome(
        eBiomeIDs_BirchForestHills, ForestBiome::eType_Birch,
        (new BiomeProperties(L"birch_forest_hills"))->depth(0.45F)->scale(0.3F)->setTemperatureAndDownfall(0.6F, 0.6F));
    BIRCH_FOREST_HILLS->setPreviewColor(eMinecraftColour_Preview_BirchForestHills)
        ->setNameAndDescription(IDS_BIOME_BIRCH_FOREST, IDS_BIOME_DESC_BIRCH_FOREST)
        ->setWaterSkyColor(eMinecraftColour_Water_BirchForestHills, eMinecraftColour_Sky_BirchForestHills);

    ROOFED_FOREST = new ForestBiome(eBiomeIDs_RoofedForest, ForestBiome::eType_Roofed,
                                    (new BiomeProperties(L"roofed_forest"))->setTemperatureAndDownfall(0.7F, 0.8F));
    ROOFED_FOREST->setPreviewColor(eMinecraftColour_Preview_RoofedForest)
        ->setNameAndDescription(IDS_BIOME_ROOFED_FOREST, IDS_BIOME_DESC_ROOFED_FOREST)
        ->setWaterSkyColor(eMinecraftColour_Water_RoofedForest, eMinecraftColour_Sky_RoofedForest);

    COLD_TAIGA = new TaigaBiome(
        eBiomeIDs_ColdTaiga, TaigaBiome::eType_Normal,
        (new BiomeProperties(L"cold_taiga"))->depth(0.2F)->scale(0.2F)->setTemperatureAndDownfall(-0.5F, 0.4F)->snow());
    COLD_TAIGA->setPreviewColor(eMinecraftColour_Preview_ColdTaiga)
        ->setNameAndDescription(IDS_BIOME_COLD_TAIGA, IDS_BIOME_DESC_COLD_TAIGA)
        ->setWaterSkyColor(eMinecraftColour_Water_ColdTaiga, eMinecraftColour_Sky_ColdTaiga);

    COLD_TAIGA_HILLS = new TaigaBiome(eBiomeIDs_ColdTaigaHills, TaigaBiome::eType_Normal,
                                      (new BiomeProperties(L"cold_taiga_hills"))
                                          ->depth(0.45F)
                                          ->scale(0.3F)
                                          ->setTemperatureAndDownfall(-0.5F, 0.4F)
                                          ->snow());
    COLD_TAIGA_HILLS->setPreviewColor(eMinecraftColour_Preview_ColdTaigaHills)
        ->setNameAndDescription(IDS_BIOME_COLD_TAIGA, IDS_BIOME_DESC_COLD_TAIGA)
        ->setWaterSkyColor(eMinecraftColour_Water_ColdTaigaHills, eMinecraftColour_Sky_ColdTaigaHills);

    MEGA_TAIGA = new TaigaBiome(
        eBiomeIDs_MegaTaiga, TaigaBiome::eType_MegaTaiga,
        (new BiomeProperties(L"mega_taiga"))->setTemperatureAndDownfall(0.3F, 0.8F)->depth(0.2F)->scale(0.2F));
    MEGA_TAIGA->setPreviewColor(eMinecraftColour_Preview_MegaTaiga)
        ->setNameAndDescription(IDS_BIOME_REDWOOD_TAIGA, IDS_BIOME_DESC_REDWOOD_TAIGA)
        ->setWaterSkyColor(eMinecraftColour_Water_MegaTaiga, eMinecraftColour_Sky_MegaTaiga);

    MEGA_TAIGA_HILLS = new TaigaBiome(
        eBiomeIDs_MegaTaigaHills, TaigaBiome::eType_MegaTaiga,
        (new BiomeProperties(L"mega_taiga_hills"))->depth(0.45F)->scale(0.3F)->setTemperatureAndDownfall(0.3F, 0.8F));
    MEGA_TAIGA_HILLS->setPreviewColor(eMinecraftColour_Preview_MegaTaigaHills)
        ->setNameAndDescription(IDS_BIOME_REDWOOD_TAIGA, IDS_BIOME_DESC_REDWOOD_TAIGA)
        ->setWaterSkyColor(eMinecraftColour_Water_MegaTaigaHills, eMinecraftColour_Sky_MegaTaigaHills);

    EXTREME_HILLS_PLUS_TREES = new ExtremeHillsBiome(eBiomeIDs_ExtremeHillsPlusTrees,
                                                     ExtremeHillsBiome::eType_ExtraTrees,
                                                     (new BiomeProperties(L"extreme_hills_plus_trees"))
                                                         ->depth(1.0F)
                                                         ->scale(0.5F)
                                                         ->setTemperatureAndDownfall(0.2F, 0.3F));
    EXTREME_HILLS_PLUS_TREES->setPreviewColor(eMinecraftColour_Preview_ExtremeHillsPlus)
        ->setNameAndDescription(IDS_BIOME_EXTREME_HILLS, IDS_BIOME_DESC_EXTREME_HILLS)
        ->setWaterSkyColor(eMinecraftColour_Water_ExtremeHillsPlus, eMinecraftColour_Sky_ExtremeHillsPlus);

    SAVANNA = new SavannaBiome(
        eBiomeIDs_Savanna,
        (new BiomeProperties(L"savanna"))->depth(0.125F)->scale(0.05F)->setTemperatureAndDownfall(1.2F, 0.0F)->dry());
    SAVANNA->setPreviewColor(eMinecraftColour_Preview_Savanna)
        ->setNameAndDescription(IDS_BIOME_SAVANNA, IDS_BIOME_DESC_SAVANNA)
        ->setWaterSkyColor(eMinecraftColour_Water_Savanna, eMinecraftColour_Sky_Savanna);

    SAVANNA_PLATEAU = new SavannaBiome(eBiomeIDs_SavannaPlateau, (new BiomeProperties(L"savanna_plateau"))
                                                                     ->depth(1.5F)
                                                                     ->scale(0.025F)
                                                                     ->setTemperatureAndDownfall(1.0F, 0.0F)
                                                                     ->dry());
    SAVANNA_PLATEAU->setPreviewColor(eMinecraftColour_Preview_SavannaPlateau)
        ->setNameAndDescription(IDS_BIOME_SAVANNA, IDS_BIOME_DESC_SAVANNA)
        ->setWaterSkyColor(eMinecraftColour_Water_SavannaPlateau, eMinecraftColour_Sky_SavannaPlateau);

    MESA = new MesaBiome(eBiomeIDs_Mesa, false, false,
                         (new BiomeProperties(L"mesa"))->setTemperatureAndDownfall(2.0F, 0.0F)->dry());
    MESA->setPreviewColor(eMinecraftColour_Preview_Mesa)
        ->setNameAndDescription(IDS_BIOME_MESA, IDS_BIOME_DESC_MESA)
        ->setWaterSkyColor(eMinecraftColour_Water_Mesa, eMinecraftColour_Sky_Mesa);

    MESA_ROCK = new MesaBiome(eBiomeIDs_MesaRock, false, true,
                              (new BiomeProperties(L"mesa_plateau_stone"))
                                  ->depth(1.5F)
                                  ->scale(0.025F)
                                  ->setTemperatureAndDownfall(2.0F, 0.0F)
                                  ->dry());
    MESA_ROCK->setPreviewColor(eMinecraftColour_Preview_MesaPlateauF)
        ->setNameAndDescription(IDS_BIOME_MESA, IDS_BIOME_DESC_MESA)
        ->setWaterSkyColor(eMinecraftColour_Water_MesaPlateauF, eMinecraftColour_Sky_MesaPlateauF);

    MESA_CLEAR_ROCK = new MesaBiome(eBiomeIDs_MesaClearRock, false, false,
                                    (new BiomeProperties(L"mesa_plateau"))
                                        ->depth(1.5F)
                                        ->scale(0.025F)
                                        ->setTemperatureAndDownfall(2.0F, 0.0F)
                                        ->dry());
    MESA_CLEAR_ROCK->setPreviewColor(eMinecraftColour_Preview_MesaPlateau)
        ->setNameAndDescription(IDS_BIOME_MESA, IDS_BIOME_DESC_MESA)
        ->setWaterSkyColor(eMinecraftColour_Water_MesaPlateau, eMinecraftColour_Sky_MesaPlateau);

    THE_VOID = new VoidBiome(eBiomeIDs_TheVoid, (new BiomeProperties(L"the_void"))->dry());

    SUNFLOWER_PLAINS = new PlainsBiome(eBiomeIDs_SunflowerPlains, true,
                                       (new BiomeProperties(L"sunflower_plains"))
                                           ->mutated(eBiomeIDs_Plains)
                                           ->depth(0.125F)
                                           ->scale(0.05F)
                                           ->setTemperatureAndDownfall(0.8F, 0.4F));
    SUNFLOWER_PLAINS->setPreviewColor(eMinecraftColour_Preview_Plains_Mutated)
        ->setNameAndDescription(IDS_BIOME_PLAINS, IDS_BIOME_DESC_PLAINS)
        ->setWaterSkyColor(eMinecraftColour_Water_Plains, eMinecraftColour_Sky_Plains);

    DESERT_MUTATED = new DesertBiome(eBiomeIDs_DesertMutated, (new BiomeProperties(L"desert_mutated"))
                                                                  ->mutated(eBiomeIDs_Desert)
                                                                  ->depth(0.225F)
                                                                  ->scale(0.25F)
                                                                  ->setTemperatureAndDownfall(2.0F, 0.0F)
                                                                  ->dry());
    DESERT_MUTATED->setPreviewColor(eMinecraftColour_Preview_Desert_Mutated)
        ->setNameAndDescription(IDS_BIOME_DESERT, IDS_BIOME_DESC_DESERT)
        ->setWaterSkyColor(eMinecraftColour_Water_Desert, eMinecraftColour_Sky_Desert);

    EXTREME_HILLS_MUTATED = new ExtremeHillsBiome(eBiomeIDs_ExtremeHillsMutated, ExtremeHillsBiome::eType_Mutated,
                                                  (new BiomeProperties(L"extreme_hills_mutated"))
                                                      ->mutated(eBiomeIDs_ExtremeHills)
                                                      ->depth(1.0F)
                                                      ->scale(0.5F)
                                                      ->setTemperatureAndDownfall(0.2F, 0.3F));
    EXTREME_HILLS_MUTATED->setPreviewColor(eMinecraftColour_Preview_ExtremeHills_Mutated)
        ->setNameAndDescription(IDS_BIOME_EXTREME_HILLS, IDS_BIOME_DESC_EXTREME_HILLS)
        ->setWaterSkyColor(eMinecraftColour_Water_ExtremeHills, eMinecraftColour_Sky_ExtremeHills);

    FLOWER_FOREST = new ForestBiome(eBiomeIDs_FlowerForest, ForestBiome::eType_Flower,
                                    (new BiomeProperties(L"flower_forest"))
                                        ->mutated(eBiomeIDs_Forest)
                                        ->scale(0.4F)
                                        ->setTemperatureAndDownfall(0.7F, 0.8F));
    FLOWER_FOREST->setPreviewColor(eMinecraftColour_Preview_Forest_Mutated)
        ->setNameAndDescription(IDS_BIOME_FOREST, IDS_BIOME_DESC_FOREST)
        ->setWaterSkyColor(eMinecraftColour_Water_Forest, eMinecraftColour_Sky_Forest);

    TAIGA_MUTATED = new TaigaBiome(eBiomeIDs_TaigaMutated, TaigaBiome::eType_Normal,
                                   (new BiomeProperties(L"taiga_mutated"))
                                       ->mutated(eBiomeIDs_Taiga)
                                       ->depth(0.3F)
                                       ->scale(0.4F)
                                       ->setTemperatureAndDownfall(0.25F, 0.8F));
    TAIGA_MUTATED->setPreviewColor(eMinecraftColour_Preview_Taiga_Mutated)
        ->setNameAndDescription(IDS_BIOME_TAIGA, IDS_BIOME_DESC_TAIGA)
        ->setWaterSkyColor(eMinecraftColour_Water_Taiga, eMinecraftColour_Sky_Taiga);

    SWAMPLAND_MUTATED = new SwampBiome(eBiomeIDs_SwamplandMutated, (new BiomeProperties(L"swampland_mutated"))
                                                                       ->mutated(eBiomeIDs_Swampland)
                                                                       ->depth(-0.1F)
                                                                       ->scale(0.3F)
                                                                       ->setTemperatureAndDownfall(0.8F, 0.9F)
                                                                       ->waterColor(0xE0FFAE));
    SWAMPLAND_MUTATED->setPreviewColor(eMinecraftColour_Preview_Swampland_Mutated)
        ->setNameAndDescription(IDS_BIOME_SWAMPLAND, IDS_BIOME_DESC_SWAMPLAND)
        ->setWaterSkyColor(eMinecraftColour_Water_Swampland, eMinecraftColour_Sky_Swampland);

    ICE_PLAINS_SPIKES = new IceBiome(eBiomeIDs_IcePlainsSpikes, true,
                                     (new BiomeProperties(L"ice_plains_spikes"))
                                         ->mutated(eBiomeIDs_IcePlains)
                                         ->depth(0.425F)
                                         ->scale(0.45000002F)
                                         ->setTemperatureAndDownfall(0.0F, 0.5F)
                                         ->snow());
    ICE_PLAINS_SPIKES->setPreviewColor(eMinecraftColour_Preview_IcePlains_Mutated)
        ->setNameAndDescription(IDS_BIOME_ICE_FLATS, IDS_BIOME_DESC_ICE_FLATS)
        ->setWaterSkyColor(eMinecraftColour_Water_IcePlains, eMinecraftColour_Sky_IcePlains);

    JUNGLE_MUTATED = new JungleBiome(eBiomeIDs_JungleMutated, false,
                                     (new BiomeProperties(L"jungle_mutated"))
                                         ->mutated(eBiomeIDs_Jungle)
                                         ->depth(0.2F)
                                         ->scale(0.4F)
                                         ->setTemperatureAndDownfall(0.95F, 0.9F));
    JUNGLE_MUTATED->setPreviewColor(eMinecraftColour_Preview_Jungle_Mutated)
        ->setNameAndDescription(IDS_BIOME_JUNGLE, IDS_BIOME_DESC_JUNGLE)
        ->setWaterSkyColor(eMinecraftColour_Water_Jungle, eMinecraftColour_Sky_Jungle);

    JUNGLE_EDGE_MUTATED = new JungleBiome(eBiomeIDs_JungleEdgeMutated, true,
                                          (new BiomeProperties(L"jungle_edge_mutated"))
                                              ->mutated(eBiomeIDs_JungleEdge)
                                              ->depth(0.2F)
                                              ->scale(0.4F)
                                              ->setTemperatureAndDownfall(0.95F, 0.8F));
    JUNGLE_EDGE_MUTATED->setPreviewColor(eMinecraftColour_Preview_JungleEdge_Mutated)
        ->setNameAndDescription(IDS_BIOME_JUNGLE, IDS_BIOME_DESC_JUNGLE)
        ->setWaterSkyColor(eMinecraftColour_Water_JungleEdge, eMinecraftColour_Sky_JungleEdge);

    BIRCH_FOREST_MUTATED = new MutatedForestBiome(eBiomeIDs_BirchForestMutated,
                                                  (new BiomeProperties(L"birch_forest_mutated"))
                                                      ->mutated(eBiomeIDs_BirchForest)
                                                      ->depth(0.2F)
                                                      ->scale(0.4F)
                                                      ->setTemperatureAndDownfall(0.6F, 0.6F));
    BIRCH_FOREST_MUTATED->setPreviewColor(eMinecraftColour_Preview_BirchForest_Mutated)
        ->setNameAndDescription(IDS_BIOME_BIRCH_FOREST, IDS_BIOME_DESC_BIRCH_FOREST)
        ->setWaterSkyColor(eMinecraftColour_Water_BirchForest, eMinecraftColour_Sky_BirchForest);

    BIRCH_FOREST_HILLS_MUTATED = new MutatedForestBiome(eBiomeIDs_BirchForestHillsMutated,
                                                        (new BiomeProperties(L"birch_forest_hills_mutated"))
                                                            ->mutated(eBiomeIDs_BirchForestHills)
                                                            ->depth(0.55F)
                                                            ->scale(0.5F)
                                                            ->setTemperatureAndDownfall(0.6F, 0.6F));
    BIRCH_FOREST_HILLS_MUTATED->setPreviewColor(eMinecraftColour_Preview_BirchForestHills_Mutated)
        ->setNameAndDescription(IDS_BIOME_BIRCH_FOREST, IDS_BIOME_DESC_BIRCH_FOREST)
        ->setWaterSkyColor(eMinecraftColour_Water_BirchForestHills, eMinecraftColour_Sky_BirchForestHills);

    ROOFED_FOREST_MUTATED = new ForestBiome(eBiomeIDs_RoofedForestMutated, ForestBiome::eType_Roofed,
                                            (new BiomeProperties(L"roofed_forest_mutated"))
                                                ->mutated(eBiomeIDs_RoofedForest)
                                                ->depth(0.2F)
                                                ->scale(0.4F)
                                                ->setTemperatureAndDownfall(0.7F, 0.8F));
    ROOFED_FOREST_MUTATED->setPreviewColor(eMinecraftColour_Preview_RoofedForest_Mutated)
        ->setNameAndDescription(IDS_BIOME_ROOFED_FOREST, IDS_BIOME_DESC_ROOFED_FOREST)
        ->setWaterSkyColor(eMinecraftColour_Water_RoofedForest, eMinecraftColour_Sky_RoofedForest);

    COLD_TAIGA_MUTATED = new TaigaBiome(eBiomeIDs_ColdTaigaMutated, TaigaBiome::eType_Normal,
                                        (new BiomeProperties(L"cold_taiga_mutated"))
                                            ->mutated(eBiomeIDs_ColdTaiga)
                                            ->depth(0.3F)
                                            ->scale(0.4F)
                                            ->setTemperatureAndDownfall(-0.5F, 0.4F)
                                            ->snow());
    COLD_TAIGA_MUTATED->setPreviewColor(eMinecraftColour_Preview_ColdTaiga_Mutated)
        ->setNameAndDescription(IDS_BIOME_COLD_TAIGA, IDS_BIOME_DESC_COLD_TAIGA)
        ->setWaterSkyColor(eMinecraftColour_Water_ColdTaiga, eMinecraftColour_Sky_ColdTaiga);

    MEGA_SPRUCE_TAIGA = new TaigaBiome(eBiomeIDs_MegaSpruceTaiga, TaigaBiome::eType_Mutated,
                                       (new BiomeProperties(L"mega_spruce_taiga"))
                                           ->mutated(eBiomeIDs_MegaTaiga)
                                           ->depth(0.2F)
                                           ->scale(0.2F)
                                           ->setTemperatureAndDownfall(0.25F, 0.8F));
    MEGA_SPRUCE_TAIGA->setPreviewColor(eMinecraftColour_Preview_MegaTaiga_Mutated)
        ->setNameAndDescription(IDS_BIOME_REDWOOD_TAIGA, IDS_BIOME_DESC_REDWOOD_TAIGA)
        ->setWaterSkyColor(eMinecraftColour_Water_MegaTaiga, eMinecraftColour_Sky_MegaTaiga);

    REDWOOD_TAIGA_HILLS_MUTATED = new TaigaBiome(eBiomeIDs_RedwoodTaigaHillsMutated, TaigaBiome::eType_Mutated,
                                                 (new BiomeProperties(L"redwood_taiga_hills_mutated"))
                                                     ->mutated(eBiomeIDs_MegaTaigaHills)
                                                     ->depth(0.2F)
                                                     ->scale(0.2F)
                                                     ->setTemperatureAndDownfall(0.25F, 0.8F));
    REDWOOD_TAIGA_HILLS_MUTATED->setPreviewColor(eMinecraftColour_Preview_MegaTaigaHills_Mutated)
        ->setNameAndDescription(IDS_BIOME_REDWOOD_TAIGA, IDS_BIOME_DESC_REDWOOD_TAIGA)
        ->setWaterSkyColor(eMinecraftColour_Water_MegaTaigaHills, eMinecraftColour_Sky_MegaTaigaHills);

    EXTREME_HILLS_PLUS_TREES_MUTATED = new ExtremeHillsBiome(eBiomeIDs_ExtremeHillsPlusTreesMutated,
                                                             ExtremeHillsBiome::eType_Mutated,
                                                             (new BiomeProperties(L"extreme_hills_plus_trees_mutated"))
                                                                 ->mutated(eBiomeIDs_ExtremeHillsPlusTrees)
                                                                 ->depth(1.0F)
                                                                 ->scale(0.5F)
                                                                 ->setTemperatureAndDownfall(0.2F, 0.3F));
    EXTREME_HILLS_PLUS_TREES_MUTATED->setPreviewColor(eMinecraftColour_Preview_ExtremeHillsPlus_Mutated)
        ->setNameAndDescription(IDS_BIOME_EXTREME_HILLS, IDS_BIOME_DESC_EXTREME_HILLS)
        ->setWaterSkyColor(eMinecraftColour_Water_ExtremeHillsPlus, eMinecraftColour_Sky_ExtremeHillsPlus);

    SAVANNA_MUTATED = new MutatedSavannaBiome(eBiomeIDs_SavannaMutated, (new BiomeProperties(L"savanna_mutated"))
                                                                            ->mutated(eBiomeIDs_Savanna)
                                                                            ->depth(0.3625F)
                                                                            ->scale(1.225F)
                                                                            ->setTemperatureAndDownfall(1.1F, 0.0F)
                                                                            ->dry());
    SAVANNA_MUTATED->setPreviewColor(eMinecraftColour_Preview_Savanna_Mutated)
        ->setNameAndDescription(IDS_BIOME_SAVANNA, IDS_BIOME_DESC_SAVANNA)
        ->setWaterSkyColor(eMinecraftColour_Water_Savanna, eMinecraftColour_Sky_Savanna);

    SAVANNA_PLATEAU_MUTATED = new MutatedSavannaBiome(eBiomeIDs_SavannaPlateauMutated,
                                                      (new BiomeProperties(L"savanna_plateau_mutated"))
                                                          ->mutated(eBiomeIDs_SavannaPlateau)
                                                          ->depth(1.05F)
                                                          ->scale(1.2125001F)
                                                          ->setTemperatureAndDownfall(1.0F, 0.0F)
                                                          ->dry());
    SAVANNA_PLATEAU_MUTATED->setPreviewColor(eMinecraftColour_Preview_SavannaPlateau_Mutated)
        ->setNameAndDescription(IDS_BIOME_SAVANNA, IDS_BIOME_DESC_SAVANNA)
        ->setWaterSkyColor(eMinecraftColour_Water_SavannaPlateau, eMinecraftColour_Sky_SavannaPlateau);

    MESA_BRYCE = new MesaBiome(
        eBiomeIDs_MesaBryce, true, false,
        (new BiomeProperties(L"mesa_bryce"))->mutated(eBiomeIDs_Mesa)->setTemperatureAndDownfall(2.0F, 0.0F)->dry());
    MESA_BRYCE->setPreviewColor(eMinecraftColour_Preview_Mesa_Mutated)
        ->setNameAndDescription(IDS_BIOME_MESA, IDS_BIOME_DESC_MESA)
        ->setWaterSkyColor(eMinecraftColour_Water_Mesa, eMinecraftColour_Sky_Mesa);

    MESA_ROCK_MUTATED = new MesaBiome(eBiomeIDs_MesaRockMutated, false, true,
                                      (new BiomeProperties(L"mesa_plateau_stone_mutated"))
                                          ->mutated(eBiomeIDs_MesaRock)
                                          ->depth(0.45F)
                                          ->scale(0.3F)
                                          ->setTemperatureAndDownfall(2.0F, 0.0F)
                                          ->dry());
    MESA_ROCK_MUTATED->setPreviewColor(eMinecraftColour_Preview_MesaPlateauF_Mutated)
        ->setNameAndDescription(IDS_BIOME_MESA, IDS_BIOME_DESC_MESA)
        ->setWaterSkyColor(eMinecraftColour_Water_MesaPlateauF, eMinecraftColour_Sky_MesaPlateauF);

    MESA_CLEAR_ROCK_MUTATED = new MesaBiome(eBiomeIDs_MesaClearRockMutated, false, false,
                                            (new BiomeProperties(L"mesa_plateau_mutated"))
                                                ->mutated(eBiomeIDs_MesaClearRock)
                                                ->depth(0.45F)
                                                ->scale(0.3F)
                                                ->setTemperatureAndDownfall(2.0F, 0.0F)
                                                ->dry());
    MESA_CLEAR_ROCK_MUTATED->setPreviewColor(eMinecraftColour_Preview_MesaPlateau_Mutated)
        ->setNameAndDescription(IDS_BIOME_MESA, IDS_BIOME_DESC_MESA)
        ->setWaterSkyColor(eMinecraftColour_Water_MesaPlateau, eMinecraftColour_Sky_MesaPlateau);

    DEFAULT = OCEAN;
}

int Biome::getId(Biome* biome)
{
    return biome->id;
}

Biome* Biome::byId(int biomeId)
{
    return biomes[biomeId];
}

Biome* Biome::byNameId(std::wstring name)
{
    return sBiomesByName[name];
}

Biome** Biome::getBiomes()
{
    return biomes;
}

Biome* Biome::getBiome(int biomeId, Biome* fallback)
{
    if (biomeId < 0 || biomeId > MAX_BIOME_ID)
    {
        return OCEAN;
    }

    if (biomes[biomeId] != NULL)
    {
        return biomes[biomeId];
    }

    return fallback;
}

Biome* Biome::getBiome(int biomeId)
{
    return getBiome(biomeId, NULL);
}

void Biome::generateColoursDebugOutput()
{
}

BiomeDecorator* Biome::createDecorator()
{
    return new BiomeDecorator();
}

Biome* Biome::setWaterSkyColor(eMinecraftColour waterColour, eMinecraftColour skyColour)
{
    mWaterColour = waterColour;
    mSkyColour = skyColour;

    return this;
}

Biome* Biome::setNameAndDescription(int nameId, int descriptionId)
{
    mNameId = nameId;
    mDescriptionId = descriptionId;

    return this;
}

Biome* Biome::setPreviewColor(eMinecraftColour previewColour)
{
    mPreviewColour = previewColour;

    return this;
}

Biome* Biome::setRunDepthAndFloorBlock(unsigned int runDepth, unsigned int floorDepth, BlockPair floorBlock)
{
    mRunDepth = runDepth;
    mFloorDepth = floorDepth;
    mFloorBlock = floorBlock;

    return this;
}

Biome* Biome::setSnowAccumulation(float minAccumulation, float maxAccumulation)
{
    mSnowAccumulation = minAccumulation;
    mMinSnowAccumulation = minAccumulation;
    mMaxSnowAccumulation = maxAccumulation;

    if (minAccumulation > 0.0F && getBiomeType() != eBiomeType_Ocean)
    {
        mCreatures.push_back(new MobSpawnerData(4, EntityType::RABBIT, 2, 3));
    }

    return this;
}

Biome::BiomeType Biome::getBiomeType() const
{
    return mBiomeType;
}

eMinecraftColour Biome::getPreviewColor()
{
    return mPreviewColour;
}

void Biome::clearDefaultFriendlies()
{
    mCreatures.clear();
    mChickens.clear();
}

int Biome::getMinSnowLayers() const
{
    return (int)(mMinSnowAccumulation * 8.0F);
}

int Biome::getMaxSnowLayers() const
{
    return (int)(mMaxSnowAccumulation * 8.0F);
}

std::vector<Biome::MobSpawnerData*>* Biome::getMobs(MobCategory* category)
{
    if (category == MobCategory::MONSTER)
    {
        return &mMonsters;
    }

    if (category == MobCategory::CREATURE)
    {
        return &mCreatures;
    }

    if (category == MobCategory::WATER_CREATURE)
    {
        return &mWaterCreatures;
    }

    if (category == MobCategory::CHICKEN)
    {
        return &mChickens;
    }

    if (category == MobCategory::SQUID)
    {
        return &mSquid;
    }

    if (category == MobCategory::WOLF)
    {
        return &mWolves;
    }

    if (category == MobCategory::MUSHROOM_COW)
    {
        return &mMushroomCows;
    }

    if (category == MobCategory::SLIME)
    {
        return &mSlimes;
    }

    if (category == MobCategory::AMBIENT)
    {
        return &mAmbientCreatures;
    }

    return NULL;
}

bool Biome::isSnowCovered()
{
    return mSnowEnabled || mSnowAccumulation > 0.0F;
}

float Biome::getDownfall()
{
    return mDownfall;
}

float Biome::getTemperature() const
{
    return mTemperature;
}

float Biome::getDepth()
{
    return mDepth;
}

float Biome::getScale()
{
    return mScale;
}

float Biome::getDefaultBiomeTemperature() const
{
    return mTemperature;
}

std::wstring Biome::getName(bool useShortName)
{
    if (mNameId == -1)
    {
        return L"";
    }

    return L10N::GetString(mNameId);
}

std::wstring Biome::getDescription()
{
    std::wstring description = L"";

    if (mDescriptionId != -1)
    {
        description = L10N::GetString(mDescriptionId);
    }

    return description;
}

OceanRuinConfiguration Biome::getOceanRuinConfig() const
{
    return mOceanRuinConfig;
}

AbstractTreeFeature* Biome::getTreeFeature(Random& random)
{
    if (random.nextInt(10) == 0)
    {
        return new BigTreeFeature(false);
    }

    return new TreeFeature(false);
}

Feature* Biome::getGrassFeature(Random& random)
{
    return new TallgrassFeature(TallGrassBlock::Variant::GRASS);
}

FlowerBlock::Flower* Biome::getRandomFlower(Random& random, const BlockPos& pos)
{
    if (random.nextInt(3) > 0)
    {
        return FlowerBlock::Flower::DANDELION;
    }

    return FlowerBlock::Flower::POPPY;
}

unsigned int Biome::getSkyColor(float temperature)
{
    if (Minecraft::GetInstance()->isUsingDefaultSkin())
    {
        float scaled = Mth::clamp(temperature / 3.0F, -1.0F, 1.0F);

        return Mth::hsvToRgb(0.62222224F - scaled * 0.05F, 0.5F + scaled * 0.1F, 1.0F);
    }

    return Minecraft::GetInstance()->getColourTable()->getColour(mSkyColour);
}

bool Biome::hasSnow()
{
    return isSnowCovered();
}

bool Biome::hasRain()
{
    if (isSnowCovered())
    {
        return false;
    }

    return mRainEnabled;
}

bool Biome::isHumid()
{
    return getDownfall() > 0.85F;
}

float Biome::getCreatureProbability()
{
    return 0.1F;
}

float Biome::getTemperature(const BlockPos& pos)
{
    if (pos.y > 64)
    {
        float noise = (float)(TEMPERATURE_NOISE->getValue(pos.x * 0.125F, pos.z * 0.125F) * 4.0);

        return getTemperature() - (noise + pos.y - 64.0F) * 0.05F / 30.0F;
    }

    return getTemperature();
}

void Biome::decorate(Level* level, Random& random, const BlockPos& pos)
{
    mDecorator->decorate(level, &random, this, pos);
}

unsigned int Biome::getGrassColor(const BlockPos& pos)
{
    double temperature = Mth::clamp(getTemperature(pos), 0.0F, 1.0F);
    double downfall = Mth::clamp(getDownfall(), 0.0F, 1.0F);

    return GrassColor::get(temperature, downfall);
}

unsigned int Biome::getFoliageColor(const BlockPos& pos)
{
    double temperature = Mth::clamp(getTemperature(pos), 0.0F, 1.0F);
    double downfall = Mth::clamp(getDownfall(), 0.0F, 1.0F);

    return FoliageColor::get(temperature, downfall);
}

void Biome::setGrassColor(const BlockPos& pos, unsigned int colour)
{
    double temperature = Mth::clamp(getTemperature(pos), 0.0F, 1.0F);
    double downfall = Mth::clamp(getDownfall(), 0.0F, 1.0F);

    GrassColor::set(temperature, downfall, colour);
}

void Biome::setFoliageColor(const BlockPos& pos, unsigned int colour)
{
    double temperature = Mth::clamp(getTemperature(pos), 0.0F, 1.0F);
    double downfall = Mth::clamp(getDownfall(), 0.0F, 1.0F);

    FoliageColor::set(temperature, downfall, colour);
}

void Biome::buildSurfaceAt(Level* level, Random& random, ChunkPrimer* primer, int x, int z, double noise)
{
    buildSurfaceAtDefault(level, random, primer, x, z, noise);
}

void Biome::buildSurfaceAtDefault(Level* level, Random& random, ChunkPrimer* primer, int x, int z, double noise)
{
    PIXBeginNamedEvent(0.0F, "Biome::buildSurfaceAtDefault");

    BlockPair filler = mFillerBlock;
    BlockPair top = mTopBlock;

    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();

    if (options != NULL)
    {
        options->getBiomeOverride(id, &filler, &top);
    }

    int runDepth = -1;
    int depth = (int)(noise / 3.0 + 3.0 + random.nextDouble() * 0.25);

    MutableBlockPos pos(0, 0, 0);

    for (int y = 127; y >= 0; --y)
    {
        int index = (x & 15) << 7 | (z & 15) << 11 | y;

        if (y <= random.nextInt(2) + 1)
        {
            primer->setBlockAndData(index, BEDROCK_BLOCK.blockId, BEDROCK_BLOCK.blockData);
            continue;
        }

        int blockId = primer->getBlockId(index);

        if (blockId == BlockId::AIR)
        {
            runDepth = -1;
            continue;
        }

        if (blockId != BlockId::STONE)
        {
            continue;
        }

        if (runDepth == -1)
        {
            if (depth <= 0)
            {
                filler = STONE_BLOCK;
                top = AIR_BLOCK;
            }
            else if (y >= 59 && y <= 64)
            {
                filler = mFillerBlock;
                top = mTopBlock;

                if (options != NULL)
                {
                    options->getBiomeOverride(id, &filler, &top);
                }
            }

            if (y < 63 && top.blockId == BlockId::AIR)
            {
                pos.x = x;
                pos.y = y;
                pos.z = z;

                if (getTemperature(pos) < 0.15F)
                {
                    top = ICE_BLOCK;
                }
                else
                {
                    top = WATER_BLOCK;
                }
            }

            runDepth = depth;

            if (y >= 62)
            {
                primer->setBlockAndData(index, top.blockId, top.blockData);
            }
            else if (y < (63 - (int)mRunDepth) - (int)mFloorDepth)
            {
                filler = STONE_BLOCK;
                top = AIR_BLOCK;

                primer->setBlockAndData(index, mFloorBlock.blockId, mFloorBlock.blockData);
            }
            else
            {
                primer->setBlockAndData(index, filler.blockId, filler.blockData);
            }
        }
        else if (runDepth > 0)
        {
            --runDepth;

            primer->setBlockAndData(index, filler.blockId, filler.blockData);

            if (runDepth == 0 && filler.blockId == BlockId::SAND && depth > 1)
            {
                int extra = y - 63;

                if (extra < 0)
                {
                    extra = 0;
                }

                runDepth = random.nextInt(4) + extra;

                BlockPair sandstone = SANDSTONE_BLOCK;

                if (filler.blockData == SandBlock::Variant::RED_SAND->getData())
                {
                    sandstone = RED_SANDSTONE_BLOCK;
                }

                filler = sandstone;
            }
        }
    }

    PIXEndNamedEvent();
}

Biome::TemperatureCategory Biome::getTemperatureCategory() const
{
    float temperature = getTemperature();

    if (temperature < 0.2)
    {
        return eTemperatureCategory_Cold;
    }

    if (temperature < 1.0)
    {
        return eTemperatureCategory_Medium;
    }

    return eTemperatureCategory_Warm;
}

bool Biome::isAlwaysValidSpawn()
{
    return false;
}

unsigned int Biome::getWaterColor()
{
    return Minecraft::GetInstance()->getColourTable()->getWaterSurfaceColor(id, mBaseBiomeId);
}

unsigned int Biome::getUnderWaterFogColor()
{
    return Minecraft::GetInstance()->getColourTable()->getWaterFogColor(id, mBaseBiomeId);
}

float Biome::getFogDistance()
{
    return (float)Minecraft::GetInstance()->getColourTable()->getWaterFogDistance(id, mBaseBiomeId);
}

Vec3* Biome::getFogColor(float celestialAngle, float partialTicks) const
{
    float brightness = Mth::cos(celestialAngle * 3.1415927F * 2.0F) * 2.0F + 0.5F;
    if (brightness < 0.0F)
        brightness = 0.0F;
    else if (brightness > 1.0F)
        brightness = 1.0F;

    unsigned int fogColour = Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Default_Fog_Colour);

    float red = (float)(fogColour >> 16 & 0xFF) / 255.0F;
    float green = (float)(fogColour >> 8 & 0xFF) / 255.0F;
    float blue = (float)(fogColour & 0xFF) / 255.0F;

    red = red * (brightness * 0.94F + 0.06F);
    green = green * (brightness * 0.94F + 0.06F);
    blue = blue * (brightness * 0.91F + 0.09F);

    return Vec3::newTemp(red, green, blue);
}

bool Biome::isFoggy()
{
    return false;
}

bool Biome::isNatural()
{
    return true;
}
