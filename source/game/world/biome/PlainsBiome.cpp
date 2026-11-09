#include "game/world/biome/PlainsBiome.hpp"
#include "game/core/EntityType.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/biome/BiomeDecorator.hpp"
#include "game/world/generation/BigTreeFeature.hpp"
#include "game/world/generation/DoublePlantFeature.hpp"
#include "game/world/generation/TreeFeature.hpp"
#include "game/core/PerlinSimplexNoise.hpp"

PlainsBiome::PlainsBiome(EBiomeIDs biomeId, bool sunflower, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_Plains, properties),
      mIsSunflower(sunflower)
{
    mCreatures.push_back(new MobSpawnerData(5, EntityType::HORSE, 2, 6));
    mCreatures.push_back(new MobSpawnerData(1, EntityType::DONKEY, 1, 3));

    mDecorator->treesPerChunk = 0;
    mDecorator->extraTreeChance = 0.05F;
    mDecorator->flowersPerChunk = 4;
    mDecorator->grassPerChunk = 10;
}

FlowerBlock::Flower* PlainsBiome::getRandomFlower(Random& random, const BlockPos& pos)
{
    const double temperature = BIOME_INFO_NOISE->getValue(pos.x / 200.0, pos.z / 200.0);
    if (temperature < -0.8)
    {
        const int tulip = random.nextInt(4);
        switch (tulip)
        {
        case 0:
            return FlowerBlock::Flower::ORANGE_TULIP;
        case 1:
            return FlowerBlock::Flower::RED_TULIP;
        case 2:
            return FlowerBlock::Flower::PINK_TULIP;
        case 3:
        default:
            return FlowerBlock::Flower::WHITE_TULIP;
        }
    }
    else if (random.nextInt(3) > 0)
    {
        const int flower = random.nextInt(3);
        if (flower == 0)
        {
            return FlowerBlock::Flower::POPPY;
        }
        if (flower == 1)
        {
            return FlowerBlock::Flower::HOUSTONIA;
        }
        return FlowerBlock::Flower::OXEYE_DAISY;
    }
    else
    {
        return FlowerBlock::Flower::DANDELION;
    }
}

AbstractTreeFeature* PlainsBiome::getTreeFeature(Random& random)
{
    if (random.nextInt(3) == 0)
    {
        return new BigTreeFeature(false);
    }

    return new TreeFeature(false);
}

void PlainsBiome::decorate(Level* level, Random& random, const BlockPos& pos)
{
    const double temperature = BIOME_INFO_NOISE->getValue((pos.x + 8) / 200.0, (pos.z + 8) / 200.0);

    if (temperature < -0.8)
    {
        mDecorator->flowersPerChunk = 15;
        mDecorator->grassPerChunk = 5;
    }
    else
    {
        mDecorator->flowersPerChunk = 4;
        mDecorator->grassPerChunk = 10;
        DOUBLE_PLANT_FEATURE->setNextPlantType(gDoublePlantGrass);

        for (int i = 0; i < 7; ++i)
        {
            const int x = random.nextInt(16) + 8;
            const int z = random.nextInt(16) + 8;
            const int y = random.nextInt(level->getHeightmapPos(pos.offset(x, 0, z)).y + 32);
            DOUBLE_PLANT_FEATURE->place(level, random, pos.offset(x, y, z));
        }
    }

    if (mIsSunflower)
    {
        DOUBLE_PLANT_FEATURE->setNextPlantType(DoublePlantBlock::Variant::SUNFLOWER);
        for (int i = 0; i < 10; ++i)
        {
            const int x = random.nextInt(16) + 8;
            const int z = random.nextInt(16) + 8;
            const int y = random.nextInt(level->getHeightmapPos(pos.offset(x, 0, z)).y + 32);
            DOUBLE_PLANT_FEATURE->place(level, random, pos.offset(x, y, z));
        }
    }

    Biome::decorate(level, random, pos);
}

Biome::EBiomeBaseClass PlainsBiome::getBaseClass()
{
    return eBiomeBaseClass_Plains;
}
