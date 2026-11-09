#ifndef GAME_ENTITY_AI_GROUNDPATHNAVIGATION_HPP
#define GAME_ENTITY_AI_GROUNDPATHNAVIGATION_HPP

#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/ai/WalkNodeEvaluator.hpp"

class Entity;
class Level;
class Mob;
class NodeEvaluator;
class Path;
class Vec3;

class GroundPathNavigation : public PathNavigation
{
public:
    GroundPathNavigation(Mob* mob, Level* level, bool createPathFinder);
    virtual ~GroundPathNavigation() {}

    virtual NodeEvaluator* getNodeEvaluator();
    virtual PathFinder* createPathFinder();
    bool canFloat();
    bool canSinkInWaterWalk();
    virtual bool canUpdatePath();
    int getSurfaceY();
    virtual Vec3* getTempMobPos();
    virtual Path* createPath(const BlockPos& pos);
    virtual Path* createPath(boost::shared_ptr<Entity> entity);
    virtual void trimPath();
    virtual bool canMoveDirectly(Vec3* start, Vec3* end, int width, int height, int depth);

    bool canWalkAbove(int x, int y, int z, int sizeX, int sizeY, int sizeZ, Vec3* pos, double xStep, double zStep);
    bool canWalkOn(int x, int y, int z, int sizeX, int sizeY, int sizeZ, Vec3* pos, double xStep, double zStep);
    void setCanOpenDoors(bool value);
    void setCanPassDoors(bool value);
    bool canPassDoors();
    bool canOpenDoors();
    void setCanFloat(bool value);
    void setAvoidSun(bool value);
    virtual bool shouldAvoidSun() const;
    void setSinkInWaterWalk(bool value);

protected:
    WalkNodeEvaluator mNodeEvaluator;
    bool mAvoidSun;
};

#endif
