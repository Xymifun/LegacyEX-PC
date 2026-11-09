#include "game/item/SetItemDataFunction.hpp"

#include "game/core/Mth.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/Random.hpp"

SetItemDataFunction::SetItemDataFunction(std::vector<LootItemCondition*>* conditions, RandomValueBounds value)
    : LootItemFunction(conditions)
{
    this->value = value;
}

not_null_ptr<ItemInstance> SetItemDataFunction::apply(const not_null_ptr<ItemInstance>& itemInstance, Random* random,
                                                      LootTableContext& context)
{
    if (!itemInstance->isDamageableItem())
    {
        itemInstance->setAuxValue(Mth::nextInt(random, Mth::floor(value.min), Mth::floor(value.max)));
    }

    return itemInstance;
}
