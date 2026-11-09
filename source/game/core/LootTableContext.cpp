#include "game/core/LootTableContext.hpp"

#include <algorithm>

#include "game/core/DamageSource.hpp"
#include "game/world/generation/Player.hpp"

LootTableContext::LootTableContext(float luck, ServerLevel* level, LootTables* lootTables,
                                   boost::shared_ptr<Entity> thisEntity, boost::shared_ptr<Player> killerEntity,
                                   const DamageSource* damageSource)
{
    mLuck = luck;
    mLevel = level;
    mLootTables = lootTables;
    mThisEntity = thisEntity;
    mKillerEntity = killerEntity;
    mDamageSource = damageSource;
}

bool LootTableContext::addVisitedTable(LootTable* lootTable)
{
    std::list<LootTable*>::iterator it = std::find(mVisitedTables.begin(), mVisitedTables.end(), lootTable);

    if (it == mVisitedTables.end())
    {
        mVisitedTables.push_back(lootTable);
        return true;
    }

    return false;
}

void LootTableContext::removeVisitedTable(LootTable* lootTable)
{
    std::list<LootTable*>::iterator it = std::find(mVisitedTables.begin(), mVisitedTables.end(), lootTable);

    if (it != mVisitedTables.end())
    {
        mVisitedTables.erase(it);
    }
}

boost::shared_ptr<Entity> LootTableContext::getEntity(EntityTarget target)
{
    if (target == THIS)
    {
        return mThisEntity.lock();
    }

    if (target == DIRECT_KILLER)
    {
        return mDamageSource == NULL ? boost::shared_ptr<Entity>() : mDamageSource->getEntity();
    }

    if (target == KILLER)
    {
        return mKillerEntity.lock();
    }

    return boost::shared_ptr<Entity>();
}
