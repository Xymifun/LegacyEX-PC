#include "game/core/LinearBlendSize.hpp"

#include <cstddef>

#include "game/util/Random.hpp"

LinearBlendSize::LinearBlendSize(unsigned int minSize, unsigned int maxSize)
    : mMinSize(static_cast<int>(minSize)),
      mMaxSize(static_cast<int>(maxSize))
{
}

unsigned int LinearBlendSize::GetLayerSize(Random& random, double blend) const
{
    return mMinSize + (int)(blend * (mMaxSize - mMinSize));
}
