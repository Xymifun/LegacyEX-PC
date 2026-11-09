#include "game/world/biome/BeachBiome.hpp"
#include "game/core/EntityType.hpp"

#include "game/world/biome/BiomeDecorator.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/world/block/Blocks.hpp"

static const float TURTLE_MIN_TEMPERATURE = 0.5F;

BeachBiome::BeachBiome(EBiomeIDs biomeId, BiomeProperties* properties) : Biome(biomeId, eBiomeType_Beach, properties)
{
    mCreatures.clear();
    mChickens.clear();

    mTopBlock = BlockPair(BlockId::SAND,
                          Blocks::sand->convertBlockStateToLegacyData(Blocks::sand->defaultBlockState()));
    mFillerBlock = BlockPair(BlockId::SAND,
                             Blocks::sand->convertBlockStateToLegacyData(Blocks::sand->defaultBlockState()));

    if (getTemperature() > TURTLE_MIN_TEMPERATURE)
    {
        mCreatures.push_back(new MobSpawnerData(8, EntityType::TURTLE, 2, 6));
    }

    mDecorator->treesPerChunk = -999;
    mDecorator->deadBushPerChunk = 0;
    mDecorator->reedsPerChunk = 0;
    mDecorator->cactiPerChunk = 0;
}

Biome::EBiomeBaseClass BeachBiome::getBaseClass()
{
    return eBiomeBaseClass_Beach;
}
