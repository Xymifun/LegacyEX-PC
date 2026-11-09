#ifndef GAME_CORE_CANATTACKPREDICATE_HPP
#define GAME_CORE_CANATTACKPREDICATE_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/block/Predicate.hpp"

class Entity;
class MobNearestMobTargetGoal;

class CanAttackPredicate : public Predicate<boost::shared_ptr<Entity> >
{
public:
    CanAttackPredicate(MobNearestMobTargetGoal* goal);

    virtual bool apply(boost::shared_ptr<Entity> entity) const;
    virtual Predicate<boost::shared_ptr<Entity> >* copy() const;
    virtual ~CanAttackPredicate() {}

private:
    MobNearestMobTargetGoal* mGoal;
};

#endif
