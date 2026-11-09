#ifndef GAME_WORLD_GENERATION_VOIDLAYERTHEME_HPP
#define GAME_WORLD_GENERATION_VOIDLAYERTHEME_HPP

#include "game/world/generation/LayerTheme.hpp"

class VoidLayerTheme : public LayerTheme
{
public:
    virtual void Apply(Random& random, BlockTransaction* pTransaction, int& iMinY, int& iMaxY) const;
};

#endif
