#include "game/world/biome/RareBiomeSpotLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/core/PIXNamedEvents.hpp"

RareBiomeSpotLayer::RareBiomeSpotLayer(long long seed, boost::shared_ptr<Layer> parent) : Layer(seed)
{
    this->parent = parent;
}

RareBiomeSpotLayer::~RareBiomeSpotLayer()
{
}

arrayWithLength<int> RareBiomeSpotLayer::getArea(int x, int z, int width, int height)
{
    arrayWithLength<int> parentArea = parent->getArea(x - 1, z - 1, width + 2, height + 2);

    PIXBeginNamedEvent(0.0f, "RareBiomeSpotLayer::getArea");

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            initRandom((long long)(x + j), (long long)(z + i));

            int parentBiomeId = parentArea[j + 1 + (i + 1) * (width + 2)];

            if (nextRandom(57) == 0 && parentBiomeId == Biome::PLAINS->id)
            {
                area[j + i * width] = Biome::SUNFLOWER_PLAINS->id;
            }
            else
            {
                area[j + i * width] = parentBiomeId;
            }
        }
    }

    PIXEndNamedEvent();

    return area;
}
