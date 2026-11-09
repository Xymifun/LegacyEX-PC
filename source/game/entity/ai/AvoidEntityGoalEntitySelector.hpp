#ifndef GAME_ENTITY_AI_AVOIDENTITYGOALENTITYSELECTOR_HPP
#define GAME_ENTITY_AI_AVOIDENTITYGOALENTITYSELECTOR_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/block/Predicate.hpp"

class AvoidEntityGoal;
class Entity;

class AvoidEntityGoalEntitySelector : public Predicate<boost::shared_ptr<Entity> >
{
public:
    AvoidEntityGoalEntitySelector(AvoidEntityGoal* avoidEntityGoal);
    virtual bool apply(boost::shared_ptr<Entity> entity) const;
    virtual ~AvoidEntityGoalEntitySelector();

private:
    AvoidEntityGoal* mAvoidEntityGoal;
};

#endif
