#ifndef GAME_ENTITY_AI_ENTITYBYTYPESELECTOR_HPP
#define GAME_ENTITY_AI_ENTITYBYTYPESELECTOR_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/block/Predicate.hpp"

class Entity;

class EntityByTypeSelector : public Predicate<boost::shared_ptr<Entity> >
{
public:
    EntityByTypeSelector(unsigned long long entityType);

    virtual bool apply(boost::shared_ptr<Entity> entity) const;
    virtual ~EntityByTypeSelector();

    unsigned long long mEntityType;
};

#endif
