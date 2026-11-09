#include "game/world/generation/Layer.hpp"

#include "game/core/CustomizableSourceSettings.hpp"
#include "game/core/VoronoiZoom.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/BiomeCacheLayer.hpp"
#include "game/world/biome/BiomeEdgeLayer.hpp"
#include "game/world/biome/BiomeInitLayer.hpp"
#include "game/world/biome/RareBiomeSpotLayer.hpp"
#include "game/world/generation/AddDeepOceanLayer.hpp"
#include "game/world/generation/AddEdgeLayer.hpp"
#include "game/world/generation/AddIslandLayer.hpp"
#include "game/world/generation/AddMushroomIslandLayer.hpp"
#include "game/world/generation/AddOceanEdgeLayer.hpp"
#include "game/world/generation/AddOceanTemperatureLayer.hpp"
#include "game/world/generation/AddSnowLayer.hpp"
#include "game/world/generation/FuzzyZoomLayer.hpp"
#include "game/world/generation/GrowMushroomIslandLayer.hpp"
#include "game/world/generation/IslandLayer.hpp"
#include "game/world/generation/OceanMixerLayer.hpp"
#include "game/world/generation/RegionHillsLayer.hpp"
#include "game/world/generation/RemoveTooMuchOceanLayer.hpp"
#include "game/world/generation/RiverInitLayer.hpp"
#include "game/world/generation/RiverLayer.hpp"
#include "game/world/generation/RiverMixerLayer.hpp"
#include "game/world/generation/ShoreLayer.hpp"
#include "game/world/generation/SmoothLayer.hpp"
#include "game/world/generation/ZoomLayer.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/LevelType.hpp"

Layer::Layer(long long seed)
{
    parent = boost::shared_ptr<Layer>();
    worldGenSeed = 0;

    baseSeed = seed;
    baseSeed *= baseSeed * 6364136223846793005LL + 1442695040888963407LL;
    baseSeed += seed;
    baseSeed *= baseSeed * 6364136223846793005LL + 1442695040888963407LL;
    baseSeed += seed;
    baseSeed *= baseSeed * 6364136223846793005LL + 1442695040888963407LL;
    baseSeed += seed;
}

