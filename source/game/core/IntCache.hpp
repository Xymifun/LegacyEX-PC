#ifndef GAME_CORE_INTCACHE_HPP
#define GAME_CORE_INTCACHE_HPP

#include <vector>

#include "game/world/biome/arrayWithLength.hpp"

class IntCache
{
public:
    static arrayWithLength<int> allocate(int size);
    static void releaseAll();
    static void CreateNewThreadStorage();
    static void ReleaseThreadStorage();
    static void Reset();
#ifdef MINECRAFT_PC
    struct PcMark
    {
        size_t large;
        size_t small;
    };
    static PcMark pcMark();
    static void pcReleaseSince(const PcMark& mark);
#endif

private:
    class ThreadStorage
    {
    public:
        ThreadStorage();
        ~ThreadStorage();

        int mIntCacheSize;
        std::vector<arrayWithLength<int> > mFreeSmallArrays;
        std::vector<arrayWithLength<int> > mInUseSmallArrays;
        std::vector<arrayWithLength<int> > mFreeLargeArrays;
        std::vector<arrayWithLength<int> > mInUseLargeArrays;
        std::vector<arrayWithLength<int> > mOldLargeArrays;
    };
};

#endif
