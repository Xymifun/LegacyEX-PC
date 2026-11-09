#ifndef GAME_WORLD_GENERATION_LARGEFEATURE_HPP
#define GAME_WORLD_GENERATION_LARGEFEATURE_HPP

#include <string>

#include "game/util/Random.hpp"

class ChunkPrimer;
class Level;

class LargeFeature
{
public:
    static const std::wstring STRONGHOLD;
    static const std::wstring VILLAGE;
    static const std::wstring MONUMENT;
    static const std::wstring MANSION;
    static const std::wstring END_CITY;
    static const std::wstring TEMPLE;
    static const std::wstring MINESHAFT;
    static const std::wstring FORTRESS;
    static const std::wstring BURIED_TREASURE;
    static const std::wstring SHIPWRECK;
    static const std::wstring RUIN;

    LargeFeature();
    virtual ~LargeFeature();

    virtual void apply(Level* level, int chunkX, int chunkZ, ChunkPrimer* primer);
    virtual void addFeature(Level* level, int chunkX, int chunkZ, int originChunkX, int originChunkZ,
                            ChunkPrimer* primer);

protected:
    static long long setupChunkSeed(long long baseSeed, Random& random, int chunkX, int chunkZ);

    int range;
    Random random;
    Level* level;
};

#endif
