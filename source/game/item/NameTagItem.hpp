#ifndef GAME_ITEM_NAMETAGITEM_HPP
#define GAME_ITEM_NAMETAGITEM_HPP

#include "game/item/Item.hpp"

class NameTagItem : public Item
{
public:
    NameTagItem();
    virtual bool interactEnemy(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                               boost::shared_ptr<LivingEntity> target, InteractionHand::EInteractionHand hand);
};

#endif