arrayWithLength<boost::shared_ptr<Layer> > Layer::getDefaultLayers(long long seed, LevelType* levelType,
                                                                   SuperflatConfig* levelTypeOptions,
                                                                   LevelData* levelData,
                                                                   BiomeSource::LayerOverrideSettings* overrideSettings)
{
    unsigned char biomeScale = 0;
    int biomeCentreXChunk = 0;
    int biomeCentreZChunk = 0;

    if (overrideSettings != NULL)
    {
        biomeScale = overrideSettings->GetBiomeScale();
        biomeCentreXChunk = overrideSettings->GetCentreXChunk();
        biomeCentreZChunk = overrideSettings->GetCentreZChunk();
    }
    else if (levelData != NULL)
    {
        biomeScale = levelData->getBiomeScale();
        biomeCentreXChunk = levelData->getBiomeCentreXChunk();
        biomeCentreZChunk = levelData->getBiomeCentreZChunk();
    }

    boost::shared_ptr<Layer> layer(new IslandLayer(1LL));

    layer = boost::shared_ptr<Layer>(new FuzzyZoomLayer(2000LL, layer));
    layer = boost::shared_ptr<Layer>(new AddIslandLayer(1LL, layer));
    layer = boost::shared_ptr<Layer>(new ZoomLayer(2001LL, layer));
    layer = boost::shared_ptr<Layer>(new AddIslandLayer(2LL, layer));
    layer = boost::shared_ptr<Layer>(new AddIslandLayer(50LL, layer));
    layer = boost::shared_ptr<Layer>(new AddIslandLayer(70LL, layer));
    layer = boost::shared_ptr<Layer>(new RemoveTooMuchOceanLayer(2LL, layer));

    boost::shared_ptr<Layer> oceanTemperatureLayer = layer;

    oceanTemperatureLayer = boost::shared_ptr<Layer>(new AddOceanTemperatureLayer(2LL, oceanTemperatureLayer));
    oceanTemperatureLayer = boost::shared_ptr<Layer>(new AddOceanEdgeLayer(2LL, oceanTemperatureLayer, true));
    oceanTemperatureLayer = ZoomLayer::zoom(2001LL, oceanTemperatureLayer, 6);

    layer = boost::shared_ptr<Layer>(new AddSnowLayer(2LL, layer));
    layer = boost::shared_ptr<Layer>(new AddIslandLayer(3LL, layer));
    layer = boost::shared_ptr<Layer>(new AddEdgeLayer(2LL, layer, AddEdgeLayer::COOL_WARM));
    layer = boost::shared_ptr<Layer>(new AddEdgeLayer(2LL, layer, AddEdgeLayer::HEAT_ICE));
    layer = boost::shared_ptr<Layer>(new AddEdgeLayer(3LL, layer, AddEdgeLayer::SPECIAL));
    layer = boost::shared_ptr<Layer>(new ZoomLayer(2002LL, layer));
    layer = boost::shared_ptr<Layer>(new ZoomLayer(2003LL, layer));
    layer = boost::shared_ptr<Layer>(new AddIslandLayer(4LL, layer));

    if (biomeScale != 0)
    {
        layer = boost::shared_ptr<Layer>(new AddMushroomIslandLayer(5LL, layer));
    }

    layer = boost::shared_ptr<Layer>(new AddDeepOceanLayer(4LL, layer));
    layer = ZoomLayer::zoom(1000LL, layer, 0);

    int biomeSize = 4;
    int riverSize = 4;

    if (levelType == LevelType::CUSTOMIZED && levelTypeOptions != NULL)
    {
        CustomizableSourceSettings::Builder* builder = CustomizableSourceSettings::Builder::fromString(
            levelTypeOptions);
        CustomizableSourceSettings* settings = builder->build();

        biomeSize = settings->biomeSize;
        riverSize = settings->riverSize;

        delete builder;
    }

    if (levelType == LevelType::LARGE_BIOMES)
    {
        biomeSize = 6;
    }

    boost::shared_ptr<Layer> riverLayer = layer;

    riverLayer = ZoomLayer::zoom(1000LL, riverLayer, 0);
    riverLayer = boost::shared_ptr<Layer>(new RiverInitLayer(100LL, riverLayer));

    boost::shared_ptr<Layer> biomeLayer = layer;

    biomeLayer = boost::shared_ptr<Layer>(new BiomeInitLayer(200LL, biomeLayer, levelType, levelTypeOptions));
    biomeLayer = ZoomLayer::zoom(1000LL, biomeLayer, biomeScale);
    biomeLayer = boost::shared_ptr<Layer>(new BiomeEdgeLayer(1000LL, biomeLayer));

    boost::shared_ptr<Layer> hillsLayer = riverLayer;

    hillsLayer = ZoomLayer::zoom(1000LL, hillsLayer, 2);

    biomeLayer = boost::shared_ptr<Layer>(new RegionHillsLayer(1000LL, biomeLayer, hillsLayer));

    riverLayer = ZoomLayer::zoom(1000LL, riverLayer, 2);
    riverLayer = ZoomLayer::zoom(1000LL, riverLayer, riverSize);
    riverLayer = boost::shared_ptr<Layer>(new RiverLayer(1LL, riverLayer));
    riverLayer = boost::shared_ptr<Layer>(new SmoothLayer(1000LL, riverLayer));

    biomeLayer = boost::shared_ptr<Layer>(new RareBiomeSpotLayer(1001LL, biomeLayer));

    for (int zoomStep = 0; zoomStep < biomeSize; zoomStep++)
    {
        biomeLayer = boost::shared_ptr<Layer>(new ZoomLayer((long long)(1000 + zoomStep), biomeLayer));

        if (zoomStep == 0)
        {
            biomeLayer = boost::shared_ptr<Layer>(new AddIslandLayer(3LL, biomeLayer));
        }

        if (zoomStep == 0 && biomeScale == 0)
        {
            biomeLayer = boost::shared_ptr<Layer>(new AddMushroomIslandLayer(5LL, biomeLayer));
        }

        if (zoomStep == 1 || biomeSize == 1)
        {
            if (biomeScale == 0)
            {
                biomeLayer = boost::shared_ptr<Layer>(new GrowMushroomIslandLayer(5LL, biomeLayer));
            }

            biomeLayer = boost::shared_ptr<Layer>(new ShoreLayer(1000LL, biomeLayer));
        }
    }

    biomeLayer = boost::shared_ptr<Layer>(new SmoothLayer(1000LL, biomeLayer));
    biomeLayer = boost::shared_ptr<Layer>(new RiverMixerLayer(100LL, biomeLayer, riverLayer));
    biomeLayer = boost::shared_ptr<Layer>(new OceanMixerLayer(100LL, biomeLayer, oceanTemperatureLayer));

    if (levelData != NULL)
    {
        biomeLayer = boost::shared_ptr<Layer>(new BiomeCacheLayer(1, biomeLayer, levelData->getXZSize(), biomeScale,
                                                                  biomeCentreXChunk, biomeCentreZChunk));
    }

    boost::shared_ptr<Layer> resultBiomeLayer = biomeLayer;
    boost::shared_ptr<Layer> voronoiZoom(new VoronoiZoom(10LL, biomeLayer));

    biomeLayer->init(seed);
    voronoiZoom->init(seed);

    arrayWithLength<boost::shared_ptr<Layer> > layers(3, true);

    layers.data[0] = biomeLayer;
    layers.data[1] = voronoiZoom;
    layers.data[2] = resultBiomeLayer;

    return layers;
}

