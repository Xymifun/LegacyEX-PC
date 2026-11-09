#include "game/item/ArmorDispenseItemBehavior.hpp"

#include "game/item/ArmorItem.hpp"
#include "game/item/ItemInstance.hpp"

ArmorDispenseItemBehavior::~ArmorDispenseItemBehavior()
{
}

not_null_ptr<ItemInstance> ArmorDispenseItemBehavior::execute(BlockSource* blockSource,
                                                              not_null_ptr<ItemInstance> itemInstance,
                                                              eOUTCOME& outcome)
{
    const not_null_ptr<ItemInstance> result = ArmorItem::dispenseArmor(blockSource, itemInstance, outcome);

    return result->isEmpty() ? DefaultDispenseItemBehavior::execute(blockSource, itemInstance, outcome) : result;
}
