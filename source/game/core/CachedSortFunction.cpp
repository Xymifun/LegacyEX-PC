#include "game/core/CachedSortFunction.hpp"

#include "game/util/AABB.hpp"

static inline BlockPos getMinBlockPos(const AABB* bounds)
{
    return BlockPos(bounds->mMinX, bounds->mMinY, bounds->mMinZ);
}

static inline BlockPos getBlockSize(const AABB* bounds)
{
    return BlockPos(bounds->mMaxX - bounds->mMinX, bounds->mMaxY - bounds->mMinY, bounds->mMaxZ - bounds->mMinZ);
}

CachedSortFunction::CachedSortFunction(arrayWithLength<double> values, const AABB* bounds)
    : mValues(values),
      mMin(getMinBlockPos(bounds)),
      mSize(getBlockSize(bounds).offset(BlockPos(1, 1, 1)))
{
}

int CachedSortFunction::getIndex(const BlockPos& pos) const
{
    BlockPos offset = pos.subtract(mMin);

    return offset.y + offset.z * mSize.y + offset.x * mSize.y * mSize.z;
}
