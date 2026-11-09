#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/world/biome/BiomeCache.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/core/MemSect.hpp"

BiomeCache::BiomeCache(BiomeSource* biomeSource)
{
    mLastUpdate = 0;
    mBiomeSource = biomeSource;
    InitializeCriticalSection(&mCriticalSection);
}

BiomeCache::~BiomeCache()
{
    for (std::vector<Area*>::iterator it = mAreaList.begin(); it != mAreaList.end(); ++it)
    {
        delete *it;
    }

    DeleteCriticalSection(&mCriticalSection);
}

BiomeCache::Area* BiomeCache::getBlockAt(int x, int z)
{
    EnterCriticalSection(&mCriticalSection);

    x >>= 4;
    z >>= 4;
    long long key = (static_cast<long long>(x) & 0xFFFFFFFFLL) | ((static_cast<long long>(z) & 0xFFFFFFFFLL) << 32);
    AreaMap::iterator found = mAreas.find(key);
    Area* area = NULL;

    if (found == mAreas.end())
    {
        MemSect(0x30);
        area = new Area(x, z, this);
        mAreas[key] = area;
        mAreaList.push_back(area);
        MemSect(0);
    }
    else
    {
        area = found->second;
    }

    area->mLastUpdate = static_cast<long long>(gApp.getAppTime());
    LeaveCriticalSection(&mCriticalSection);
    return area;
}

Biome* BiomeCache::getBiome(int x, int z, Biome* fallback)
{
    Biome* biome = getBlockAt(x, z)->getBiome(x, z);
    if (biome == NULL)
    {
        return fallback;
    }
    return biome;
}

void BiomeCache::update()
{
    EnterCriticalSection(&mCriticalSection);

    unsigned long long now = static_cast<long long>(gApp.getAppTime());
    unsigned long long elapsed = now - mLastUpdate;
    if (elapsed > 7500)
    {
        mLastUpdate = now;

        std::vector<Area*>::iterator it = mAreaList.begin();
        while (it != mAreaList.end())
        {
            Area* area = *it;
            unsigned long long age = now - area->mLastUpdate;
            if (age > 30000)
            {
                it = mAreaList.erase(it);
                long long key = (static_cast<long long>(area->mX) & 0xFFFFFFFFLL)
                                | ((static_cast<long long>(area->mZ) & 0xFFFFFFFFLL) << 32);
                mAreas.erase(key);
                delete area;
            }
            else
            {
                ++it;
            }
        }
    }

    LeaveCriticalSection(&mCriticalSection);
}

arrayWithLength<Biome*> BiomeCache::getBiomeBlockAt(int x, int z)
{
    arrayWithLength<unsigned char> indices = getBlockAt(x, z)->mBiomeIndices;
    arrayWithLength<Biome*> result(indices.length, true);

    for (unsigned int i = 0; i < indices.length; ++i)
    {
        result[i] = Biome::getBiome(indices[i], Biome::DEFAULT);
    }

    return result;
}

arrayWithLength<unsigned char> BiomeCache::getBiomeIndexBlockAt(int x, int z)
{
    return getBlockAt(x, z)->mBiomeIndices;
}

BiomeCache::Area::Area(int x, int z, BiomeCache* biomeCache)
{
    mBiomeIndices = arrayWithLength<unsigned char>(256, false);
    mLastUpdate = 0;
    mX = x;
    mZ = z;
    biomeCache->mBiomeSource->getBiomeIndexBlock(mBiomeIndices, x << 4, z << 4, 16, 16, false);
}

BiomeCache::Area::~Area()
{
    delete[] mBiomeIndices.data;
}

Biome* BiomeCache::Area::getBiome(int x, int z)
{
    return Biome::biomes[mBiomeIndices.data[(x & 15) | ((z & 15) << 4)]];
}
