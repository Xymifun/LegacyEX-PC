#include "game/world/generation/ShoreLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/MesaBiome.hpp"

ShoreLayer::ShoreLayer(long long seed, boost::shared_ptr<Layer> parentLayer) : Layer(seed)
{
    parent = parentLayer;
}

bool ShoreLayer::isJungleCompatible(int biomeId)
{
    if (Biome::getBiome(biomeId) != NULL && Biome::getBiome(biomeId)->getBaseClass() == Biome::eBiomeBaseClass_Jungle)
    {
        return true;
    }

    return biomeId == Biome::eBiomeIDs_JungleEdge || biomeId == Biome::eBiomeIDs_Jungle
           || biomeId == Biome::eBiomeIDs_JungleHills || biomeId == Biome::eBiomeIDs_Forest
           || biomeId == Biome::eBiomeIDs_Taiga || isOcean(biomeId);
}

void ShoreLayer::replaceIfNeighborOcean(arrayWithLength<int> parentArea, arrayWithLength<int> area, int x, int z,
                                        int width, int centre, int replacement)
{
    if (isOcean(centre))
    {
        area[x + z * width] = centre;
    }
    else
    {
        int north = parentArea[x + 1 + (z + 1 - 1) * (width + 2)];
        int east = parentArea[x + 1 + 1 + (z + 1) * (width + 2)];
        int west = parentArea[x + 1 - 1 + (z + 1) * (width + 2)];
        int south = parentArea[x + 1 + (z + 1 + 1) * (width + 2)];

        if (!isOcean(north) && !isOcean(east) && !isOcean(west) && !isOcean(south))
        {
            area[x + z * width] = centre;
        }
        else
        {
            area[x + z * width] = replacement;
        }
    }
}

bool ShoreLayer::isMesa(int biomeId)
{
    return dynamic_cast<MesaBiome*>(Biome::getBiome(biomeId)) != NULL;
}

arrayWithLength<int> ShoreLayer::getArea(int x, int z, int width, int height)
{
    arrayWithLength<int> parentArea = parent->getArea(x - 1, z - 1, width + 2, height + 2);

    PIXBeginNamedEvent(0.0f, "ShoreLayer::getArea");

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int localZ = 0; localZ < height; localZ++)
    {
        for (int localX = 0; localX < width; localX++)
        {
            initRandom((long long)(localX + x), (long long)(localZ + z));

            int centre = parentArea[localX + 1 + (localZ + 1) * (width + 2)];
            Biome* biome = Biome::getBiome(centre);

            if (centre == Biome::MUSHROOM_ISLAND->id)
            {
                int north = parentArea[localX + 1 + (localZ + 1 - 1) * (width + 2)];
                int east = parentArea[localX + 1 + 1 + (localZ + 1) * (width + 2)];
                int west = parentArea[localX + 1 - 1 + (localZ + 1) * (width + 2)];
                int south = parentArea[localX + 1 + (localZ + 1 + 1) * (width + 2)];

                if (isShallowOcean(north) || isShallowOcean(east) || isShallowOcean(west) || isShallowOcean(south))
                {
                    area[localX + localZ * width] = Biome::MUSHROOM_ISLAND_SHORE->id;
                }
                else
                {
                    area[localX + localZ * width] = centre;
                }
            }
            else if (biome != NULL && biome->getBaseClass() == Biome::eBiomeBaseClass_Jungle)
            {
                int north = parentArea[localX + 1 + (localZ + 1 - 1) * (width + 2)];
                int east = parentArea[localX + 1 + 1 + (localZ + 1) * (width + 2)];
                int west = parentArea[localX + 1 - 1 + (localZ + 1) * (width + 2)];
                int south = parentArea[localX + 1 + (localZ + 1 + 1) * (width + 2)];

                if (!isJungleCompatible(north) || !isJungleCompatible(east) || !isJungleCompatible(west)
                    || !isJungleCompatible(south))
                {
                    area[localX + localZ * width] = Biome::JUNGLE_EDGE->id;
                }
                else if (isOcean(north) || isOcean(east) || isOcean(west) || isOcean(south))
                {
                    area[localX + localZ * width] = Biome::BEACH->id;
                }
                else
                {
                    area[localX + localZ * width] = centre;
                }
            }
            else if (centre == Biome::EXTREME_HILLS->id || centre == Biome::EXTREME_HILLS_PLUS_TREES->id
                     || centre == Biome::EXTREME_HILLS_EDGE->id)
            {
                replaceIfNeighborOcean(parentArea, area, localX, localZ, width, centre, Biome::STONE_BEACH->id);
            }
            else if (biome != NULL && biome->isSnowCovered())
            {
                replaceIfNeighborOcean(parentArea, area, localX, localZ, width, centre, Biome::COLD_BEACH->id);
            }
            else if (centre == Biome::MESA->id || centre == Biome::MESA_ROCK->id)
            {
                int north = parentArea[localX + 1 + (localZ + 1 - 1) * (width + 2)];
                int east = parentArea[localX + 1 + 1 + (localZ + 1) * (width + 2)];
                int west = parentArea[localX + 1 - 1 + (localZ + 1) * (width + 2)];
                int south = parentArea[localX + 1 + (localZ + 1 + 1) * (width + 2)];

                if (!isOcean(north) && !isOcean(east) && !isOcean(west) && !isOcean(south)
                    && !(isMesa(north) && isMesa(east) && isMesa(west) && isMesa(south)))
                {
                    area[localX + localZ * width] = Biome::DESERT->id;
                }
                else
                {
                    area[localX + localZ * width] = centre;
                }
            }
            else if (!isOcean(centre) && centre != Biome::RIVER->id && centre != Biome::SWAMPLAND->id)
            {
                int north = parentArea[localX + 1 + (localZ + 1 - 1) * (width + 2)];
                int east = parentArea[localX + 1 + 1 + (localZ + 1) * (width + 2)];
                int west = parentArea[localX + 1 - 1 + (localZ + 1) * (width + 2)];
                int south = parentArea[localX + 1 + (localZ + 1 + 1) * (width + 2)];

                if (isOcean(north) || isOcean(east) || isOcean(west) || isOcean(south))
                {
                    area[localX + localZ * width] = Biome::BEACH->id;
                }
                else
                {
                    area[localX + localZ * width] = centre;
                }
            }
            else
            {
                area[localX + localZ * width] = centre;
            }
        }
    }

    PIXEndNamedEvent();

    return area;
}
