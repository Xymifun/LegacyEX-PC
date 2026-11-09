#include "game/world/generation/AddMushroomIslandLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/world/biome/Biome.hpp"

AddMushroomIslandLayer::AddMushroomIslandLayer(long long seed, boost::shared_ptr<Layer> parentLayer) : Layer(seed)
{
    parent = parentLayer;
}

arrayWithLength<int> AddMushroomIslandLayer::getArea(int x, int z, int width, int height)
{
    int px = x - 1;
    int pz = z - 1;
    int parentWidth = width + 2;
    int parentHeight = height + 2;
    arrayWithLength<int> parentArea = parent->getArea(px, pz, parentWidth, parentHeight);

    PIXBeginNamedEvent(0.0f, "AddMushroomIslandLayer::getArea");

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

            initRandom((long long)(j + x), (long long)(i + z));

            if (isShallowOcean(centre) && isShallowOcean(northWest) && isShallowOcean(northEast)
                && isShallowOcean(southWest) && isShallowOcean(southEast) && nextRandom(100) == 0)
            {
                area[j + i * width] = Biome::MUSHROOM_ISLAND->id;
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
