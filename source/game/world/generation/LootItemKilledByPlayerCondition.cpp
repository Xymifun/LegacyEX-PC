#include "game/world/generation/LootItemKilledByPlayerCondition.hpp"

#include "game/core/LootTableContext.hpp"
#include "game/util/Random.hpp"

LootItemKilledByPlayerCondition::LootItemKilledByPlayerCondition(bool inverse) : inverse(inverse)
{
}

bool LootItemKilledByPlayerCondition::applies(Random* random, LootTableContext& context)
{
    bool killedByPlayer = context.mKillerEntity.lock() != NULL;
    return killedByPlayer == !inverse;
}
