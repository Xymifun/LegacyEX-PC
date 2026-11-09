#ifndef GAME_WORLD_GENERATION_SEAGRASSFEATURE_HPP
#define GAME_WORLD_GENERATION_SEAGRASSFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class SeagrassFeature : public Feature
{
public:
    SeagrassFeature();
    virtual ~SeagrassFeature();
    bool isNotIce(Level* level, const BlockPos& pos);
    virtual bool place(Level* level, Random& random, const BlockPos& origin);

private:
    bool mGenerating;
};

#endif
