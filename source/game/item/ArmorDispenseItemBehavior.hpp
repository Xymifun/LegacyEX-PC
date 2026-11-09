#ifndef GAME_ITEM_ARMORDISPENSEITEMBEHAVIOR_HPP
#define GAME_ITEM_ARMORDISPENSEITEMBEHAVIOR_HPP

#include "game/item/DefaultDispenseItemBehavior.hpp"

class ArmorDispenseItemBehavior : public DefaultDispenseItemBehavior
{
public:
    virtual ~ArmorDispenseItemBehavior();

    virtual not_null_ptr<ItemInstance> execute(BlockSource* blockSource, not_null_ptr<ItemInstance> itemInstance,
                                               eOUTCOME& outcome);
};

#endif
