#ifndef GAME_WORLD_GENERATION_ENDGATEWAYFEATURE_HPP
#define GAME_WORLD_GENERATION_ENDGATEWAYFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class BlockPos;
class Level;
class Random;

class EndGatewayFeature : public Feature
{
public:
    explicit EndGatewayFeature(int portalNumber);
    virtual bool place(Level* level, Random& random, const BlockPos& origin);

private:
    int mPortalNumber;
};

#endif
