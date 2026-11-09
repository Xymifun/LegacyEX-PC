#include "game/world/generation/IslandLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"

IslandLayer::IslandLayer(long long seed) : Layer(seed)
{
}

arrayWithLength<int> IslandLayer::getArea(int x, int z, int width, int height)
{
    PIXBeginNamedEvent(0.0f, "IslandLayer::getArea");

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            initRandom((long long)(x + j), (long long)(z + i));

            area[j + i * width] = nextRandom(10) == 0 ? 1 : 0;
        }
    }

    if (x > -width && x <= 0 && z > -height && z <= 0)
    {
        area[-x + -z * width] = 1;
    }

    PIXEndNamedEvent();

    return area;
}
