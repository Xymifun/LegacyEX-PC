#include "game/world/biome/MutatedSavannaBiome.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/BiomeDecorator.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirtBlock.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"

MutatedSavannaBiome::MutatedSavannaBiome(EBiomeIDs biomeId, BiomeProperties* properties)
    : SavannaBiome(biomeId, properties)
{
    mDecorator->treesPerChunk = 2;
    mDecorator->flowersPerChunk = 2;
    mDecorator->grassPerChunk = 5;
}

void MutatedSavannaBiome::buildSurfaceAt(Level* level, Random& random, ChunkPrimer* primer, int x, int z, double noise)
{
    mTopBlock = BlockPair(2, 0);
    mFillerBlock = BlockPair(3, 0);

    if (noise > 1.75)
    {
        mTopBlock = STONE_BLOCK;
        mFillerBlock = STONE_BLOCK;
    }
    else if (noise > -0.5)
    {
        mTopBlock = BlockPair(
            3, Blocks::dirt->convertBlockStateToLegacyData(
                   Blocks::dirt->defaultBlockState()->setValue(DirtBlock::VARIANT, DirtBlock::Variant::COARSE_DIRT)));
    }

    buildSurfaceAtDefault(level, random, primer, x, z, noise);
}

void MutatedSavannaBiome::decorate(Level* level, Random& random, const BlockPos& pos)
{
    mDecorator->decorate(level, &random, this, pos);
}
