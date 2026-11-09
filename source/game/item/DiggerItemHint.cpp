#include "game/item/DiggerItemHint.hpp"
#include "game/core/EntityType.hpp"

#include "game/entity/mob/Entity.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/world/block/BlockState.hpp"

DiggerItemHint::DiggerItemHint(eTutorial_Hint hint, Tutorial* tutorial, int descriptionId, Item** items,
                               unsigned int itemCount)
    : TutorialHint(hint, tutorial, descriptionId, HINT_NONE, true, true, -1)
{
    mItemCount = itemCount;
    mItems = new Item*[mItemCount];

    for (unsigned int i = 0; i < mItemCount; ++i)
    {
        mItems[i] = items[i];
    }

    tutorial->addMessage(0xe729d451, true, 3);
}

DiggerItemHint::~DiggerItemHint()
{
    delete[] mItems;
}

int DiggerItemHint::startDestroyBlock(not_null_ptr<ItemInstance> item, const BlockState* state)
{
    if (!item->isEmpty())
    {
        bool itemFound = false;
        for (unsigned int i = 0; i < mItemCount; ++i)
        {
            if (item->getItem() == mItems[i])
            {
                itemFound = true;
                break;
            }
        }

        if (itemFound)
        {
            float speed = item->getDestroySpeed(state);
            if (speed == 1.0f)
            {
                return descriptionId;
            }
        }
    }

    return -1;
}

unsigned int DiggerItemHint::attack(not_null_ptr<ItemInstance> item, boost::shared_ptr<Entity> entity)
{
    if (!item->isEmpty())
    {
        bool itemFound = false;
        for (unsigned int i = 0; i < mItemCount; ++i)
        {
            if (item->getItem() == mItems[i])
            {
                itemFound = true;
                break;
            }
        }

        if (itemFound)
        {
            bool isMob = eTYPE_DERIVED_FROM(EntityType::MOB, entity->GetType());
            if (isMob)
            {
                return 0xe729d451U;
            }
            return 0xffffffffU;
        }
    }

    return 0xffffffffU;
}
