#ifndef GAME_WORLD_BIOME_BIOMEINITLAYER_HPP
#define GAME_WORLD_BIOME_BIOMEINITLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/Layer.hpp"

class Biome;
class CustomizableSourceSettings;
class LevelType;
class SuperflatConfig;

class BiomeInitLayer : public Layer
{
public:
    BiomeInitLayer(long long seed, boost::shared_ptr<Layer> parent, LevelType* levelType,
                   SuperflatConfig* levelTypeOptions);
    virtual ~BiomeInitLayer();

    virtual arrayWithLength<int> getArea(int x, int z, int width, int height);

protected:
    arrayWithLength<Biome*> warmBiomes;
    arrayWithLength<Biome*> mediumBiomes;
    arrayWithLength<Biome*> coldBiomes;
    arrayWithLength<Biome*> iceBiomes;
    CustomizableSourceSettings* settings;
};

#endif
