#include "game/item/SetItemCountFunction.hpp"

#include "game/core/Mth.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/Random.hpp"

SetItemCountFunction::SetItemCountFunction(std::vector<LootItemCondition*>* conditions, RandomValueBounds count)
    : LootItemFunction(conditions)
{
    this->count = count;
}

not_null_ptr<ItemInstance> SetItemCountFunction::apply(const not_null_ptr<ItemInstance>& itemInstance, Random* random,
                                                       LootTableContext& context)
{
    itemInstance->setCount(count.getInt(random));
    return itemInstance;
}
