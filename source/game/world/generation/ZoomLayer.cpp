#include "game/world/generation/ZoomLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/System.hpp"

ZoomLayer::ZoomLayer(long long seed, boost::shared_ptr<Layer> parent) : Layer(seed)
{
    this->parent = parent;
}

boost::shared_ptr<Layer> ZoomLayer::zoom(long long seed, boost::shared_ptr<Layer> parent, int count)
{
    boost::shared_ptr<Layer> layer = parent;

    for (int i = 0; i < count; i++)
    {
        layer = boost::shared_ptr<Layer>(new ZoomLayer(seed + i, layer));
    }

    return layer;
}

arrayWithLength<int> ZoomLayer::getArea(int x, int z, int width, int height)
{
    int parentX = x >> 1;
    int parentZ = z >> 1;
    int parentWidth = (width >> 1) + 2;
    int parentHeight = (height >> 1) + 2;

    arrayWithLength<int> parentArea = parent->getArea(parentX, parentZ, parentWidth, parentHeight);

    PIXBeginNamedEvent(0.0f, "ZoomLayer::getArea");

    int zoomedWidth = (parentWidth - 1) << 1;
    int zoomedHeight = (parentHeight - 1) << 1;

    arrayWithLength<int> zoomedArea = IntCache::allocate(zoomedWidth * zoomedHeight);

    for (int row = 0; row < parentHeight - 1; row++)
    {
        int destIndex = (row << 1) * zoomedWidth;

        int upperLeft = parentArea[(row + 0) * parentWidth];
        int lowerLeft = parentArea[(row + 1) * parentWidth];

        for (int column = 0; column < parentWidth - 1; column++)
        {
            initRandom((column + parentX) << 1, (row + parentZ) << 1);

            int upperRight = parentArea[(column + 1) + (row + 0) * parentWidth];
            int lowerRight = parentArea[(column + 1) + (row + 1) * parentWidth];

            zoomedArea[destIndex] = upperLeft;
            zoomedArea[destIndex++ + zoomedWidth] = random(upperLeft, lowerLeft);
            zoomedArea[destIndex] = random(upperLeft, upperRight);
            zoomedArea[destIndex++ + zoomedWidth] = modeOrRandom(upperLeft, upperRight, lowerLeft, lowerRight);

            upperLeft = upperRight;
            lowerLeft = lowerRight;
        }
    }

    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int row = 0; row < height; row++)
    {
        System::arraycopy(zoomedArea, (row + (z & 1)) * zoomedWidth + (x & 1), &area, row * width, width);
    }

    PIXEndNamedEvent();

    return area;
}
