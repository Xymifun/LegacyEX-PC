#include "game/world/biome/ExtremeHillsBiome.hpp"
#include "game/core/EntityType.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/BiomeDecorator.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/MonsterEggBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/generation/OreFeature.hpp"
#include "game/world/generation/SpruceFeature.hpp"
#include "game/core/PIXNamedEvents.hpp"

ExtremeHillsBiome::ExtremeHillsBiome(EBiomeIDs biomeId, EType type, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_ExtremeHills, properties)
{
    if (type == eType_ExtraTrees)
    {
        mDecorator->treesPerChunk = 3;
    }

    mCreatures.push_back(new MobSpawnerData(5, EntityType::LLAMA, 4, 6));
    mType = type;

    mMonsterEggFeature = new OreFeature(Blocks::monster_egg->defaultBlockState()->setValue<MonsterEggBlock::Variant*>(
                                            MonsterEggBlock::VARIANT, MonsterEggBlock::Variant::STONE),
                                        9);
}

ExtremeHillsBiome::~ExtremeHillsBiome()
{
    delete mMonsterEggFeature;
}

AbstractTreeFeature* ExtremeHillsBiome::getTreeFeature(Random& random)
{
    if (random.nextInt(3) > 0)
    {
        return new SpruceFeature(false);
    }

    return Biome::getTreeFeature(random);
}

void ExtremeHillsBiome::decorate(Level* level, Random& random, const BlockPos& pos)
{
    Biome::decorate(level, random, pos);

    int oreCount = random.nextInt(6) + 3;
    for (int i = 0; i < oreCount; ++i)
    {
        int x = random.nextInt(16);
        int y = random.nextInt(28) + 4;
        int z = random.nextInt(16);
        BlockPos orePos = pos.offset(x, y, z);
        if (level->getBlockState(orePos)->getBlock() == Blocks::stone)
        {
            level->setBlock(orePos, Blocks::emerald_ore->defaultBlockState(), Block::UPDATE_CLIENTS, false);
        }
    }

    for (int i = 0; i < 7; ++i)
    {
        int x = random.nextInt(16);
        int y = random.nextInt(64);
        int z = random.nextInt(16);
        mMonsterEggFeature->place(level, random, pos.offset(x, y, z));
    }
}

void ExtremeHillsBiome::buildSurfaceAt(Level* level, Random& random, ChunkPrimer* primer, int x, int z, double noise)
{
    PIXBeginNamedEvent(0.0F, "ExtremeHillsBiome::buildSurfaceAtDefault");

    mTopBlock = BlockPair(2, 0);
    mFillerBlock = BlockPair(3, 0);

    if ((noise < -1.0 || noise > 2.0) && mType == eType_Mutated)
    {
        mTopBlock = BlockPair(13, 0);
        mFillerBlock = BlockPair(13, 0);
    }
    else if (noise > 1.0 && mType != eType_ExtraTrees)
    {
        mTopBlock = BlockPair(1, 0);
        mFillerBlock = BlockPair(1, 0);
    }

    buildSurfaceAtDefault(level, random, primer, x, z, noise);
    PIXEndNamedEvent();
}

Biome::EBiomeBaseClass ExtremeHillsBiome::getBaseClass()
{
    return eBiomeBaseClass_ExtremeHills;
}
