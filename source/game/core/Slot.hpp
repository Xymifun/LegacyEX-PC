#ifndef GAME_CORE_SLOT_HPP
#define GAME_CORE_SLOT_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/item/Container.hpp"
#include "game/item/not_null_ptr.hpp"

class ItemInstance;
class Player;

class Slot
{
    friend class AbstractContainerMenu;
    friend class UIScene_AbstractContainerMenu;

public:
    Slot(boost::shared_ptr<Container> container, int index, int x, int y);
    virtual ~Slot() {}

    void onQuickCraft(not_null_ptr<ItemInstance> oldItem, not_null_ptr<ItemInstance> newItem);
    virtual void onQuickCraft(not_null_ptr<ItemInstance> item, int count);
    virtual void onSwapCraft(int count);
    virtual void checkTakeAchievements(not_null_ptr<ItemInstance> item);
    virtual not_null_ptr<ItemInstance> onTake(boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> item);
    virtual bool mayPlace(not_null_ptr<ItemInstance> item);
    virtual not_null_ptr<ItemInstance> getItem();
    virtual bool hasItem();
    virtual void set(not_null_ptr<ItemInstance> item);
    virtual void setChanged();
    virtual int getMaxStackSize();
    virtual int getMaxStackSize(not_null_ptr<ItemInstance> item);
    virtual std::wstring getNoItemIcon();
    virtual not_null_ptr<ItemInstance> remove(int count);
    virtual bool isAt(boost::shared_ptr<Container> container, int index);
    virtual bool mayPickup(boost::shared_ptr<Player> player);
    virtual bool isActive();
    virtual bool mayCombine(not_null_ptr<ItemInstance> item);
    virtual not_null_ptr<ItemInstance> combine(not_null_ptr<ItemInstance> item);

    void swap(Slot* other);

    int mSlot;
    boost::shared_ptr<Container> mContainer;
    int mIndex;
    int mX;
    int mY;
};

#endif
