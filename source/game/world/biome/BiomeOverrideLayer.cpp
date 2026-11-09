#include "game/world/biome/BiomeOverrideLayer.hpp"

#include "game/core/IntCache.hpp"

BiomeOverrideLayer::BiomeOverrideLayer(int size)
    : Layer(0),
      overrideWidth(size),
      overrideHeight(size),
      overrideBiomes(NULL)
{
}

arrayWithLength<int> BiomeOverrideLayer::getArea(int x, int z, int width, int height)
{
    arrayWithLength<int> area = IntCache::allocate(width * height);

    int originX = x + overrideWidth / 2;
    int originZ = z + overrideHeight / 2;

    if (originX < 0)
    {
        originX = 0;
    }

    if (originX >= (int)overrideWidth)
    {
        originX = overrideWidth - 1;
    }

    if (originZ < 0)
    {
        originZ = 0;
    }

    if (originZ >= (int)overrideHeight)
    {
        originZ = overrideHeight - 1;
    }

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int sampleX = originX + j;
            int sampleZ = originZ + i;

            if (sampleX >= (int)overrideWidth)
            {
                sampleX = overrideWidth - 1;
            }

            if (sampleZ >= (int)overrideHeight)
            {
                sampleZ = overrideHeight - 1;
            }

            int index = sampleX + sampleZ * overrideWidth;
            unsigned char value = overrideBiomes[index];
            area[j + i * width] = value;
        }
    }

    return area;
}
