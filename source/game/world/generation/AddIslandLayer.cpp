#include "game/world/generation/AddIslandLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"

AddIslandLayer::AddIslandLayer(long long seed, boost::shared_ptr<Layer> parentLayer) : Layer(seed)
{
    parent = parentLayer;
}

arrayWithLength<int> AddIslandLayer::getArea(int x, int z, int width, int height)
{
    int parentX = x - 1;
    int parentZ = z - 1;
    int parentWidth = width + 2;
    int parentHeight = height + 2;
    arrayWithLength<int> parentArea = parent->getArea(parentX, parentZ, parentWidth, parentHeight);

    PIXBeginNamedEvent(0.0f, "AddIslandLayer::getArea");
    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int northWest = parentArea[j + 0 + (i + 0) * parentWidth];
            int northEast = parentArea[j + 2 + (i + 0) * parentWidth];
            int southWest = parentArea[j + 0 + (i + 2) * parentWidth];
            int southEast = parentArea[j + 2 + (i + 2) * parentWidth];
            int centre = parentArea[j + 1 + (i + 1) * parentWidth];

            initRandom((long long)(j + x), (long long)(i + z));

            if (isShallowOcean(centre)
                && (!isShallowOcean(northWest) || !isShallowOcean(northEast) || !isShallowOcean(southWest)
                    || !isShallowOcean(southEast)))
            {
                int nonOceanCount = 1;
                int selected = 1;

                if (!isShallowOcean(northWest) && nextRandom(nonOceanCount++) == 0)
                {
                    selected = northWest;
                }
                if (!isShallowOcean(northEast) && nextRandom(nonOceanCount++) == 0)
                {
                    selected = northEast;
                }
                if (!isShallowOcean(southWest) && nextRandom(nonOceanCount++) == 0)
                {
                    selected = southWest;
                }
                if (!isShallowOcean(southEast) && nextRandom(nonOceanCount++) == 0)
                {
                    selected = southEast;
                }

                if (nextRandom(3) == 0)
                {
                    area[j + i * width] = selected;
                }
                else if (selected == 4)
                {
                    area[j + i * width] = 4;
                }
                else
                {
                    area[j + i * width] = 0;
                }
            }
            else if (!isShallowOcean(centre)
                     && (isShallowOcean(northWest) || isShallowOcean(northEast) || isShallowOcean(southWest)
                         || isShallowOcean(southEast)))
            {
                if (nextRandom(5) == 0)
                {
                    if (isShallowOcean(northWest))
                    {
                        if (centre == 4)
                        {
                            area[j + i * width] = centre;
                        }
                        else
                        {
                            area[j + i * width] = northWest;
                        }
                    }
                    else if (isShallowOcean(northEast))
                    {
                        if (centre == 4)
                        {
                            area[j + i * width] = centre;
                        }
                        else
                        {
                            area[j + i * width] = northEast;
                        }
                    }
                    else if (isShallowOcean(southWest))
                    {
                        if (centre == 4)
                        {
                            area[j + i * width] = centre;
                        }
                        else
                        {
                            area[j + i * width] = southWest;
                        }
                    }
                    else if (isShallowOcean(southEast))
                    {
                        if (centre == 4)
                        {
                            area[j + i * width] = centre;
                        }
                        else
                        {
                            area[j + i * width] = southEast;
                        }
                    }
                }
                else
                {
                    area[j + i * width] = centre;
                }
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
