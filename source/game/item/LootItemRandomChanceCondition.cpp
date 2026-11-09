#include "game/item/LootItemRandomChanceCondition.hpp"
#include "game/core/LootTableContext.hpp"
#include "game/util/Random.hpp"

bool LootItemRandomChanceCondition::applies(Random* random, LootTableContext& context)
{
    return random->nextFloat() < chance;
}
