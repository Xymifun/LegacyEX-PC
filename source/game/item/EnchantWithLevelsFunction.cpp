#include "game/item/EnchantWithLevelsFunction.hpp"

#include "game/core/LootTableContext.hpp"
#include "game/core/Mth.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/Random.hpp"

EnchantWithLevelsFunction::EnchantWithLevelsFunction(std::vector<LootItemCondition*>* conditions,
                                                     RandomValueBounds levels, bool allowTreasure)
    : LootItemFunction(conditions)
{
    this->levels = levels;
    this->allowTreasure = allowTreasure;
}

not_null_ptr<ItemInstance> EnchantWithLevelsFunction::apply(const not_null_ptr<ItemInstance>& itemInstance,
                                                            Random* random, LootTableContext& context)
{
    return EnchantmentHelper::enchantItem(random, itemInstance, levels.getInt(random), allowTreasure);
}
