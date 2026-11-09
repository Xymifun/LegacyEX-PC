#include "game/world/generation/GrowMushroomIslandLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"

GrowMushroomIslandLayer::GrowMushroomIslandLayer(long long seed, boost::shared_ptr<Layer> parent) : Layer(seed)
{
    this->parent = parent;
}

arrayWithLength<int> GrowMushroomIslandLayer::getArea(int x, int z, int width, int height)
{
    int px = x - 1;
    int pz = z - 1;
    int parentWidth = width + 2;
    int parentHeight = height + 2;
    arrayWithLength<int> parentArea = parent->getArea(px, pz, parentWidth, parentHeight);

    PIXBeginNamedEvent(0.0f, "GrowMushroomIslandLayer::getArea");

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int northWest = parentArea[j + i * parentWidth];
            int northEast = parentArea[j + 2 + i * parentWidth];
            int southWest = parentArea[j + (i + 2) * parentWidth];
            int southEast = parentArea[j + 2 + (i + 2) * parentWidth];
            int centre = parentArea[j + 1 + (i + 1) * parentWidth];

            if (northWest == 14 || northEast == 14 || southWest == 14 || southEast == 14)
            {
                area[j + i * width] = 14;
            }
            else
            {
                area[j + i * width] = centre;
            }
        }
    }

    PIXEndNamedEvent();

    return area;
}
