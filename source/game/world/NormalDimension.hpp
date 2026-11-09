#ifndef GAME_WORLD_NORMALDIMENSION_HPP
#define GAME_WORLD_NORMALDIMENSION_HPP

#include "game/world/Dimension.hpp"

class NormalDimension : public Dimension
{
public:
    NormalDimension();
    virtual bool canUnloadChunk(int x, int z);
    virtual const DimensionType* getType() const;
};

#endif
