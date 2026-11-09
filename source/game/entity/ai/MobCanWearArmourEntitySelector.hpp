#ifndef GAME_ENTITY_AI_MOBCANWEARARMOURENTITYSELECTOR_HPP
#define GAME_ENTITY_AI_MOBCANWEARARMOURENTITYSELECTOR_HPP

#include <boost/shared_ptr.hpp>

#include "game/item/not_null_ptr.hpp"
#include "game/world/block/Predicate.hpp"

class Entity;
class ItemInstance;

class MobCanWearArmourEntitySelector : public Predicate<boost::shared_ptr<Entity> >
{
public:
    MobCanWearArmourEntitySelector(not_null_ptr<ItemInstance> itemInstance);
    virtual bool apply(boost::shared_ptr<Entity> entity) const;

private:
    not_null_ptr<ItemInstance> mItemInstance;
};

#endif
