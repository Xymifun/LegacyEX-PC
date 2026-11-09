#include "game/world/biome/BiomeCacheLayer.hpp"

#include <algorithm>
#include <cmath>

#include "game/core/IntCache.hpp"

BiomeCacheLayer::BiomeCacheLayer(int seed, boost::shared_ptr<Layer> parent, unsigned int xzSize,
                                 unsigned char biomeScale, int centreXChunk, int centreZChunk)
    : Layer(seed)
{
    this->parent = parent;

    biomeCache = arrayWithLength<unsigned char>();

    cacheSize = (xzSize & 0x0fffffff) << 2;

    int scale = (int)pow(2.0, (double)biomeScale);

    cacheX = (centreXChunk * scale * 16 >> 2) - (cacheSize / 2);
    cacheZ = (centreZChunk * scale * 16 >> 2) - (cacheSize / 2);
}

BiomeCacheLayer::~BiomeCacheLayer()
{
    delete[] biomeCache.data;
}

arrayWithLength<int> BiomeCacheLayer::getArea(int x, int z, int width, int height)
{
    if (biomeCache.data == NULL)
    {
        biomeCache = arrayWithLength<unsigned char>(cacheSize * cacheSize, true);

#ifdef MINECRAFT_PC
        static const int WHOLE_AREA_LIMIT = 1280;
        static const int TILE_SIZE = 512;
        if (cacheSize > WHOLE_AREA_LIMIT)
        {
            for (int tileZ = 0; tileZ < cacheSize; tileZ += TILE_SIZE)
            {
                for (int tileX = 0; tileX < cacheSize; tileX += TILE_SIZE)
                {
                    const int tileWidth = std::min(TILE_SIZE, cacheSize - tileX);
                    const int tileHeight = std::min(TILE_SIZE, cacheSize - tileZ);
                    const IntCache::PcMark mark = IntCache::pcMark();
                    arrayWithLength<int> tile = parent->getArea(cacheX + tileX, cacheZ + tileZ, tileWidth, tileHeight);
                    for (int row = 0; row < tileHeight; ++row)
                    {
                        for (int column = 0; column < tileWidth; ++column)
                        {
                            biomeCache[(tileX + column) + (tileZ + row) * cacheSize] =
                                (unsigned char)tile[column + row * tileWidth];
                        }
                    }
                    IntCache::pcReleaseSince(mark);
                }
            }
        }
        else
#endif
        {
            arrayWithLength<int> parentArea = parent->getArea(cacheX, cacheZ, cacheSize, cacheSize);

            for (unsigned int i = 0; i < biomeCache.length; i++)
            {
                biomeCache[i] = (unsigned char)parentArea[i];
            }
        }
    }

    arrayWithLength<int> area = IntCache::allocate(width * height);

    x -= cacheX;
    z -= cacheZ;

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int cacheColumn = std::max(0, std::min(cacheSize - 1, x + j));
            int cacheRow = std::max(0, std::min(cacheSize - 1, z + i));

            area[j + i * width] = biomeCache[cacheColumn + cacheRow * cacheSize];
        }
    }

    return area;
}
