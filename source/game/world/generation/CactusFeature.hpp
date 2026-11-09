#ifndef GAME_WORLD_GENERATION_CACTUSFEATURE_HPP
#define GAME_WORLD_GENERATION_CACTUSFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class CactusFeature : public Feature
{
public:
    virtual ~CactusFeature();
    virtual bool place(Level* level, Random& random, const BlockPos& origin);
};

#endif
