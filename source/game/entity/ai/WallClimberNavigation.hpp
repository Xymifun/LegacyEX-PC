#ifndef GAME_ENTITY_AI_WALLCLIMBERNAVIGATION_HPP
#define GAME_ENTITY_AI_WALLCLIMBERNAVIGATION_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/ai/GroundPathNavigation.hpp"

class BlockPos;
class Entity;
class Level;
class Mob;
class Path;

class WallClimberNavigation : public GroundPathNavigation
{
public:
    WallClimberNavigation(Mob* mob, Level* level);
    virtual ~WallClimberNavigation();

    using GroundPathNavigation::moveTo;

    virtual Path* createPath(const BlockPos& pos);
    virtual Path* createPath(boost::shared_ptr<Entity> entity);
    virtual bool moveTo(boost::shared_ptr<Entity> entity, double speed);
    virtual void tick();

    void setPosition(BlockPos* pos);

private:
    BlockPos* mTargetPos;
};

#endif
