#include "game/core/LootPool.hpp"

#include <list>

#include "game/core/LootPoolEntry.hpp"
#include "game/core/LootPoolTiers.hpp"
#include "game/core/LootTableContext.hpp"
#include "game/core/Mth.hpp"
#include "game/item/LootItemConditions.hpp"
#include "game/util/Random.hpp"

LootPool::LootPool(std::vector<LootPoolEntry*>* entries, std::vector<LootItemCondition*>* conditions,
                   RandomValueBounds rolls, RandomValueBounds bonusRolls, LootPoolTiers* tiers)
    : mEntries(entries),
      mConditions(conditions),
      mRolls(rolls),
      mBonusRolls(bonusRolls),
      mTiers(tiers)
{
}

void LootPool::addTier(std::vector<not_null_ptr<ItemInstance> >* items, Random* random, LootTableContext& context,
                       int tier)
{
    if (tier < 0 || tier >= (int)mEntries->size())
    {
        return;
    }

    (*mEntries)[tier]->createItem(items, random, context);
}

void LootPool::addRandomItem(std::vector<not_null_ptr<ItemInstance> >* items, Random* random, LootTableContext& context)
{
    std::list<LootPoolEntry*> eligibleEntries;
    int totalWeight = 0;

    for (std::vector<LootPoolEntry*>::iterator it = mEntries->begin(); it != mEntries->end(); ++it)
    {
        LootPoolEntry* entry = *it;
        if (LootItemConditions::allApply(entry->mConditions, random, context))
        {
            int weight = entry->getWeight(context.mLuck);
            if (weight > 0)
            {
                eligibleEntries.push_back(entry);
                totalWeight += weight;
            }
        }
    }

    if (totalWeight == 0 || eligibleEntries.empty())
    {
        return;
    }

    int selectedWeight = random->nextInt(totalWeight);
    for (std::list<LootPoolEntry*>::iterator it = eligibleEntries.begin(); it != eligibleEntries.end(); ++it)
    {
        selectedWeight -= (*it)->getWeight(context.mLuck);
        if (selectedWeight < 0)
        {
            (*it)->createItem(items, random, context);
            return;
        }
    }
}

void LootPool::addRandomItems(std::vector<not_null_ptr<ItemInstance> >* items, Random* random,
                              LootTableContext& context)
{
    if (!LootItemConditions::allApply(mConditions, random, context))
    {
        return;
    }

    if (mTiers != NULL)
    {
        int tierCount = random->nextInt(mTiers->tierCount);
        for (int i = 0; i < mTiers->bonusTierRolls; ++i)
        {
            if (random->nextFloat() < mTiers->bonusTierChance)
            {
                ++tierCount;
            }
        }
        addTier(items, random, context, tierCount);
        return;
    }

    int count = Mth::nextInt(random, Mth::floor(mRolls.min), Mth::floor(mRolls.max));
    count += Mth::floor(Mth::nextFloat(random, mBonusRolls.min, mBonusRolls.max) * context.mLuck);
    for (int i = 0; i < count; ++i)
    {
        addRandomItem(items, random, context);
    }
}
