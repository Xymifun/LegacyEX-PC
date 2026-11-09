#ifndef GAME_ITEM_ITEMCOOLDOWNS_HPP
#define GAME_ITEM_ITEMCOOLDOWNS_HPP

#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>

class Item;

class ItemCooldowns
{
public:
    class CooldownInstance
    {
    public:
        CooldownInstance(int startTime, int endTime);

        int mStartTime;
        int mEndTime;
    };

    ItemCooldowns();
    virtual ~ItemCooldowns();

    float getCooldownPercent(Item* item, float partialTicks);
    bool isOnCooldown(Item* item);
    void tick();
    void addCooldown(Item* item, int duration);
    void removeCooldown(Item* item);

    virtual void onCooldownStarted(Item* item, int duration);
    virtual void onCooldownEnded(Item* item);

protected:
    typedef boost::unordered_map<Item*, CooldownInstance*> CooldownMap;

    CooldownMap mCooldowns;
    int mTickCount;
};

#endif
