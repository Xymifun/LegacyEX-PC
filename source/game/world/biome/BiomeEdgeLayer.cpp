#include "game/world/biome/BiomeEdgeLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/core/PIXNamedEvents.hpp"

BiomeEdgeLayer::BiomeEdgeLayer(long long seed, boost::shared_ptr<Layer> parent) : Layer(seed)
{
    this->parent = parent;
}

BiomeEdgeLayer::~BiomeEdgeLayer()
{
}

bool BiomeEdgeLayer::isValidTemperatureEdge(int firstBiomeId, int secondBiomeId)
{
    if (Layer::isSame(firstBiomeId, secondBiomeId))
    {
        return true;
    }

    Biome* firstBiome = Biome::getBiome(firstBiomeId);
    Biome* secondBiome = Biome::getBiome(secondBiomeId);

    if (firstBiome != NULL && secondBiome != NULL)
    {
        Biome::TemperatureCategory firstCategory = firstBiome->getTemperatureCategory();
        Biome::TemperatureCategory secondCategory = secondBiome->getTemperatureCategory();

        return firstCategory == secondCategory || firstCategory == Biome::eTemperatureCategory_Medium
               || secondCategory == Biome::eTemperatureCategory_Medium;
    }

    return false;
}

bool BiomeEdgeLayer::checkEdge(arrayWithLength<int> parentArea, arrayWithLength<int> area, int x, int z, int width,
                               int centre, int edge, int replacement)
{
    if (Layer::isSame(centre, edge))
    {
        int north = parentArea[x + 1 + (z + 1 - 1) * (width + 2)];
        int east = parentArea[x + 1 + 1 + (z + 1) * (width + 2)];
        int west = parentArea[x + 1 - 1 + (z + 1) * (width + 2)];
        int south = parentArea[x + 1 + (z + 1 + 1) * (width + 2)];

        if (isValidTemperatureEdge(north, edge) && isValidTemperatureEdge(east, edge)
            && isValidTemperatureEdge(west, edge) && isValidTemperatureEdge(south, edge))
        {
            area[x + z * width] = centre;
        }
        else
        {
            area[x + z * width] = replacement;
        }

        return true;
    }

    return false;
}

bool BiomeEdgeLayer::checkEdgeStrict(arrayWithLength<int> parentArea, arrayWithLength<int> area, int x, int z,
                                     int width, int centre, int edge, int replacement)
{
    if (centre == edge)
    {
        int north = parentArea[x + 1 + (z + 1 - 1) * (width + 2)];
        int east = parentArea[x + 1 + 1 + (z + 1) * (width + 2)];
        int west = parentArea[x + 1 - 1 + (z + 1) * (width + 2)];
        int south = parentArea[x + 1 + (z + 1 + 1) * (width + 2)];

        if (Layer::isSame(north, edge) && Layer::isSame(east, edge) && Layer::isSame(west, edge)
            && Layer::isSame(south, edge))
        {
            area[x + z * width] = centre;
        }
        else
        {
            area[x + z * width] = replacement;
        }

        return true;
    }

    return false;
}

arrayWithLength<int> BiomeEdgeLayer::getArea(int x, int z, int width, int height)
{
    arrayWithLength<int> parentArea = parent->getArea(x - 1, z - 1, width + 2, height + 2);

    PIXBeginNamedEvent(0.0f, "BiomeEdgeLayer::getArea");

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int localZ = 0; localZ < height; localZ++)
    {
        for (int localX = 0; localX < width; localX++)
        {
            initRandom((long long)(localX + x), (long long)(localZ + z));

            int centre = parentArea[localX + 1 + (localZ + 1) * (width + 2)];

            if (!checkEdge(parentArea, area, localX, localZ, width, centre, Biome::EXTREME_HILLS->id,
                           Biome::EXTREME_HILLS_EDGE->id)
                && !checkEdgeStrict(parentArea, area, localX, localZ, width, centre, Biome::MESA_ROCK->id,
                                    Biome::MESA->id)
                && !checkEdgeStrict(parentArea, area, localX, localZ, width, centre, Biome::MESA_CLEAR_ROCK->id,
                                    Biome::MESA->id)
                && !checkEdgeStrict(parentArea, area, localX, localZ, width, centre, Biome::MEGA_TAIGA->id,
                                    Biome::TAIGA->id))
            {
                if (centre == Biome::DESERT->id)
                {
                    int north = parentArea[localX + 1 + (localZ + 1 - 1) * (width + 2)];
                    int east = parentArea[localX + 1 + 1 + (localZ + 1) * (width + 2)];
                    int west = parentArea[localX + 1 - 1 + (localZ + 1) * (width + 2)];
                    int south = parentArea[localX + 1 + (localZ + 1 + 1) * (width + 2)];

                    if (north != Biome::ICE_PLAINS->id && east != Biome::ICE_PLAINS->id && west != Biome::ICE_PLAINS->id
                        && south != Biome::ICE_PLAINS->id)
                    {
                        area[localX + localZ * width] = centre;
                    }
                    else
                    {
                        area[localX + localZ * width] = Biome::EXTREME_HILLS_PLUS_TREES->id;
                    }
                }
                else if (centre == Biome::SWAMPLAND->id)
                {
                    int north = parentArea[localX + 1 + (localZ + 1 - 1) * (width + 2)];
                    int east = parentArea[localX + 1 + 1 + (localZ + 1) * (width + 2)];
                    int west = parentArea[localX + 1 - 1 + (localZ + 1) * (width + 2)];
                    int south = parentArea[localX + 1 + (localZ + 1 + 1) * (width + 2)];

                    if (north != Biome::DESERT->id && east != Biome::DESERT->id && west != Biome::DESERT->id
                        && south != Biome::DESERT->id && north != Biome::COLD_TAIGA->id && east != Biome::COLD_TAIGA->id
                        && west != Biome::COLD_TAIGA->id && south != Biome::COLD_TAIGA->id
                        && north != Biome::ICE_PLAINS->id && east != Biome::ICE_PLAINS->id
                        && west != Biome::ICE_PLAINS->id && south != Biome::ICE_PLAINS->id)
                    {
                        if (north != Biome::JUNGLE->id && south != Biome::JUNGLE->id && east != Biome::JUNGLE->id
                            && west != Biome::JUNGLE->id)
                        {
                            area[localX + localZ * width] = centre;
                        }
                        else
                        {
                            area[localX + localZ * width] = Biome::JUNGLE_EDGE->id;
                        }
                    }
                    else
                    {
                        area[localX + localZ * width] = Biome::PLAINS->id;
                    }
                }
                else
                {
                    area[localX + localZ * width] = centre;
                }
            }
        }
    }

    PIXEndNamedEvent();

    return area;
}
