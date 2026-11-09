#ifndef GAME_WORLD_GENERATION_RANDOMSCATTEREDLARGEFEATURE_HPP
#define GAME_WORLD_GENERATION_RANDOMSCATTEREDLARGEFEATURE_HPP

#include <vector>
#include <string>

#include "game/world/generation/StructureFeature.hpp"
#include "game/world/generation/StructureStart.hpp"
#include "game/world/biome/Biome.hpp"

class BlockPos;
class Level;
class Random;
class SuperflatConfig;

class RandomScatteredLargeFeature : public StructureFeature
{
public:
    class ScatteredFeatureStart : public StructureStart
    {
    public:
        ScatteredFeatureStart(Level* level, Random& random, int chunkX, int chunkZ);
        ScatteredFeatureStart();
        virtual EStructureStart GetType();
        static StructureStart* Create();
    };

    RandomScatteredLargeFeature();
    RandomScatteredLargeFeature(SuperflatConfig* config);
    virtual ~RandomScatteredLargeFeature();

    static void staticCtor();
    void _init();
    virtual std::wstring getFeatureName();
    virtual BlockPos* getNearestGeneratedFeature(Level* level, const BlockPos& pos, bool skipExistingChunks);
    virtual bool isFeatureChunk(int chunkX, int chunkZ, bool flatWorld);
    virtual StructureStart* createStructureStart(int chunkX, int chunkZ);
    bool isSwamphut(const BlockPos& pos);
    std::vector<Biome::MobSpawnerData*>* getSwamphutEnemies();

private:
    std::vector<Biome::MobSpawnerData*> mSwamphutEnemies;
    int mSpacing;
    int mSeparation;
};

#endif
