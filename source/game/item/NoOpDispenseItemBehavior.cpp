#include "game/item/NoOpDispenseItemBehavior.hpp"

NoOpDispenseItemBehavior::~NoOpDispenseItemBehavior()
{
}

not_null_ptr<ItemInstance> NoOpDispenseItemBehavior::dispense(BlockSource* blockSource,
                                                              not_null_ptr<ItemInstance> itemInstance)
{
    return itemInstance;
}
