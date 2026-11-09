#include "game/world/generation/RiverInitLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"

RiverInitLayer::RiverInitLayer(long long seed, boost::shared_ptr<Layer> parent) : Layer(seed)
{
    this->parent = parent;
}

arrayWithLength<int> RiverInitLayer::getArea(int x, int z, int width, int height)
{
    arrayWithLength<int> parentArea = parent->getArea(x, z, width, height);

    PIXBeginNamedEvent(0.0f, "RiverInitLayer::getArea");

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            initRandom((long long)(j + x), (long long)(i + z));

            int centre = parentArea.data[j + i * width];

            if (isShallowOcean(centre))
            {
                area.data[j + i * width] = centre;
            }
            else
            {
                area.data[j + i * width] = nextRandom(299999) + 2;
            }
        }
    }

    PIXEndNamedEvent();

    return area;
}
