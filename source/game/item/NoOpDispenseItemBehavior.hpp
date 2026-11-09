#ifndef GAME_ITEM_NOOPDISPENSEITEMBEHAVIOR_HPP
#define GAME_ITEM_NOOPDISPENSEITEMBEHAVIOR_HPP

#include "game/item/DispenseItemBehavior.hpp"
#include "game/item/not_null_ptr.hpp"

class BlockSource;
class ItemInstance;

class NoOpDispenseItemBehavior : public DispenseItemBehavior
{
public:
    virtual ~NoOpDispenseItemBehavior();

    virtual not_null_ptr<ItemInstance> dispense(BlockSource* blockSource, not_null_ptr<ItemInstance> itemInstance);
};

#endif
