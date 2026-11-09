#include "game/core/FurnaceResultSlot.hpp"

#include <algorithm>
#include <math.h>

#include "game/core/ExperienceOrb.hpp"
#include "game/core/GenericStats.hpp"
#include "game/item/FurnaceRecipes.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/Math.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

FurnaceResultSlot::FurnaceResultSlot(boost::shared_ptr<Player> player, boost::shared_ptr<Container> container,
                                     int index, int x, int y)
    : Slot(container, index, x, y)
{
    mPlayer = player;
    mRemoveCount = 0;
}

bool FurnaceResultSlot::mayPlace(not_null_ptr<ItemInstance> item)
{
    return false;
}

not_null_ptr<ItemInstance> FurnaceResultSlot::remove(int count)
{
    if (hasItem())
    {
        mRemoveCount += std::min(count, getItem()->getCount());
    }
    return Slot::remove(count);
}

not_null_ptr<ItemInstance> FurnaceResultSlot::onTake(boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> item)
{
    checkTakeAchievements(item);
    Slot::onTake(player, item);
    return item;
}

bool FurnaceResultSlot::mayCombine(not_null_ptr<ItemInstance> item)
{
    return false;
}

void FurnaceResultSlot::onQuickCraft(not_null_ptr<ItemInstance> item, int count)
{
    mRemoveCount += count;
    checkTakeAchievements(item);
}

void FurnaceResultSlot::checkTakeAchievements(not_null_ptr<ItemInstance> item)
{
    item->onCraftedBy(mPlayer->mLevel, mPlayer, mRemoveCount);

    if (!mPlayer->mLevel->mIsClientSide)
    {
        int count = mRemoveCount;
        float value = FurnaceRecipes::getInstance()->getRecipeValue(item);
        if (value != 0.0f)
        {
            if (value < 1.0f)
            {
                float experience = count * value;
                count = static_cast<int>(floorf(experience));
                float ceiling = ceilf(experience);
                if (count < ceiling && Math::random() < experience - count)
                {
                    ++count;
                }
            }

            while (count > 0)
            {
                int orbValue = ExperienceOrb::getExperienceValue(count);
                count -= orbValue;
                mPlayer->mLevel->addEntity(boost::shared_ptr<ExperienceOrb>(
                    new ExperienceOrb(mPlayer->mLevel, mPlayer->mX, mPlayer->mY + 0.5, mPlayer->mZ + 0.5, orbValue)));
            }
        }
    }

    if (item->getItem() == Items::ironIngot)
    {
        mPlayer->awardStat(GenericStats::acquireIron(), GenericStats::param_acquireIron());
    }

    if (item->getItem() == Items::cookedFish)
    {
        mPlayer->awardStat(GenericStats::cookFish(), GenericStats::param_cookFish());
    }

    mRemoveCount = 0;
}
