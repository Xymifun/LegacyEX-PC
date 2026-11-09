#ifndef GAME_WORLD_GENERATION_CANATTACKPLAYERPREDICATE_HPP
#define GAME_WORLD_GENERATION_CANATTACKPLAYERPREDICATE_HPP

#include "game/world/block/Predicate.hpp"

#include <boost/shared_ptr.hpp>

class Entity;
class MobNearestPlayerTargetGoal;

class CanAttackPlayerPredicate : public Predicate<boost::shared_ptr<Entity> >
{
public:
    CanAttackPlayerPredicate(MobNearestPlayerTargetGoal* goal);
    virtual bool apply(boost::shared_ptr<Entity> entity) const;
    virtual ~CanAttackPlayerPredicate() {}

private:
    MobNearestPlayerTargetGoal* mGoal;
};

#endif
