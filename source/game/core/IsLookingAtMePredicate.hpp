#ifndef GAME_CORE_ISLOOKINGATMEPREDICATE_HPP
#define GAME_CORE_ISLOOKINGATMEPREDICATE_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/block/Predicate.hpp"

class EnderMan;
class Entity;

class IsLookingAtMePredicate : public Predicate<boost::shared_ptr<Entity> >
{
public:
    virtual bool apply(boost::shared_ptr<Entity> entity) const;
    virtual ~IsLookingAtMePredicate() {}

    EnderMan* mEnderMan;
};

#endif
