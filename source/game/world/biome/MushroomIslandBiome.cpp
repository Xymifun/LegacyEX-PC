#include "game/world/biome/MushroomIslandBiome.hpp"
#include "game/core/EntityType.hpp"

#include "game/world/biome/BiomeDecorator.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/world/block/Blocks.hpp"

static const int MUSHROOM_FEATURE_DISABLED = -100;

MushroomIslandBiome::MushroomIslandBiome(EBiomeIDs biomeId, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_MushroomIsland, properties)
{
    mDecorator->treesPerChunk = MUSHROOM_FEATURE_DISABLED;
    mDecorator->flowersPerChunk = MUSHROOM_FEATURE_DISABLED;
    mDecorator->grassPerChunk = MUSHROOM_FEATURE_DISABLED;
    mDecorator->mushroomsPerChunk = 1;
    mDecorator->bigMushroomsPerChunk = 1;

    mTopBlock = BlockPair(BlockId::MYCELIUM,
                          Blocks::mycelium->convertBlockStateToLegacyData(Blocks::mycelium->defaultBlockState()));

    mMonsters.clear();
    mCreatures.clear();
    mChickens.clear();
    mWolves.clear();
    mWaterCreatures.clear();

    mMushroomCows.push_back(new MobSpawnerData(8, EntityType::MUSHROOM_COW, 4, 8));
}

Biome::EBiomeBaseClass MushroomIslandBiome::getBaseClass()
{
    return eBiomeBaseClass_MushroomIsland;
}
