#include "game/world/biome/SavannaBiome.hpp"
#include "game/core/EntityType.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/BiomeDecorator.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/DoublePlantFeature.hpp"
#include "game/world/generation/SavannaTreeFeature.hpp"
#include "game/world/generation/TreeFeature.hpp"

extern DoublePlantBlock::Variant* gDoublePlantGrass;

SavannaBiome::SavannaBiome(EBiomeIDs biomeId, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_Savanna, properties)
{
    mCreatures.push_back(new MobSpawnerData(1, EntityType::HORSE, 2, 6));
    mCreatures.push_back(new MobSpawnerData(1, EntityType::DONKEY, 1, 1));
    if (getDepth() > 1.1F)
    {
        mCreatures.push_back(new MobSpawnerData(8, EntityType::LLAMA, 4, 4));
    }

    mDecorator->treesPerChunk = 1;
    mDecorator->flowersPerChunk = 4;
    mDecorator->grassPerChunk = 20;
}

AbstractTreeFeature* SavannaBiome::getTreeFeature(Random& random)
{
    if (random.nextInt(5) > 0)
    {
        return new SavannaTreeFeature(false);
    }

    return new TreeFeature(false);
}

void SavannaBiome::decorate(Level* level, Random& random, const BlockPos& pos)
{
    DOUBLE_PLANT_FEATURE->setNextPlantType(gDoublePlantGrass);

    for (int i = 0; i < 7; ++i)
    {
        int x = random.nextInt(16) + 8;
        int z = random.nextInt(16) + 8;
        int y = random.nextInt(level->getHeightmapPos(pos.offset(x, 0, z)).y + 32);
        DOUBLE_PLANT_FEATURE->place(level, random, pos.offset(x, y, z));
    }

    Biome::decorate(level, random, pos);
}

Biome::EBiomeBaseClass SavannaBiome::getBaseClass()
{
    return eBiomeBaseClass_Savanna;
}
