#ifndef GAME_WORLD_BIOME_BIOMECACHELAYER_HPP
#define GAME_WORLD_BIOME_BIOMECACHELAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/Layer.hpp"

class BiomeCacheLayer : public Layer
{
public:
    BiomeCacheLayer(int seed, boost::shared_ptr<Layer> parent, unsigned int xzSize, unsigned char biomeScale,
                    int centreXChunk, int centreZChunk);
    virtual ~BiomeCacheLayer();

    virtual arrayWithLength<int> getArea(int x, int z, int width, int height);

private:
    int cacheX;
    int cacheZ;
    int cacheSize;
    arrayWithLength<unsigned char> biomeCache;
};

#endif
