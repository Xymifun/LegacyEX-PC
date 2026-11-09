#ifndef GAME_ITEM_AREAEFFECTCLOUD_PREDICATE_HPP
#define GAME_ITEM_AREAEFFECTCLOUD_PREDICATE_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/block/Predicate.hpp"

class Entity;

class AreaEffectCloud_Predicate : public Predicate<boost::shared_ptr<Entity> >
{
public:
    virtual bool apply(boost::shared_ptr<Entity> entity) const;
    virtual ~AreaEffectCloud_Predicate() {}
};

#endif
