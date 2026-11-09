#ifndef GAME_ENTITY_AI_RIDINGSELECTOR_HPP
#define GAME_ENTITY_AI_RIDINGSELECTOR_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/block/Predicate.hpp"

class Entity;

class RidingSelector : public Predicate<boost::shared_ptr<Entity> >
{
public:
    RidingSelector(const boost::shared_ptr<Entity>& vehicle, bool riding) : vehicle(vehicle), riding(riding) {}

    virtual bool apply(boost::shared_ptr<Entity> entity) const;

    boost::shared_ptr<Entity> vehicle;
    bool riding;
};

#endif
