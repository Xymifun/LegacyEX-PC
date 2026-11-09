#include "game/world/generation/RiverMixerLayer.hpp"

#include "game/core/IntCache.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/world/biome/Biome.hpp"

RiverMixerLayer::RiverMixerLayer(long long seed, boost::shared_ptr<Layer> biomeLayer,
                                 boost::shared_ptr<Layer> riverLayer)
    : Layer(seed)
{
    this->biomeLayer = biomeLayer;
    this->riverLayer = riverLayer;
}

void RiverMixerLayer::init(long long seed)
{
    biomeLayer->init(seed);
    riverLayer->init(seed);
    Layer::init(seed);
}

arrayWithLength<int> RiverMixerLayer::getArea(int x, int z, int width, int height)
{
    arrayWithLength<int> biomes = biomeLayer->getArea(x, z, width, height);
    arrayWithLength<int> rivers = riverLayer->getArea(x, z, width, height);

    PIXBeginNamedEvent(0.0f, "RiverMixerLayer::getArea");

    arrayWithLength<int> result = IntCache::allocate(width * height);

    for (int i = 0; i < width * height; i++)
    {
        if (!isOcean(biomes.data[i]))
        {
            if (rivers.data[i] == Biome::RIVER->id)
            {
                if (biomes.data[i] == Biome::ICE_PLAINS->id)
                {
                    result.data[i] = Biome::FROZEN_RIVER->id;
                }
                else if (biomes.data[i] == Biome::MUSHROOM_ISLAND->id
                         || biomes.data[i] == Biome::MUSHROOM_ISLAND_SHORE->id)
                {
                    result.data[i] = Biome::MUSHROOM_ISLAND->id;
                }
                else
                {
                    result.data[i] = rivers.data[i] & 255;
                }
            }
            else
            {
                result.data[i] = biomes.data[i];
            }
        }
        else
        {
            result.data[i] = biomes.data[i];
        }
    }

    PIXEndNamedEvent();
    return result;
}
