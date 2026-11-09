#ifndef GAME_UI_FURNACEMENU_HPP
#define GAME_UI_FURNACEMENU_HPP

#include <boost/shared_ptr.hpp>

#include "game/ui/AbstractContainerMenu.hpp"

class Container;
class Inventory;
class Player;

class FurnaceMenu : public AbstractContainerMenu
{
public:
    FurnaceMenu(boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Container> container);

    virtual void addSlotListener(net_minecraft_world_inventory::ContainerListener* listener);
    virtual void broadcastChanges();
    virtual void setData(int id, int value);
    virtual bool stillValid(boost::shared_ptr<Player> player);
    int GetInvSlotStart();
    void fjCheckAchievements(const boost::shared_ptr<Player>& player, const not_null_ptr<ItemInstance>& item);
    virtual not_null_ptr<ItemInstance> quickMoveStack(boost::shared_ptr<Player> player, int index);
    virtual not_null_ptr<ItemInstance> clicked(int slot, int button, ClickType::eClickType type,
                                               boost::shared_ptr<Player> player);
    virtual void removed(boost::shared_ptr<Player> player);

private:
    boost::shared_ptr<Container> mContainer;
    int mCookingProgress;
    int mCookingTotalTime;
    int mLitTime;
    int mLitDuration;
};

#endif
