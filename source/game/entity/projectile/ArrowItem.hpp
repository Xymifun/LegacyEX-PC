#ifndef GAME_ENTITY_PROJECTILE_ARROWITEM_HPP
#define GAME_ENTITY_PROJECTILE_ARROWITEM_HPP

#include <boost/shared_ptr.hpp>

#include "game/item/Item.hpp"
#include "game/item/not_null_ptr.hpp"

class Arrow;
class Level;
class LivingEntity;

class ArrowItem : public Item
{
public:
    ArrowItem();
    virtual ~ArrowItem() {}

    virtual boost::shared_ptr<Arrow> createArrow(Level* level, not_null_ptr<ItemInstance> itemInstance,
                                                 boost::shared_ptr<LivingEntity> entity);
};

#endif
