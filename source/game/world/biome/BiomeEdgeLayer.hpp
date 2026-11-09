#ifndef GAME_WORLD_BIOME_BIOMEEDGELAYER_HPP
#define GAME_WORLD_BIOME_BIOMEEDGELAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/generation/Layer.hpp"

class BiomeEdgeLayer : public Layer
{
public:
    BiomeEdgeLayer(long long seed, boost::shared_ptr<Layer> parent);
    virtual ~BiomeEdgeLayer();

    bool isValidTemperatureEdge(int firstBiomeId, int secondBiomeId);
    bool checkEdge(arrayWithLength<int> parentArea, arrayWithLength<int> area, int x, int z, int width, int centre,
                   int edge, int replacement);
    bool checkEdgeStrict(arrayWithLength<int> parentArea, arrayWithLength<int> area, int x, int z, int width,
                         int centre, int edge, int replacement);
    virtual arrayWithLength<int> getArea(int x, int z, int width, int height);
};

#endif
