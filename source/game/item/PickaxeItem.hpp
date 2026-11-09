#ifndef GAME_ITEM_PICKAXEITEM_HPP
#define GAME_ITEM_PICKAXEITEM_HPP

#include "game/item/DiggerItem.hpp"

class PickaxeItem : public DiggerItem
{
public:
    PickaxeItem(const Item::Tier* tier);

    virtual bool canDestroySpecial(const BlockState* state);
    virtual float getDestroySpeed(not_null_ptr<ItemInstance> itemInstance, const BlockState* state);

    static void staticCtor();
};

#endif
