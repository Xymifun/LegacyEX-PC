#ifndef GAME_WORLD_BIOME_BIOMECACHE_HPP
#define GAME_WORLD_BIOME_BIOMECACHE_HPP

#include <boost/unordered_map.hpp>
#include <stdint.h>
#include <vector>

#include "game/core/CriticalSection.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/chunk/PlayerChunkMap.hpp"

class Biome;
class BiomeSource;

class BiomeCache
{
public:
    class Area
    {
    public:
        Area(int x, int z, BiomeCache* biomeCache);
        ~Area();

        Biome* getBiome(int x, int z);

        arrayWithLength<unsigned char> mBiomeIndices;
        int mX;
        int mZ;
        long long mLastUpdate;
    };

    typedef boost::unordered_map<long long, Area*, LongKeyHash, LongKeyEq> AreaMap;

    BiomeCache(BiomeSource* biomeSource);
    ~BiomeCache();

    Area* getBlockAt(int x, int z);
    Biome* getBiome(int x, int z, Biome* fallback);
    void update();
    arrayWithLength<Biome*> getBiomeBlockAt(int x, int z);
    arrayWithLength<unsigned char> getBiomeIndexBlockAt(int x, int z);

private:
    BiomeSource* mBiomeSource;
    long long mLastUpdate;
    AreaMap mAreas;
    std::vector<Area*> mAreaList;
    OSFastMutex mCriticalSection;
};

#endif
