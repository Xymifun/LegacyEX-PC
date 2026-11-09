#ifndef GAME_CORE_CACHEDSORTFUNCTION_HPP
#define GAME_CORE_CACHEDSORTFUNCTION_HPP

#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BlockPos.hpp"

class AABB;

class CachedSortFunction
{
public:
    CachedSortFunction(arrayWithLength<double> values, const AABB* bounds);

    int getIndex(const BlockPos& pos) const;

    bool operator()(const BlockPos& first, const BlockPos& second) const
    {
        return mValues.data[getIndex(first)] < mValues.data[getIndex(second)];
    }

private:
    arrayWithLength<double> mValues;
    BlockPos mMin;
    BlockPos mSize;
};

#endif
