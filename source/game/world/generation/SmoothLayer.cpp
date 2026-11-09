#include "game/world/generation/SmoothLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"

SmoothLayer::SmoothLayer(long long seed, boost::shared_ptr<Layer> parent) : Layer(seed)
{
    this->parent = parent;
}

arrayWithLength<int> SmoothLayer::getArea(int x, int z, int width, int height)
{
    int px = x - 1;
    int pz = z - 1;
    int parentWidth = width + 2;
    int parentHeight = height + 2;

    arrayWithLength<int> parentArea = parent->getArea(px, pz, parentWidth, parentHeight);

    PIXBeginNamedEvent(0.0f, "SmoothLayer::getArea");

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int west = parentArea[j + (i + 1) * parentWidth];
            int east = parentArea[j + 2 + (i + 1) * parentWidth];
            int north = parentArea[j + 1 + i * parentWidth];
            int south = parentArea[j + 1 + (i + 2) * parentWidth];
            int centre = parentArea[j + 1 + (i + 1) * parentWidth];

            if (west == east && north == south)
            {
                initRandom(j + x, i + z);

                if (nextRandom(2) == 0)
                {
                    centre = west;
                }
                else
                {
                    centre = north;
                }
            }
            else
            {
                if (west == east)
                {
                    centre = west;
                }

                if (north == south)
                {
                    centre = north;
                }
            }

            area[j + i * width] = centre;
        }
    }

    PIXEndNamedEvent();

    return area;
}
