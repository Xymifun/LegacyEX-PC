#ifndef GAME_WORLD_GENERATION_ENDPODIUMFEATURE_HPP
#define GAME_WORLD_GENERATION_ENDPODIUMFEATURE_HPP

#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/Feature.hpp"

class EndPodiumFeature : public Feature
{
public:
    static const BlockPos END_PODIUM_LOCATION;

    EndPodiumFeature(bool active);

    virtual bool place(Level* level, Random& random, const BlockPos& origin);

private:
    bool mActive;
};

#endif
