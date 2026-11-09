#include "game/item/TakeItemHint.hpp"

#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/minigame/Tutorial.hpp"

TakeItemHint::~TakeItemHint()
{
    delete[] mItemIds;
    mItemIds = 0;
}

unsigned int TakeItemHint::onTake(not_null_ptr<ItemInstance> item)
{
    if (!item->isEmpty())
    {
        bool itemFound = false;
        for (unsigned int i = 0; i < mItemCount; ++i)
        {
            if (Item::VALIDgetId(item->getItem()) == mItemIds[i])
            {
                itemFound = true;
                break;
            }
        }
        if (itemFound)
        {
            Tutorial::PopupMessageDetails* details = new Tutorial::PopupMessageDetails();
            details->titleId = (descriptionId != -1) ? descriptionId : item->getUseDescriptionId();
            details->itemDescriptionId = item->getDescriptionId(-1);
            details->itemId = Item::VALIDgetId(item->getItem());
            details->isItem = true;
            return tutorial->setMessage(this, details);
        }
    }
    return 0;
}
