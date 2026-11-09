#ifndef GAME_WORLD_GENERATION_BONUSCHESTFEATURE_HPP
#define GAME_WORLD_GENERATION_BONUSCHESTFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class BonusChestFeature : public Feature
{
public:
    virtual ~BonusChestFeature();
    bool place(Level* level, Random& random, const BlockPos& origin, bool useOrigin);
    virtual bool place(Level* level, Random& random, const BlockPos& origin);
};

#endif
