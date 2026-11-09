#include "game/item/LootItemRandomChanceWithSpecialModifierCondition.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/LootTableContext.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/Random.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/Level.hpp"

LootItemRandomChanceWithSpecialModifierCondition::LootItemRandomChanceWithSpecialModifierCondition(float chanceValue)
    : chance(chanceValue)
{
}

bool LootItemRandomChanceWithSpecialModifierCondition::applies(Random* random, LootTableContext& context)
{
    return random->nextFloat() < chance * context.mLevel->getSpecialMultiplier(context.mThisEntity.lock()->mDimension);
}
