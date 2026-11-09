#ifndef GAME_ENTITY_AI_WATERBOUNDPATHNAVIGATION_HPP
#define GAME_ENTITY_AI_WATERBOUNDPATHNAVIGATION_HPP

#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/ai/SwimNodeEvaluator.hpp"

class BlockPos;
class Level;
class Mob;
class NodeEvaluator;
class PathFinder;
class Vec3;

class WaterBoundPathNavigation : public PathNavigation
{
public:
    WaterBoundPathNavigation(Mob* mob, Level* level, bool createPathFinder, bool allowBreaching);
    virtual ~WaterBoundPathNavigation() {}

    virtual PathFinder* createPathFinder();
    virtual void updatePath();
    virtual Vec3* getTempMobPos();
    virtual bool canUpdatePath();
    virtual bool canMoveDirectly(Vec3* a, Vec3* b, int x, int y, int z);
    virtual NodeEvaluator* getNodeEvaluator();
    virtual bool isStableDestination(const BlockPos& pos);

    SwimNodeEvaluator mNodeEvaluator;
};

#endif
