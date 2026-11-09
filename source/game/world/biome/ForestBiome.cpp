#include "game/world/biome/ForestBiome.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/PerlinSimplexNoise.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/biome/BiomeDecorator.hpp"
#include "game/world/generation/BigTreeFeature.hpp"
#include "game/world/generation/BirchFeature.hpp"
#include "game/world/generation/DoublePlantFeature.hpp"
#include "game/world/generation/HugeMushroomFeature.hpp"
#include "game/world/generation/RoofTreeFeature.hpp"
#include "game/world/generation/TreeFeature.hpp"

ForestBiome::ForestBiome(EBiomeIDs biomeId, EType type, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_Forest, properties),
      mType(type)
{
    if (type != eType_Flower)
    {
        mDecorator->grassPerChunk = 2;
        mDecorator->treesPerChunk = 10;
    }
    else
    {
        mDecorator->treesPerChunk = 6;
        mDecorator->grassPerChunk = 1;
        mDecorator->flowersPerChunk = 100;
        mCreatures.push_back(new MobSpawnerData(4, EntityType::RABBIT, 2, 3));
    }

    if (type == eType_Normal)
        mWolves.push_back(new MobSpawnerData(5, EntityType::WOLF, 4, 4));

    if (type == eType_Roofed)
        mDecorator->treesPerChunk = -999;
}

AbstractTreeFeature* ForestBiome::getTreeFeature(Random& random)
{
    if (mType == eType_Roofed && random.nextInt(3) > 0)
        return new RoofTreeFeature(false);
    if (mType == eType_Birch || random.nextInt(5) == 0)
        return new BirchFeature(false, false);
    if (random.nextInt(10) == 0)
        return new BigTreeFeature(false);
    return new TreeFeature(false);
}

FlowerBlock::Flower* ForestBiome::getRandomFlower(Random& random, const BlockPos& pos)
{
    if (mType == eType_Flower)
    {
        double noise = BIOME_INFO_NOISE->getValue(pos.x / 48.0, pos.z / 48.0);
        double value = Mth::clamp((noise + 1.0) * 0.5, 0.0, 0.9999);
        int index = (int)(value * FlowerBlock::Flower::FLOWER_COUNT);
        FlowerBlock::Flower* flower = FlowerBlock::Flower::FLOWERS[index];
        if (flower == FlowerBlock::Flower::BLUE_ORCHID)
            return FlowerBlock::Flower::POPPY;
        return flower;
    }

    return Biome::getRandomFlower(random, pos);
}

void ForestBiome::addMushrooms(Level* level, Random& random, const BlockPos& pos)
{
    for (int row = 0; row < 4; ++row)
    {
        for (int column = 0; column < 4; ++column)
        {
            const int x = row * 4 + 9 + random.nextInt(3);
            const int z = column * 4 + 9 + random.nextInt(3);
            BlockPos origin = pos.offset(x, 0, z);
            BlockPos top = level->getHeightmapPos(origin);
            if (random.nextInt(20) == 0)
            {
                HugeMushroomFeature* mushroom = new HugeMushroomFeature();
                mushroom->place(level, random, top);
                delete mushroom;
            }
            else
            {
                AbstractTreeFeature* tree = getTreeFeature(random);
                tree->init();
                if (tree->place(level, random, top))
                    tree->postPlaceTree(level, random, top);
                delete tree;
            }
        }
    }
}

void ForestBiome::addDoublePlants(Level* level, Random& random, const BlockPos& pos, int count)
{
    for (int i = 0; i < count; ++i)
    {
        int plant = random.nextInt(3);
        if (plant == 0)
            DOUBLE_PLANT_FEATURE->setNextPlantType(DoublePlantBlock::Variant::SYRINGA);
        else if (plant == 1)
            DOUBLE_PLANT_FEATURE->setNextPlantType(DoublePlantBlock::Variant::ROSE);
        else if (plant == 2)
            DOUBLE_PLANT_FEATURE->setNextPlantType(DoublePlantBlock::Variant::PAEONIA);

        for (int attempt = 0; attempt < 5; ++attempt)
        {
            int x = random.nextInt(16) + 8;
            int z = random.nextInt(16) + 8;
            int y = random.nextInt(level->getHeightmapPos(pos.offset(x, 0, z)).y + 32);
            if (DOUBLE_PLANT_FEATURE->place(level, random, BlockPos(pos.x + x, y, pos.z + z)))
                break;
        }
    }
}

void ForestBiome::decorate(Level* level, Random& random, const BlockPos& pos)
{
    if (mType == eType_Roofed)
        addMushrooms(level, random, pos);

    int count = random.nextInt(5) - 3;
    if (mType == eType_Flower)
        count += 2;
    addDoublePlants(level, random, pos, count);
    Biome::decorate(level, random, pos);
}

unsigned int ForestBiome::getGrassColor(const BlockPos& pos)
{
    unsigned int grass = Biome::getGrassColor(pos);
    if (mType == eType_Roofed)
    {
        unsigned int modifier = Minecraft::GetInstance()->getColourTable()->getColour(
            eMinecraftColour_Grass_Roofed_Forest_Modifier);
        return ((grass & 0xFEFEFE) + (modifier & 0xFEFEFE)) >> 1;
    }
    return grass;
}

Biome::EBiomeBaseClass ForestBiome::getBaseClass()
{
    return eBiomeBaseClass_Forest;
}
