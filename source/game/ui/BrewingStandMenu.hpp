#ifndef GAME_UI_BREWINGSTANDMENU_HPP
#define GAME_UI_BREWINGSTANDMENU_HPP

#include <boost/shared_ptr.hpp>

#include "game/ui/AbstractContainerMenu.hpp"

class Inventory;

class BrewingStandMenu : public AbstractContainerMenu
{
public:
    class PotionSlot : public Slot
    {
    public:
        PotionSlot(boost::shared_ptr<Container> container, int index, int x, int y);
        static bool mayPlaceItem(not_null_ptr<ItemInstance> item);
        virtual bool mayPlace(not_null_ptr<ItemInstance> item);
        virtual int getMaxStackSize();
        virtual not_null_ptr<ItemInstance> onTake(boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> item);
        virtual bool mayCombine(not_null_ptr<ItemInstance> item);
    };

    class IngredientsSlot : public Slot
    {
    public:
        IngredientsSlot(boost::shared_ptr<Container> container, int index, int x, int y);
        virtual bool mayPlace(not_null_ptr<ItemInstance> item);
        virtual int getMaxStackSize();
        virtual bool mayCombine(not_null_ptr<ItemInstance> item);
    };

    class FuelSlot : public Slot
    {
    public:
        FuelSlot(boost::shared_ptr<Container> container, int index, int x, int y);
        static bool mayPlaceItem(not_null_ptr<ItemInstance> item);
        virtual bool mayPlace(not_null_ptr<ItemInstance> item);
        virtual int getMaxStackSize();
    };

    BrewingStandMenu(boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Container> container);
    virtual void addSlotListener(net_minecraft_world_inventory::ContainerListener* listener);
    virtual void broadcastChanges();
    virtual void setData(int id, int value);
    virtual bool stillValid(boost::shared_ptr<Player> player);
    virtual not_null_ptr<ItemInstance> quickMoveStack(boost::shared_ptr<Player> player, int index);

private:
    boost::shared_ptr<Container> mContainer;
    Slot* mIngredientSlot;
    int mLastBrewTime;
    int mLastFuel;
};

#endif
