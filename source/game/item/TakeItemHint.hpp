#ifndef GAME_ITEM_TAKEITEMHINT_HPP
#define GAME_ITEM_TAKEITEMHINT_HPP

#include "game/item/not_null_ptr.hpp"
#include "game/minigame/TutorialHint.hpp"

class ItemInstance;

class TakeItemHint : public TutorialHint
{
public:
    TakeItemHint(eTutorial_Hint hint, Tutorial* tutorial, int* itemIds, unsigned int itemCount, int descriptionId);
    virtual ~TakeItemHint();

    virtual unsigned int onTake(not_null_ptr<ItemInstance> item);

private:
    int* mItemIds;
    unsigned int mItemCount;
};

#endif
