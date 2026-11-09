#ifndef GAME_ENTITY_AI_FLYINGPATHNAVIGATION_HPP
#define GAME_ENTITY_AI_FLYINGPATHNAVIGATION_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/ai/FlyNodeEvaluator.hpp"
#include "game/entity/ai/PathNavigation.hpp"

class Entity;
class Level;
class Mob;
class NodeEvaluator;
class Path;
class PathFinder;
class Vec3;

class FlyingPathNavigation : public PathNavigation
{
public:
    FlyingPathNavigation(Mob* mob, Level* level);

    using PathNavigation::createPath;

    virtual PathFinder* createPathFinder();
    virtual Path* createPath(boost::shared_ptr<Entity> entity);
    virtual void tick();
    virtual Vec3* getTempMobPos();
    virtual bool canUpdatePath();
    virtual bool canMoveDirectly(Vec3* start, Vec3* end, int width, int height, int depth);
    virtual NodeEvaluator* getNodeEvaluator();
    virtual bool isStableDestination(const BlockPos& pos);

    bool canFloat();
    void setCanOpenDoors(bool value);
    void setCanPassDoors(bool value);
    void setCanFloat(bool value);
    bool canPassDoors();
    bool canOpenDoors();

    FlyNodeEvaluator mNodeEvaluator;
};

#endif
