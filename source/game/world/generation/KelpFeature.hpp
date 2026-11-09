#ifndef GAME_WORLD_GENERATION_KELPFEATURE_HPP
#define GAME_WORLD_GENERATION_KELPFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class BlockState;

class KelpFeature : public Feature
{
public:
    KelpFeature();
    virtual ~KelpFeature();
    virtual bool place(Level* level, Random& random, const BlockPos& origin);

private:
    bool _canPlace(Level* level, const BlockPos& pos) const;

    const BlockState* mStates[16];
};

#endif
