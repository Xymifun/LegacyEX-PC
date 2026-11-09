#include "game/item/ItemCooldowns.hpp"

#include "game/core/Mth.hpp"
#include "game/item/Item.hpp"

ItemCooldowns::CooldownInstance::CooldownInstance(int startTime, int endTime) : mStartTime(startTime), mEndTime(endTime)
{
}

ItemCooldowns::ItemCooldowns() : mCooldowns(), mTickCount(0)
{
}

ItemCooldowns::~ItemCooldowns()
{
    for (CooldownMap::iterator it = mCooldowns.begin(); it != mCooldowns.end(); ++it)
    {
        delete it->second;
    }
}

float ItemCooldowns::getCooldownPercent(Item* item, float partialTicks)
{
    CooldownMap::iterator it = mCooldowns.find(item);
    if (it != mCooldowns.end())
    {
        float duration = (float)(it->second->mEndTime - it->second->mStartTime);
        float remaining = (float)it->second->mEndTime - ((float)mTickCount + partialTicks);
        return Mth::clamp(remaining / duration, 0.0f, 1.0f);
    }

    return 0.0f;
}

bool ItemCooldowns::isOnCooldown(Item* item)
{
    return getCooldownPercent(item, 0.0f) > 0.0f;
}

void ItemCooldowns::tick()
{
    ++mTickCount;

    if (!mCooldowns.empty())
    {
        CooldownMap::iterator it = mCooldowns.begin();
        while (it != mCooldowns.end())
        {
            if (it->second->mEndTime <= mTickCount)
            {
                onCooldownEnded(it->first);
                delete it->second;
                it = mCooldowns.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
}

void ItemCooldowns::addCooldown(Item* item, int duration)
{
    mCooldowns[item] = new CooldownInstance(mTickCount, mTickCount + duration);
    onCooldownStarted(item, duration);
}

void ItemCooldowns::removeCooldown(Item* item)
{
    CooldownMap::iterator it = mCooldowns.find(item);
    if (it != mCooldowns.end())
    {
        delete it->second;
        mCooldowns.erase(it);
    }

    onCooldownEnded(item);
}

void ItemCooldowns::onCooldownStarted(Item* item, int duration)
{
}

void ItemCooldowns::onCooldownEnded(Item* item)
{
}
