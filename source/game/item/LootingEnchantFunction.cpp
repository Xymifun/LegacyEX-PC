#include "game/item/LootingEnchantFunction.hpp"
#include "game/core/EntityType.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/DamageSource.hpp"
#include "game/core/LootTableContext.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/Math.hpp"
#include "game/util/Random.hpp"

LootingEnchantFunction::LootingEnchantFunction(std::vector<LootItemCondition*>* conditions, RandomValueBounds count,
                                               int limit)
    : LootItemFunction(conditions)
{
    this->count = count;
    this->limit = limit;
}

not_null_ptr<ItemInstance> LootingEnchantFunction::apply(const not_null_ptr<ItemInstance>& itemInstance, Random* random,
                                                         LootTableContext& context)
{
    boost::shared_ptr<Entity> killer = context.mDamageSource != NULL ? context.mDamageSource->getEntity()
                                                                     : boost::shared_ptr<Entity>();

    if (killer != NULL && eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, killer->GetType()))
    {
        int lootingLevel = EnchantmentHelper::getMobLooting(boost::static_pointer_cast<LivingEntity>(killer));

        if (lootingLevel == 0)
        {
            return itemInstance;
        }

        float extraCount = (float)lootingLevel * count.getFloat(random);

        itemInstance->grow(Math::round(extraCount));

        if (limit != 0 && itemInstance->getCount() > limit)
        {
            itemInstance->setCount(limit);
        }
    }

    return itemInstance;
}
