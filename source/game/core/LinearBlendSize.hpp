#ifndef GAME_CORE_LINEARBLENDSIZE_HPP
#define GAME_CORE_LINEARBLENDSIZE_HPP

#include "game/core/LayerSize.hpp"

class LinearBlendSize : public LayerSize
{
public:
    LinearBlendSize(unsigned int minSize, unsigned int maxSize);

    virtual unsigned int GetLayerSize(Random& random, double blend) const;

private:
    int mMinSize;
    int mMaxSize;
};

#endif
