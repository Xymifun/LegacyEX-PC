#ifndef GAME_ENTITY_AI_PUSHABLEBYSELECTOR_HPP
#define GAME_ENTITY_AI_PUSHABLEBYSELECTOR_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/block/Predicate.hpp"

class Entity;
class Team;

class PushableBySelector : public Predicate<boost::shared_ptr<Entity> >
{
public:
    PushableBySelector(const boost::shared_ptr<Entity>& pusher);

    virtual bool apply(boost::shared_ptr<Entity> entity) const;
    virtual ~PushableBySelector() {}

    boost::shared_ptr<Entity> pusher;
    Team* team;
};

#endif
