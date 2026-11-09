#ifndef GAME_ENTITY_AI_DROWNEDPATHNAVIGATION_HPP
#define GAME_ENTITY_AI_DROWNEDPATHNAVIGATION_HPP

#include "game/entity/ai/GroundPathNavigation.hpp"
#include "game/entity/ai/AmphibiousNodeEvaluator.hpp"

class BlockPos;
class Level;
class Mob;
class PathFinder;
class Vec3;

class DrownedPathNavigation : public GroundPathNavigation
{
public:
    DrownedPathNavigation(Mob* mob, Level* level);
    virtual ~DrownedPathNavigation();

    virtual PathFinder* createPathFinder();
    virtual Path* createPath(const BlockPos& pos);
    virtual void tick();
    virtual void updatePath();
    virtual Vec3* getTempMobPos();
    virtual bool canUpdatePath();
    virtual bool canMoveDirectly(Vec3* start, Vec3* end, int width, int height, int depth);
    virtual bool isStableDestination(const BlockPos& pos);

private:
    AmphibiousNodeEvaluator mAmphibiousNodeEvaluator;
    PathFinder* mGroundPathFinder;
    PathFinder* mWaterPathFinder;
};

#endif
