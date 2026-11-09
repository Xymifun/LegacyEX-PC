#ifndef GAME_CORE_PICKUPTASK_HPP
#define GAME_CORE_PICKUPTASK_HPP

#include <vector>

#include "game/item/not_null_ptr.hpp"
#include "game/minigame/TutorialTask.hpp"

class Item;
class ItemInstance;
class MobEffect;

class PickupTask : public TutorialTask
{
public:
    PickupTask(Item** items, unsigned int itemCount, unsigned int requiredCount, int auxValue, Tutorial* tutorial,
               int descriptionId, bool preCompletionEnabled, std::vector<TutorialConstraint*>* constraints,
               bool showMinimumTime, bool allowFade, bool repeatMessage, MobEffect* effect);

    unsigned int hasPickedUpWoodBlock(not_null_ptr<ItemInstance> item);
    virtual void onTake(not_null_ptr<ItemInstance> item, unsigned int itemCount, unsigned int exactCount);
    virtual bool isCompleted();

private:
    Item** items;
    int itemCount;
    unsigned int requiredCount;
    int auxValue;
    MobEffect* effect;
    std::vector<unsigned int> pickedUpCounts;
};

#endif
