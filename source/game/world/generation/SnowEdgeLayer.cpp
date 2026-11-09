#include "game/world/generation/SnowEdgeLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"

SnowEdgeLayer::SnowEdgeLayer(long long seed, boost::shared_ptr<Layer> parent) : Layer(seed)
{
    this->parent = parent;
}

SnowEdgeLayer::~SnowEdgeLayer()
{
}

arrayWithLength<int> SnowEdgeLayer::getArea(int x, int z, int width, int height)
{
    arrayWithLength<int> parentArea = parent->getArea(x - 1, z - 1, width + 2, height + 2);

    PIXBeginNamedEvent(0.0f, "SnowEdgeLayer::getArea");

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int localZ = 0; localZ < height; localZ++)
    {
        for (int localX = 0; localX < width; localX++)
        {
            initRandom((long long)(localX + x), (long long)(localZ + z));

            int centre = parentArea[localX + 1 + (localZ + 1) * (width + 2)];

            if (centre == 1)
            {
                int north = parentArea[localX + 1 + (localZ + 1 - 1) * (width + 2)];
                int east = parentArea[localX + 1 + 1 + (localZ + 1) * (width + 2)];
                int west = parentArea[localX + 1 - 1 + (localZ + 1) * (width + 2)];
                int south = parentArea[localX + 1 + (localZ + 1 + 1) * (width + 2)];

                if (north == 4 || east == 4 || west == 4 || south == 4)
                {
                    area[localX + localZ * width] = 3;
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
