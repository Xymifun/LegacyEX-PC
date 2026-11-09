#include "game/world/generation/FlatLayer.hpp"

#include "game/core/IntCache.hpp"

FlatLayer::FlatLayer(int biome) : Layer(0LL), biome(biome)
{
}

arrayWithLength<int> FlatLayer::getArea(int x, int z, int width, int height)
{
    arrayWithLength<int> area = IntCache::allocate(width * height);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            area.data[j + i * width] = biome;
        }
    }

    return area;
}
