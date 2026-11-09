#include "game/world/generation/SwampRiversLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/world/biome/Biome.hpp"

SwampRiversLayer::SwampRiversLayer(long long seed, boost::shared_ptr<Layer> parent) : Layer(seed)
{
    this->parent = parent;
}

arrayWithLength<int> SwampRiversLayer::getArea(int x, int z, int width, int height)
{
    arrayWithLength<int> parentArea = parent->getArea(x - 1, z - 1, width + 2, height + 2);

    PIXBeginNamedEvent(0.0f, "SwampRiversLayer::getArea");
    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            initRandom((long long)(j + x), (long long)(i + z));

            int centre = parentArea[j + 1 + (i + 1) * (width + 2)];

            if (centre == Biome::SWAMPLAND->id && nextRandom(6) == 0)
            {
                area[j + i * width] = Biome::RIVER->id;
            }
            else if ((centre == Biome::JUNGLE->id || centre == Biome::JUNGLE_HILLS->id) && nextRandom(8) == 0)
            {
                area[j + i * width] = Biome::RIVER->id;
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
