#ifndef GAME_WORLD_GENERATION_ADDEDGELAYER_HPP
#define GAME_WORLD_GENERATION_ADDEDGELAYER_HPP

#include <boost/shared_ptr.hpp>
#include "game/world/generation/Layer.hpp"

class AddEdgeLayer : public Layer
{
public:
    enum Mode
    {
        COOL_WARM,
        HEAT_ICE,
        SPECIAL
    };

    AddEdgeLayer(long long seed, boost::shared_ptr<Layer> parentLayer, Mode mode);
    virtual ~AddEdgeLayer();
    virtual arrayWithLength<int> getArea(int x, int z, int width, int height);

private:
    arrayWithLength<int> coolWarm(int x, int z, int width, int height);
    arrayWithLength<int> heatIce(int x, int z, int width, int height);
    arrayWithLength<int> introduceSpecial(int x, int z, int width, int height);
    Mode mMode;
};

#endif
