#ifndef GAME_ITEM_BOWLFOODITEM_HPP
#define GAME_ITEM_BOWLFOODITEM_HPP

#include "game/item/FoodItem.hpp"

class BowlFoodItem : public FoodItem
{
public:
    BowlFoodItem(int nutrition);
    virtual not_null_ptr<ItemInstance> finishUsingItem(not_null_ptr<ItemInstance> itemInstance, Level* level,
                                                       boost::shared_ptr<LivingEntity> entity);
};

#endif
