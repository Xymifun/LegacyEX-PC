#ifndef GAME_ENTITY_AI_SUBSELECTENTITYSELECTOR_HPP
#define GAME_ENTITY_AI_SUBSELECTENTITYSELECTOR_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/block/Predicate.hpp"

class Entity;
class NearestAttackableTargetGoal;

class SubselectEntitySelector : public Predicate<boost::shared_ptr<Entity> >
{
public:
    SubselectEntitySelector(NearestAttackableTargetGoal* goal, const Predicate<boost::shared_ptr<Entity> >* selector,
                            bool ownsSelector);
    virtual ~SubselectEntitySelector();
    virtual bool apply(boost::shared_ptr<Entity> entity) const;

private:
    bool mOwnsSelector;
    const Predicate<boost::shared_ptr<Entity> >* mSelector;
    NearestAttackableTargetGoal* mGoal;
};

#endif
