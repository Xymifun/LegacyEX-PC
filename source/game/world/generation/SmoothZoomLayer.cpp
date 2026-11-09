#include "game/world/generation/SmoothZoomLayer.hpp"
#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/System.hpp"

SmoothZoomLayer::SmoothZoomLayer(long long seed, boost::shared_ptr<Layer> parent) : Layer(seed)
{
    this->parent = parent;
}

boost::shared_ptr<Layer> SmoothZoomLayer::zoom(long long seed, boost::shared_ptr<Layer> parent, int count)
{
    boost::shared_ptr<Layer> layer = parent;
    for (int i = 0; i < count; ++i)
    {
        layer = boost::shared_ptr<Layer>(new SmoothZoomLayer(seed + i, layer));
    }
    return layer;
}

arrayWithLength<int> SmoothZoomLayer::getArea(int x, int z, int width, int height)
{
    int parentX = x >> 1;
    int parentZ = z >> 1;
    int parentWidth = (width >> 1) + 3;
    int parentHeight = (height >> 1) + 3;
    arrayWithLength<int> parentArea = parent->getArea(parentX, parentZ, parentWidth, parentHeight);

    PIXBeginNamedEvent(0.0f, "SmoothZoomLayer::getArea");

    arrayWithLength<int> zoomedArea = IntCache::allocate((parentWidth * 2) * (parentHeight * 2));
    int zoomedWidth = parentWidth << 1;
    for (int row = 0; row < parentHeight - 1; row++)
    {
        int zoomedRow = row << 1;
        int destIndex = zoomedRow * zoomedWidth;
        int upperLeft = parentArea[(row + 0) * parentWidth];
        int lowerLeft = parentArea[(row + 1) * parentWidth];
        for (int column = 0; column < parentWidth - 1; column++)
        {
            initRandom((column + parentX) << 1, (row + parentZ) << 1);
            int upperRight = parentArea[(column + 1) + (row + 0) * parentWidth];
            int lowerRight = parentArea[(column + 1) + (row + 1) * parentWidth];

            zoomedArea[destIndex] = upperLeft;
            zoomedArea[destIndex++ + zoomedWidth] = upperLeft + (lowerLeft - upperLeft) * nextRandom(256) / 256;
            zoomedArea[destIndex] = upperLeft + (upperRight - upperLeft) * nextRandom(256) / 256;
            int upper = upperLeft + (upperRight - upperLeft) * nextRandom(256) / 256;
            int lower = lowerLeft + (lowerRight - lowerLeft) * nextRandom(256) / 256;
            zoomedArea[destIndex++ + zoomedWidth] = upper + (lower - upper) * nextRandom(256) / 256;

            upperLeft = upperRight;
            lowerLeft = lowerRight;
        }
    }

    arrayWithLength<int> area = IntCache::allocate(width * height);
    for (int row = 0; row < height; row++)
    {
        System::arraycopy(zoomedArea, (row + (z & 1)) * (parentWidth << 1) + (x & 1), &area, row * width, width);
    }

    PIXEndNamedEvent();

    return area;
}
