#include "game/world/biome/OceanFrozenBiome.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <cmath>

#include "game/core/PerlinSimplexNoise.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/BiomeDecorator.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/generation/BlueIceFeature.hpp"
#include "game/world/generation/IcebergFeature.hpp"

static const int PACKED_ICE_BLOCK_ID = 174;
static const int SNOW_BLOCK_ID = 80;

OceanFrozenBiome::OceanFrozenBiome(EBiomeIDs id, bool deep, BiomeProperties* properties)
    : Biome(id, eBiomeType_Ocean, properties),
      mDeep(deep),
      mIcebergNoise(new PerlinSimplexNoise(3456LL, 3))
{
    mSeed = 0;
    mRandom = NULL;
    mIcebergsNoise = NULL;
    mIcebergsNoise2 = NULL;
    clearDefaultFriendlies();

    mWaterCreatures.push_back(new MobSpawnerData(26, EntityType::SALMON, 3, 5));
    mWaterCreatures.push_back(new MobSpawnerData(75, EntityType::FISH, 4, 7));
    mSquid.push_back(new MobSpawnerData(10, EntityType::SQUID, 2, 4));
    mMonsters.push_back(new MobSpawnerData(10, EntityType::DROWNED, 2, 4));
    mMonsters.push_back(new MobSpawnerData(120, EntityType::STRAY, 1, 2));
    mCreatures.push_back(new MobSpawnerData(5, EntityType::POLAR_BEAR, 1, 2));

    clearDefaultFriendlies();

    mDecorator->kelpPerChunk = 20;
    mDecorator->seagrassPerChunk = 12;

    mOceanRuinConfig.biomeType = OceanRuin::eRuinType_Cold;
    mOceanRuinConfig.enabled = true;

    if (mDeep)
    {
        mOceanRuinConfig.smallProbability = 0.5F;
        mOceanRuinConfig.clusterProbability = 0.95F;
    }
    else
    {
        mOceanRuinConfig.smallProbability = 0.3F;
        mOceanRuinConfig.clusterProbability = 0.9F;
    }
}

float OceanFrozenBiome::getCreatureProbability()
{
    return 0.1F;
}

void OceanFrozenBiome::decorate(Level* level, Random& random, const BlockPos& pos)
{
    Biome::decorate(level, random, pos);

    if (random.nextInt(8) == 0)
    {
        const int x = random.nextInt(8) + 12;
        const int z = random.nextInt(8) + 12;
        mDecorator->icebergGen->place(level, random, level->getHeightmapPos(pos.offset(x, 0, z)));
    }

    int blueIceCount = random.nextInt(20);
    for (int i = 0; i < blueIceCount; ++i)
    {
        const int x = random.nextInt(16) + 8;
        const int y = random.nextInt(32) + 30;
        const int z = random.nextInt(16) + 8;
        mDecorator->blueIceGen->place(level, random, pos.offset(x, y, z));
    }
}

void OceanFrozenBiome::buildSurfaceAt(Level* level, Random& random, ChunkPrimer* primer, int x, int z, double noise)
{
    if (mIcebergsNoise == NULL || mIcebergsNoise2 == NULL || mSeed != level->getSeed())
    {
        delete mIcebergsNoise;
        delete mIcebergsNoise2;
        delete mRandom;

        mRandom = new Random(mSeed);
        mIcebergsNoise = new PerlinSimplexNoise(mRandom, 4);
        mIcebergsNoise2 = new PerlinSimplexNoise(mRandom, 1);
    }

    mSeed = level->getSeed();

    float icebergTop = 0.0F;
    float icebergBottom = 0.0F;

    int seaLevel = level->getSeaLevel();
    float temperature = getTemperature(BlockPos(x, seaLevel, z));

    float height = std::min(fabs(noise), mIcebergsNoise->getValue((double)x * 0.1, (double)z * 0.1));

    if (height > 1.8F)
    {
        double width = mIcebergsNoise2->getValue((double)x * 0.9765625, (double)z * 0.9765625);
        float thickness = height * height * 0.8F;
        float widthLimit = ceilf(fabsf((float)width) * 40.0F) + 14.0F;

        if (thickness > widthLimit)
        {
            thickness = widthLimit;
        }

        if (temperature > 0.1F)
        {
            thickness -= 2.0F;
        }

        float surface = (float)(seaLevel + 1);
        icebergTop = thickness + surface;
        icebergBottom = (surface - thickness) - 7.0F;
    }

    int snowCount = 0;
    int snowLimit = random.nextInt(4) + 2;
    int snowStart = seaLevel + 18 + random.nextInt(10);
    int packedIceData = Blocks::packed_ice->convertBlockStateToLegacyData(Blocks::packed_ice->defaultBlockState());
    int snowData = Blocks::snow->convertBlockStateToLegacyData(Blocks::snow->defaultBlockState());
    int localX = x & 15;
    int localZ = z & 15;
    int column = localX << 11 | localZ << 7;

    for (int y = 127; y >= 0; --y)
    {
        int index = column | y;

        if (y <= random.nextInt(2) + 1)
        {
            primer->setBlockAndData(index, BEDROCK_BLOCK.blockId, BEDROCK_BLOCK.blockData);
        }
        else
        {
            const BlockState* state = primer->getState(localX, y, localZ);

            if ((state->getMaterial() == Material::AIR && y < (int)icebergTop && random.nextFloat() > 0.01F)
                || (state->getMaterial() == Material::WATER && y > (int)icebergBottom && y < seaLevel
                    && icebergBottom != 0.0F && random.nextFloat() > 0.15F))
            {
                primer->setBlockAndData(index, PACKED_ICE_BLOCK_ID, packedIceData);
                state = Blocks::packed_ice->defaultBlockState();
            }

            if (state->getBlock() == Blocks::packed_ice && snowCount <= snowLimit && y > snowStart)
            {
                primer->setBlockAndData(index, SNOW_BLOCK_ID, snowData);
                ++snowCount;
            }
        }
    }

    buildSurfaceAtDefault(level, random, primer, x, z, noise);
}

float OceanFrozenBiome::getTemperature(const BlockPos& pos)
{
    float temperature = getDefaultBiomeTemperature();

    float icebergNoise = mIcebergNoise->getValue((double)pos.x * 0.05, (double)pos.z * 0.05);
    float regionNoise = BIOME_INFO_NOISE->getValue((double)pos.x * 0.2, (double)pos.z * 0.2);

    if (icebergNoise + regionNoise < 0.3F)
    {
        float localNoise = BIOME_INFO_NOISE->getValue((double)pos.x * 0.09, (double)pos.z * 0.09);

        if (localNoise < 0.8F)
        {
            temperature = 0.2F;
        }
    }

    if (pos.y > 64)
    {
        float heightNoise = (float)(TEMPERATURE_NOISE->getValue(pos.x * 0.125F, pos.z * 0.125F) * 4.0);

        return temperature - (heightNoise + pos.y - 64.0F) * 0.05F / 30.0F;
    }

    return temperature;
}

Biome::EBiomeBaseClass OceanFrozenBiome::getBaseClass()
{
    return eBiomeBaseClass_OceanFrozen;
}

Biome::TemperatureCategory OceanFrozenBiome::getTemperatureCategory() const
{
    return eTemperatureCategory_Ocean;
}
