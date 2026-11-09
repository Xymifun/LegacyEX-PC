#ifndef GAME_ITEM_CHORUSFRUITITEM_HPP
#define GAME_ITEM_CHORUSFRUITITEM_HPP

#include <boost/shared_ptr.hpp>

#include "game/item/FoodItem.hpp"

class ItemInstance;
class Level;
class LivingEntity;

class ChorusFruitItem : public FoodItem
{
public:
    ChorusFruitItem(int nutrition, float saturationModifier);
    virtual not_null_ptr<ItemInstance> finishUsingItem(not_null_ptr<ItemInstance> itemInstance, Level* level,
                                                       boost::shared_ptr<LivingEntity> entity);
};

#endif
