#include "game/world/generation/RegionHillsLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/world/biome/Biome.hpp"

RegionHillsLayer::RegionHillsLayer(long long seed, boost::shared_ptr<Layer> parent, boost::shared_ptr<Layer> riverLayer)
    : Layer(seed)
{
    this->parent = parent;
    this->riverLayer = riverLayer;
}

arrayWithLength<int> RegionHillsLayer::getArea(int x, int z, int width, int height)
{
    arrayWithLength<int> parentArea = parent->getArea(x - 1, z - 1, width + 2, height + 2);
    arrayWithLength<int> riverArea = riverLayer->getArea(x - 1, z - 1, width + 2, height + 2);

    PIXBeginNamedEvent(0.0f, "RegionHillsLayer::getArea");

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            initRandom(j + x, i + z);

            int river = riverArea[j + 1 + (i + 1) * (width + 2)];
            int centre = parentArea[j + 1 + (i + 1) * (width + 2)];
            bool mutate = (river - 2) % 29 == 0;

            if (!isShallowOcean(centre) && river >= 2 && (river - 2) % 29 == 1 && centre < 128)
            {
                Biome* mutatedBiome = Biome::getBiome(centre + 128);

                if (mutatedBiome != NULL)
                {
                    area[j + i * width] = centre + 128;
                }
                else
                {
                    area[j + i * width] = centre;
                }
            }
            else if (nextRandom(3) != 0 && !mutate)
            {
                area[j + i * width] = centre;
            }
            else
            {
                int hills = centre;

                if (centre == Biome::DESERT->id)
                {
                    hills = Biome::DESERT_HILLS->id;
                }
                else if (centre == Biome::FOREST->id)
                {
                    hills = Biome::FOREST_HILLS->id;
                }
                else if (centre == Biome::BIRCH_FOREST->id)
                {
                    hills = Biome::BIRCH_FOREST_HILLS->id;
                }
                else if (centre == Biome::ROOFED_FOREST->id)
                {
                    hills = Biome::PLAINS->id;
                }
                else if (centre == Biome::TAIGA->id)
                {
                    hills = Biome::TAIGA_HILLS->id;
                }
                else if (centre == Biome::MEGA_TAIGA->id)
                {
                    hills = Biome::MEGA_TAIGA_HILLS->id;
                }
                else if (centre == Biome::COLD_TAIGA->id)
                {
                    hills = Biome::COLD_TAIGA_HILLS->id;
                }
                else if (centre == Biome::PLAINS->id)
                {
                    if (nextRandom(3) == 0)
                    {
                        hills = Biome::FOREST_HILLS->id;
                    }
                    else
                    {
                        hills = Biome::FOREST->id;
                    }
                }
                else if (centre == Biome::ICE_PLAINS->id)
                {
                    hills = Biome::ICE_MOUNTAINS->id;
                }
                else if (centre == Biome::JUNGLE->id)
                {
                    hills = Biome::JUNGLE_HILLS->id;
                }
                else if (centre == Biome::eBiomeIDs_Ocean)
                {
                    hills = Biome::eBiomeIDs_DeepOcean;
                }
                else if (centre == Biome::eBiomeIDs_LukewarmOcean)
                {
                    hills = Biome::eBiomeIDs_DeepLukewarmOcean;
                }
                else if (centre == Biome::eBiomeIDs_ColdOcean)
                {
                    hills = Biome::eBiomeIDs_DeepColdOcean;
                }
                else if (centre == Biome::eBiomeIDs_FrozenOcean)
                {
                    hills = Biome::eBiomeIDs_DeepFrozenOcean;
                }
                else if (centre == Biome::EXTREME_HILLS->id)
                {
                    hills = Biome::EXTREME_HILLS_PLUS_TREES->id;
                }
                else if (centre == Biome::SAVANNA->id)
                {
                    hills = Biome::SAVANNA_PLATEAU->id;
                }
                else if (isSame(centre, Biome::MESA_ROCK->id))
                {
                    hills = Biome::MESA->id;
                }
                else if (isOcean(centre) && !isShallowOcean(centre) && nextRandom(3) == 0)
                {
                    if (nextRandom(2) == 0)
                    {
                        hills = Biome::PLAINS->id;
                    }
                    else
                    {
                        hills = Biome::FOREST->id;
                    }
                }

                if (mutate && hills != centre)
                {
                    Biome* mutatedBiome = Biome::getBiome(hills + 128);

                    if (mutatedBiome != NULL)
                    {
                        hills = hills + 128;
                    }
                    else
                    {
                        hills = centre;
                    }
                }

                if (hills == centre)
                {
                    area[j + i * width] = centre;
                }
                else
                {
                    int up = parentArea[j + 1 + i * (width + 2)];
                    int right = parentArea[j + 2 + (i + 1) * (width + 2)];
                    int left = parentArea[j + (i + 1) * (width + 2)];
                    int down = parentArea[j + 1 + (i + 2) * (width + 2)];
                    int matchingNeighbours = 0;

                    if (isSame(up, centre))
                    {
                        matchingNeighbours++;
                    }

                    if (isSame(right, centre))
                    {
                        matchingNeighbours++;
                    }

                    if (isSame(left, centre))
                    {
                        matchingNeighbours++;
                    }

                    if (isSame(down, centre))
                    {
                        matchingNeighbours++;
                    }

                    if (matchingNeighbours >= 3)
                    {
                        area[j + i * width] = hills;
                    }
                    else
                    {
                        area[j + i * width] = centre;
                    }
                }
            }
        }
    }

    PIXEndNamedEvent();

    return area;
}
