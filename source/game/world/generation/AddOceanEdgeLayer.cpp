#include "game/world/generation/AddOceanEdgeLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/world/biome/Biome.hpp"

AddOceanEdgeLayer::AddOceanEdgeLayer(long long seed, boost::shared_ptr<Layer> parent, bool useOceanTemperatures)
    : Layer(seed)
{
    this->parent = parent;
    (void)useOceanTemperatures;
}

arrayWithLength<int> AddOceanEdgeLayer::getArea(int x, int z, int width, int height)
{
    int parentX = x - 1;
    int parentZ = z - 1;
    int parentWidth = width + 2;
    int parentHeight = height + 2;
    arrayWithLength<int> parentArea = parent->getArea(parentX, parentZ, parentWidth, parentHeight);

    PIXBeginNamedEvent(0.0f, "AddOceanEdgeLayer::getArea");

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int localZ = 0; localZ < height; localZ++)
    {
        for (int localX = 0; localX < width; localX++)
        {
            initRandom((long long)(localX + x), (long long)(localZ + z));

            int centre = parentArea[localX + 1 + (localZ + 1) * parentWidth];
            int north = parentArea[localX + 1 + (localZ + 1 - 1) * parentWidth];
            int east = parentArea[localX + 1 + 1 + (localZ + 1) * parentWidth];
            int west = parentArea[localX + 1 - 1 + (localZ + 1) * parentWidth];
            int south = parentArea[localX + 1 + (localZ + 1 + 1) * parentWidth];

            if (centre == Biome::eBiomeIDs_WarmOcean
                && (north == Biome::eBiomeIDs_FrozenOcean || east == Biome::eBiomeIDs_FrozenOcean
                    || west == Biome::eBiomeIDs_FrozenOcean || south == Biome::eBiomeIDs_FrozenOcean))
            {
                area[localX + localZ * width] = Biome::eBiomeIDs_Ocean;
            }
            else if (centre == Biome::eBiomeIDs_FrozenOcean
                     && (north == Biome::eBiomeIDs_WarmOcean || east == Biome::eBiomeIDs_WarmOcean
                         || west == Biome::eBiomeIDs_WarmOcean || south == Biome::eBiomeIDs_WarmOcean))
            {
                area[localX + localZ * width] = Biome::eBiomeIDs_Ocean;
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
