#include "game/item/SetItemDamageFunction.hpp"

#include "game/core/Mth.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/Random.hpp"

SetItemDamageFunction::SetItemDamageFunction(std::vector<LootItemCondition*>* conditions, RandomValueBounds damage)
    : LootItemFunction(conditions)
{
    this->damage = damage;
}

not_null_ptr<ItemInstance> SetItemDamageFunction::apply(const not_null_ptr<ItemInstance>& itemInstance, Random* random,
                                                        LootTableContext& context)
{
    if (itemInstance->isDamageableItem())
    {
        float damageFraction = 1.0f - damage.getFloat(random);
        itemInstance->setAuxValue(Mth::floor(damageFraction * itemInstance->getMaxDamage()));
    }

    return itemInstance;
}
