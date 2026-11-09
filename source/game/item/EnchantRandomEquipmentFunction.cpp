#include "game/item/EnchantRandomEquipmentFunction.hpp"

#include "game/core/LootTableContext.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"

EnchantRandomEquipmentFunction::~EnchantRandomEquipmentFunction()
{
}

not_null_ptr<ItemInstance> EnchantRandomEquipmentFunction::apply(const not_null_ptr<ItemInstance>& itemInstance,
                                                                 Random* random, LootTableContext& context)
{
    float specialMultiplier = context.mLevel->getSpecialMultiplier(context.mThisEntity.lock()->mDimension);

    if (random->nextFloat() < chance * specialMultiplier)
    {
        EnchantmentHelper::enchantItem(
            random, itemInstance, static_cast<int>(specialMultiplier * static_cast<float>(random->nextInt(18)) + 5.0f),
            true);
    }

    return itemInstance;
}
