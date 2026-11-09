#include "game/world/generation/RiverLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/core/PIXNamedEvents.hpp"

RiverLayer::RiverLayer(long long seed, boost::shared_ptr<Layer> parent) : Layer(seed)
{
    this->parent = parent;
}

int RiverLayer::riverFilter(int biomeId)
{
    if (biomeId >= 2)
    {
        return (biomeId & 1) + 2;
    }
    return biomeId;
}

arrayWithLength<int> RiverLayer::getArea(int x, int z, int width, int height)
{
    int parentX = x - 1;
    int parentZ = z - 1;
    int parentWidth = width + 2;
    int parentHeight = height + 2;

    arrayWithLength<int> parentArea = parent->getArea(parentX, parentZ, parentWidth, parentHeight);

    PIXBeginNamedEvent(0.0f, "RiverLayer::getArea");

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int left = riverFilter(parentArea[j + 0 + (i + 1) * parentWidth]);
            int right = riverFilter(parentArea[j + 2 + (i + 1) * parentWidth]);
            int upper = riverFilter(parentArea[j + 1 + (i + 0) * parentWidth]);
            int lower = riverFilter(parentArea[j + 1 + (i + 2) * parentWidth]);
            int centre = riverFilter(parentArea[j + 1 + (i + 1) * parentWidth]);

            if (centre != left || centre != upper || centre != right || centre != lower)
            {
                area[j + i * width] = Biome::RIVER->id;
            }
            else
            {
                area[j + i * width] = -1;
            }
        }
    }

    PIXEndNamedEvent();

    return area;
}
