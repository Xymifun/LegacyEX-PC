#ifndef GAME_ITEM_DISPENSEITEMBEHAVIOR_HPP
#define GAME_ITEM_DISPENSEITEMBEHAVIOR_HPP

#include "game/core/Behavior.hpp"
#include "game/item/not_null_ptr.hpp"

class BlockSource;
class ItemInstance;

class DispenseItemBehavior : public Behavior
{
public:
    virtual ~DispenseItemBehavior() {}

    virtual not_null_ptr<ItemInstance> dispense(BlockSource* blockSource, not_null_ptr<ItemInstance> itemInstance) = 0;
};

#endif
