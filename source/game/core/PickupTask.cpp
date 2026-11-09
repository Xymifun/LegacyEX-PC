#include "game/core/PickupTask.hpp"

#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Potion.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/world/block/Blocks.hpp"

PickupTask::PickupTask(Item** items, unsigned int itemCount, unsigned int requiredCount, int auxValue,
                       Tutorial* tutorial, int descriptionId, bool preCompletionEnabled,
                       std::vector<TutorialConstraint*>* constraints, bool showMinimumTime, bool allowFade,
                       bool repeatMessage, MobEffect* effect)
    : TutorialTask(tutorial, descriptionId, preCompletionEnabled, constraints, showMinimumTime, allowFade,
                   repeatMessage),
      items(new Item*[itemCount]),
      itemCount(itemCount),
      requiredCount(requiredCount),
      auxValue(auxValue),
      effect(effect)
{
    unsigned int i;
    for (i = 0; i < itemCount; ++i)
    {
        this->items[i] = items[i];
        pickedUpCounts.push_back(0);
    }
}

unsigned int PickupTask::hasPickedUpWoodBlock(not_null_ptr<ItemInstance> item)
{
    Item* pickedUpItem = item->getItem();
    if (pickedUpItem == Item::byBlock(Blocks::log) || pickedUpItem == Item::byBlock(Blocks::log2)
        || pickedUpItem == Item::byBlock(Blocks::stripped_log_dark_oak)
        || pickedUpItem == Item::byBlock(Blocks::stripped_log_acacia)
        || pickedUpItem == Item::byBlock(Blocks::stripped_log_birch)
        || pickedUpItem == Item::byBlock(Blocks::stripped_log_jungle)
        || pickedUpItem == Item::byBlock(Blocks::stripped_log_oak)
        || pickedUpItem == Item::byBlock(Blocks::stripped_log_spruce))
        return true;
    return false;
}

void PickupTask::onTake(not_null_ptr<ItemInstance> item, unsigned int itemCount, unsigned int exactCount)
{
    int i;

    for (i = 0; i < this->itemCount; ++i)
    {
        if (item->getItem() != items[i])
            continue;

        if (items[i] == Items::potion && effect != NULL)
        {
            std::vector<MobEffectInstance*>& effects = PotionUtils::getPotion(item->getTag())->getEffects();
            std::vector<MobEffectInstance*>::iterator effectInstance;

            for (effectInstance = effects.begin(); effectInstance != effects.end(); ++effectInstance)
            {
                if ((*effectInstance)->getEffect() == effect)
                    completed = true;
            }
        }
        else if (hasPickedUpWoodBlock(item) && auxValue == -1)
        {
            pickedUpCounts[i] = itemCount;
        }
        else
        {
            if (auxValue != -1 || itemCount < requiredCount)
            {
                if (auxValue != item->getAuxValue() || exactCount < requiredCount)
                    continue;
            }

            completed = true;
        }
    }

    if (hasPickedUpWoodBlock(item) && auxValue == -1)
    {
        unsigned int total = 0;
        std::vector<unsigned int>::iterator count;

        for (count = pickedUpCounts.begin(); count != pickedUpCounts.end(); ++count)
            total += *count;

        if (total >= requiredCount)
            completed = true;
    }
}

bool PickupTask::isCompleted()
{
    return completed;
}
