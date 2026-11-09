#include "game/item/LootItemRandomChanceWithLootingCondition.hpp"
#include "game/core/EntityType.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/DamageSource.hpp"
#include "game/core/LootTableContext.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/util/Random.hpp"

LootItemRandomChanceWithLootingCondition::LootItemRandomChanceWithLootingCondition(float chanceValue,
                                                                                   float lootingMultiplierValue)
    : chance(chanceValue),
      lootingMultiplier(lootingMultiplierValue)
{
}

bool LootItemRandomChanceWithLootingCondition::applies(Random* random, LootTableContext& context)
{
    int lootingLevel = 0;

    if ((context.mDamageSource == NULL ? boost::shared_ptr<Entity>() : context.mDamageSource->getEntity())
        && eTYPE_DERIVED_FROM(
            EntityType::LIVING_ENTITY,
            (context.mDamageSource == NULL ? boost::shared_ptr<Entity>() : context.mDamageSource->getEntity())
                ->GetType()))
    {
        lootingLevel = EnchantmentHelper::getMobLooting(boost::dynamic_pointer_cast<LivingEntity>(
            context.mDamageSource == NULL ? boost::shared_ptr<Entity>() : context.mDamageSource->getEntity()));
    }

    return random->nextFloat() < static_cast<float>(lootingLevel) * lootingMultiplier + chance;
}
