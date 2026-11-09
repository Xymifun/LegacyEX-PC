#ifndef GAME_ITEM_SADDLEITEM_HPP
#define GAME_ITEM_SADDLEITEM_HPP

#include "game/item/Item.hpp"

class SaddleItem : public Item
{
public:
    SaddleItem();

    virtual bool interactEnemy(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                               boost::shared_ptr<LivingEntity> target, InteractionHand::EInteractionHand hand);
};

#endif
