#include "game/core/SetNBTFunction.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"

SetNBTFunction::SetNBTFunction(std::vector<LootItemCondition*>* conditions, CompoundTag* tag)
    : LootItemFunction(conditions),
      tag(tag)
{
}

not_null_ptr<ItemInstance> SetNBTFunction::apply(const not_null_ptr<ItemInstance>& itemInstance, Random* random,
                                                 LootTableContext& context)
{
    CompoundTag* itemTag = itemInstance->getTag();

    if (itemTag == NULL)
    {
        itemTag = tag->copy();
    }
    else
    {
        itemTag->merge(tag);
    }

    itemInstance->setTag(itemTag);

    return itemInstance;
}
