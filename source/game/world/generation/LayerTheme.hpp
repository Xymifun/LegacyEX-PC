#ifndef GAME_WORLD_GENERATION_LAYERTHEME_HPP
#define GAME_WORLD_GENERATION_LAYERTHEME_HPP

#include "game/util/Random.hpp"

class BlockTransaction;

class LayerTheme
{
public:
    virtual void Apply(Random& random, BlockTransaction* pTransaction, int& iMinY, int& iMaxY) const = 0;
};

#endif
