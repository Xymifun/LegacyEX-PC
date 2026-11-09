#ifndef GAME_WORLD_GENERATION_LAYER_HPP
#define GAME_WORLD_GENERATION_LAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/biome/BiomeSource.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class LevelData;
class LevelType;
class SuperflatConfig;

class Layer
{
public:
    static arrayWithLength<boost::shared_ptr<Layer> > getDefaultLayers(
        long long seed, LevelType* levelType, SuperflatConfig* levelTypeOptions, LevelData* levelData,
        BiomeSource::LayerOverrideSettings* overrideSettings);

    Layer(long long seed);
    virtual ~Layer() {}

    virtual void init(long long seed);
    virtual void initRandom(long long x, long long z);
    virtual arrayWithLength<int> getArea(int x, int z, int width, int height) = 0;
    virtual int modeOrRandom(int upperLeft, int upperRight, int lowerLeft, int lowerRight);

protected:
    static bool isSame(int firstBiomeId, int secondBiomeId);
    static bool isOcean(int biomeId);
    static bool isShallowOcean(int biomeId);

    int nextRandom(int bound);

    int random(int first, int second);
    int random(int first, int second, int third, int fourth);

    long long worldGenSeed;
    boost::shared_ptr<Layer> parent;
    long long chunkSeed;
    long long baseSeed;
};

#endif
