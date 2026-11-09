#ifndef GAME_WORLD_BLOCK_LOOTITEMENTITYPROPERTYCONDITION_HPP
#define GAME_WORLD_BLOCK_LOOTITEMENTITYPROPERTYCONDITION_HPP

#include "game/core/LootItemCondition.hpp"
#include "game/core/LootTableContext.hpp"

#include <vector>

class EntityProperty;

class LootItemEntityPropertyCondition : public LootItemCondition
{
public:
    LootItemEntityPropertyCondition(std::vector<EntityProperty*> properties,
                                    LootTableContext::EntityTarget entityTarget);

    virtual bool applies(Random* random, LootTableContext& context);

private:
    std::vector<EntityProperty*> properties;
    LootTableContext::EntityTarget entityTarget;
};

#endif
