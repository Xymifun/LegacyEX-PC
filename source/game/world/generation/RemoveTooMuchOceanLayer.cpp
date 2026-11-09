#include "game/world/generation/RemoveTooMuchOceanLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"

RemoveTooMuchOceanLayer::RemoveTooMuchOceanLayer(long long seed, boost::shared_ptr<Layer> parent) : Layer(seed)
{
    this->parent = parent;
}

RemoveTooMuchOceanLayer::~RemoveTooMuchOceanLayer()
{
}

arrayWithLength<int> RemoveTooMuchOceanLayer::getArea(int x, int z, int width, int height)
{
    int parentX = x - 1;
    int parentZ = z - 1;
    int parentWidth = width + 2;
    int parentHeight = height + 2;

    arrayWithLength<int> parentArea = parent->getArea(parentX, parentZ, parentWidth, parentHeight);

    PIXBeginNamedEvent(0.0f, "RemoveTooMuchOceanLayer::getArea");

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int north = parentArea[j + 1 + (i + 1 - 1) * (width + 2)];
            int east = parentArea[j + 1 + 1 + (i + 1) * (width + 2)];
            int west = parentArea[j + 1 - 1 + (i + 1) * (width + 2)];
            int south = parentArea[j + 1 + (i + 1 + 1) * (width + 2)];
            int centre = parentArea[j + 1 + (i + 1) * parentWidth];

            area[j + i * width] = centre;

            initRandom((long long)(j + x), (long long)(i + z));

            if (isShallowOcean(centre) && isShallowOcean(north) && isShallowOcean(east) && isShallowOcean(west)
                && isShallowOcean(south) && nextRandom(2) == 0)
            {
                area[j + i * width] = 1;
            }
        }
    }

    PIXEndNamedEvent();

    return area;
}
