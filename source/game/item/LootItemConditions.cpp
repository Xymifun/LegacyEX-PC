#include "game/item/LootItemConditions.hpp"

#include "game/core/LootItemCondition.hpp"

bool LootItemConditions::allApply(const std::vector<LootItemCondition*>* conditions, Random* random,
                                  LootTableContext& context)
{
    if (conditions == 0)
    {
        return true;
    }

    std::vector<LootItemCondition*>::const_iterator it = conditions->begin();
    while (it != conditions->end())
    {
        if (!(*it)->applies(random, context))
        {
            return false;
        }
        ++it;
    }
    return true;
}
