#include "game/world/biome/BiomeInitLayer.hpp"

#include "game/core/CustomizableSourceSettings.hpp"
#include "game/core/IntCache.hpp"
#include "game/world/LevelType.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/core/PIXNamedEvents.hpp"

BiomeInitLayer::BiomeInitLayer(long long seed, boost::shared_ptr<Layer> parent, LevelType* levelType,
                               SuperflatConfig* levelTypeOptions)
    : Layer(seed)
{
    this->parent = parent;

    if (levelType == LevelType::DEFAULT_1_1)
    {
        warmBiomes = arrayWithLength<Biome*>(6, true);
        warmBiomes.data[0] = Biome::DESERT;
        warmBiomes.data[1] = Biome::FOREST;
        warmBiomes.data[2] = Biome::EXTREME_HILLS;
        warmBiomes.data[3] = Biome::SWAMPLAND;
        warmBiomes.data[4] = Biome::PLAINS;
        warmBiomes.data[5] = Biome::TAIGA;
    }
    else
    {
        warmBiomes = arrayWithLength<Biome*>(6, true);
        warmBiomes.data[0] = Biome::DESERT;
        warmBiomes.data[1] = Biome::DESERT;
        warmBiomes.data[2] = Biome::DESERT;
        warmBiomes.data[3] = Biome::SAVANNA;
        warmBiomes.data[4] = Biome::SAVANNA;
        warmBiomes.data[5] = Biome::PLAINS;
    }

    mediumBiomes = arrayWithLength<Biome*>(6, true);
    mediumBiomes.data[0] = Biome::FOREST;
    mediumBiomes.data[1] = Biome::ROOFED_FOREST;
    mediumBiomes.data[2] = Biome::EXTREME_HILLS;
    mediumBiomes.data[3] = Biome::PLAINS;
    mediumBiomes.data[4] = Biome::BIRCH_FOREST;
    mediumBiomes.data[5] = Biome::SWAMPLAND;

    coldBiomes = arrayWithLength<Biome*>(4, true);
    coldBiomes.data[0] = Biome::FOREST;
    coldBiomes.data[1] = Biome::EXTREME_HILLS;
    coldBiomes.data[2] = Biome::TAIGA;
    coldBiomes.data[3] = Biome::PLAINS;

    iceBiomes = arrayWithLength<Biome*>(4, true);
    iceBiomes.data[0] = Biome::ICE_PLAINS;
    iceBiomes.data[1] = Biome::ICE_PLAINS;
    iceBiomes.data[2] = Biome::ICE_PLAINS;
    iceBiomes.data[3] = Biome::COLD_TAIGA;

    if (levelType != LevelType::DEFAULT_1_1 && levelType == LevelType::CUSTOMIZED)
    {
        settings = CustomizableSourceSettings::Builder::fromString(levelTypeOptions)->build();
    }
    else
    {
        settings = NULL;
    }
}

BiomeInitLayer::~BiomeInitLayer()
{
    delete[] warmBiomes.data;
    delete[] mediumBiomes.data;
    delete[] coldBiomes.data;
    delete[] iceBiomes.data;
}

arrayWithLength<int> BiomeInitLayer::getArea(int x, int z, int width, int height)
{
    arrayWithLength<int> parentArea = parent->getArea(x, z, width, height);

    PIXBeginNamedEvent(0.0f, "BiomeInitLayer::getArea");

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int row = 0; row < height; row++)
    {
        for (int column = 0; column < width; column++)
        {
            initRandom((long long)(column + x), (long long)(row + z));

            int biomeId = parentArea.data[column + row * width];
            int special = (biomeId & 0xf00) >> 8;
            biomeId &= ~0xf00;

            if (settings != NULL && settings->fixedBiome >= 0)
            {
                area.data[column + row * width] = settings->fixedBiome;
            }
            else if (isOcean(biomeId) || biomeId == Biome::MUSHROOM_ISLAND->id)
            {
                area.data[column + row * width] = biomeId;
            }
            else if (biomeId == 1)
            {
                if (special > 0)
                {
                    if (nextRandom(3) == 0)
                    {
                        area.data[column + row * width] = Biome::MESA_CLEAR_ROCK->id;
                    }
                    else
                    {
                        area.data[column + row * width] = Biome::MESA_ROCK->id;
                    }
                }
                else
                {
                    area.data[column + row * width] = warmBiomes.data[nextRandom(warmBiomes.length)]->id;
                }
            }
            else if (biomeId == 2)
            {
                if (special > 0)
                {
                    area.data[column + row * width] = Biome::JUNGLE->id;
                }
                else
                {
                    area.data[column + row * width] = mediumBiomes.data[nextRandom(mediumBiomes.length)]->id;
                }
            }
            else if (biomeId == 3)
            {
                if (special > 0)
                {
                    area.data[column + row * width] = Biome::MEGA_TAIGA->id;
                }
                else
                {
                    area.data[column + row * width] = coldBiomes.data[nextRandom(coldBiomes.length)]->id;
                }
            }
            else if (biomeId == 4)
            {
                area.data[column + row * width] = iceBiomes.data[nextRandom(iceBiomes.length)]->id;
            }
            else
            {
                area.data[column + row * width] = Biome::MUSHROOM_ISLAND->id;
            }
        }
    }

    PIXEndNamedEvent();

    return area;
}
