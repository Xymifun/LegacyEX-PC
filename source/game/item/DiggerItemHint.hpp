#ifndef GAME_ITEM_DIGGERITEMHINT_HPP
#define GAME_ITEM_DIGGERITEMHINT_HPP

#include "game/item/not_null_ptr.hpp"
#include "game/minigame/TutorialHint.hpp"

class BlockState;
class Entity;
class Item;
class ItemInstance;

class DiggerItemHint : public TutorialHint
{
public:
    DiggerItemHint(eTutorial_Hint hint, Tutorial* tutorial, int descriptionId, Item** items, unsigned int itemCount);
    virtual ~DiggerItemHint();

    virtual int startDestroyBlock(not_null_ptr<ItemInstance> item, const BlockState* state);
    virtual unsigned int attack(not_null_ptr<ItemInstance> item, boost::shared_ptr<Entity> entity);

private:
    Item** mItems;
    unsigned int mItemCount;
};

#endif
