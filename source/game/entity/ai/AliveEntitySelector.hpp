#ifndef GAME_ENTITY_AI_ALIVEENTITYSELECTOR_HPP
#define GAME_ENTITY_AI_ALIVEENTITYSELECTOR_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/block/Predicate.hpp"

class Entity;

class AliveEntitySelector : public Predicate<boost::shared_ptr<Entity> >
{
public:
    virtual bool apply(boost::shared_ptr<Entity> entity) const;
    virtual ~AliveEntitySelector();
};

#endif
