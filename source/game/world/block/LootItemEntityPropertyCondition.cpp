#include "game/world/block/LootItemEntityPropertyCondition.hpp"

#include "game/world/block/EntityProperty.hpp"

LootItemEntityPropertyCondition::LootItemEntityPropertyCondition(std::vector<EntityProperty*> properties,
                                                                 LootTableContext::EntityTarget entityTarget)
{
    this->properties = properties;
    this->entityTarget = entityTarget;
}

bool LootItemEntityPropertyCondition::applies(Random* random, LootTableContext& context)
{
    boost::shared_ptr<Entity> entity = context.getEntity(entityTarget);
    if (!entity || properties.empty())
    {
        return false;
    }

    std::vector<EntityProperty*>::const_iterator it = properties.begin();
    while (it != properties.end())
    {
        if (!(*it)->applies(random, entity))
        {
            return false;
        }
        ++it;
    }
    return true;
}
