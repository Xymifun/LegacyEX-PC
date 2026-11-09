#include "game/world/generation/AddSnowLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"

AddSnowLayer::AddSnowLayer(long long seed, boost::shared_ptr<Layer> parent) : Layer(seed)
{
    this->parent = parent;
}

arrayWithLength<int> AddSnowLayer::getArea(int x, int z, int width, int height)
{
    int parentX = x - 1;
    int parentZ = z - 1;
    int parentWidth = width + 2;
    int parentHeight = height + 2;
    arrayWithLength<int> parentArea = parent->getArea(parentX, parentZ, parentWidth, parentHeight);

    PIXBeginNamedEvent(0.0f, "AddSnowLayer::getArea");

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int centre = parentArea[j + 1 + (i + 1) * parentWidth];
            initRandom((long long)(j + x), (long long)(i + z));

            if (isShallowOcean(centre))
            {
                area[j + i * width] = centre;
            }
            else
            {
                int temperature = nextRandom(6);
                int snow;
                if (temperature == 0)
                {
                    snow = 4;
                }
                else
                {
                    snow = 1;
                    if (temperature <= 1)
                    {
                        snow = 3;
                    }
                }
                area[j + i * width] = snow;
            }
        }
    }

    PIXEndNamedEvent();

    return area;
}
