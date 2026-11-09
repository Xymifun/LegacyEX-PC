#include "game/world/biome/StoneBeachBiome.hpp"

#include "game/world/biome/BiomeDecorator.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/world/block/Blocks.hpp"

StoneBeachBiome::StoneBeachBiome(EBiomeIDs biomeId, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_StoneBeach, properties)
{
    mCreatures.clear();

    mTopBlock = BlockPair(BlockId::STONE,
                          Blocks::stone->convertBlockStateToLegacyData(Blocks::stone->defaultBlockState()));
    mFillerBlock = BlockPair(BlockId::STONE,
                             Blocks::stone->convertBlockStateToLegacyData(Blocks::stone->defaultBlockState()));

    mDecorator->treesPerChunk = -999;
    mDecorator->deadBushPerChunk = 0;
    mDecorator->reedsPerChunk = 0;
    mDecorator->cactiPerChunk = 0;
}

Biome::EBiomeBaseClass StoneBeachBiome::getBaseClass()
{
    return eBiomeBaseClass_StoneBeach;
}