void Layer::init(long long seed)
{
    worldGenSeed = seed;

    if (parent)
    {
        parent->init(seed);
    }

    worldGenSeed *= worldGenSeed * 6364136223846793005LL + 1442695040888963407LL;
    worldGenSeed += baseSeed;
    worldGenSeed *= worldGenSeed * 6364136223846793005LL + 1442695040888963407LL;
    worldGenSeed += baseSeed;
    worldGenSeed *= worldGenSeed * 6364136223846793005LL + 1442695040888963407LL;
    worldGenSeed += baseSeed;
}

void Layer::initRandom(long long x, long long z)
{
    chunkSeed = worldGenSeed;

    chunkSeed *= chunkSeed * 6364136223846793005LL + 1442695040888963407LL;
    chunkSeed += x;
    chunkSeed *= chunkSeed * 6364136223846793005LL + 1442695040888963407LL;
    chunkSeed += z;
    chunkSeed *= chunkSeed * 6364136223846793005LL + 1442695040888963407LL;
    chunkSeed += x;
    chunkSeed *= chunkSeed * 6364136223846793005LL + 1442695040888963407LL;
    chunkSeed += z;
}

int Layer::nextRandom(int bound)
{
    int value = (int)((chunkSeed >> 24) % (long long)bound);

    if (value < 0)
    {
        value += bound;
    }

    chunkSeed *= chunkSeed * 6364136223846793005LL + 1442695040888963407LL;
    chunkSeed += worldGenSeed;

    return value;
}

bool Layer::isSame(int firstBiomeId, int secondBiomeId)
{
    if (firstBiomeId == secondBiomeId)
    {
        return true;
    }

    Biome* firstBiome = Biome::getBiome(firstBiomeId);
    Biome* secondBiome = Biome::getBiome(secondBiomeId);

    if (firstBiome == NULL || secondBiome == NULL)
    {
        return false;
    }

    if (firstBiome == Biome::MESA_ROCK || firstBiome == Biome::MESA_CLEAR_ROCK)
    {
        return secondBiome == Biome::MESA_ROCK || secondBiome == Biome::MESA_CLEAR_ROCK;
    }

    if (firstBiome == secondBiome)
    {
        return true;
    }

    return firstBiome->getBaseClass() == secondBiome->getBaseClass();
}

bool Layer::isOcean(int biomeId)
{
    return biomeId == 0 || biomeId == 24 || biomeId == 40 || (biomeId >= 42 && biomeId <= 47);
}

bool Layer::isShallowOcean(int biomeId)
{
    if (biomeId < 42)
    {
        return biomeId == 0 || biomeId == 40;
    }

    return biomeId == 42 || biomeId == 44 || biomeId == 46;
}

int Layer::random(int first, int second)
{
    return nextRandom(2) == 0 ? first : second;
}

int Layer::random(int first, int second, int third, int fourth)
{
    int selection = nextRandom(4);

    if (selection == 0)
    {
        return first;
    }

    if (selection == 1)
    {
        return second;
    }

    if (selection == 2)
    {
        return third;
    }

    if (selection == 3)
    {
        return fourth;
    }

    return first;
}

int Layer::modeOrRandom(int upperLeft, int upperRight, int lowerLeft, int lowerRight)
{
    if (upperRight == lowerLeft && lowerLeft == lowerRight)
    {
        return upperRight;
    }

    if (upperLeft == upperRight && upperLeft == lowerLeft)
    {
        return upperLeft;
    }

    if (upperLeft == upperRight && upperLeft == lowerRight)
    {
        return upperLeft;
    }

    if (upperLeft == lowerLeft && upperLeft == lowerRight)
    {
        return upperLeft;
    }

    if (upperLeft == upperRight && lowerLeft != lowerRight)
    {
        return upperLeft;
    }

    if (upperLeft == lowerLeft && upperRight != lowerRight)
    {
        return upperLeft;
    }

    if (upperLeft == lowerRight && upperRight != lowerLeft)
    {
        return upperLeft;
    }

    if (upperRight == lowerLeft && upperLeft != lowerRight)
    {
        return upperRight;
    }

    if (upperRight == lowerRight && upperLeft != lowerLeft)
    {
        return upperRight;
    }

    if (lowerLeft == lowerRight && upperLeft != upperRight)
    {
        return lowerLeft;
    }

    return random(upperLeft, upperRight, lowerLeft, lowerRight);
}
