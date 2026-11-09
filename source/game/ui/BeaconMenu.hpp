#ifndef GAME_UI_BEACONMENU_HPP
#define GAME_UI_BEACONMENU_HPP

#include <boost/shared_ptr.hpp>

#include "game/ui/AbstractContainerMenu.hpp"
#include "game/core/Slot.hpp"

class Container;
class ItemInstance;
class Player;

namespace net_minecraft_world_inventory
{
class ContainerListener;
}

class BeaconMenu : public AbstractContainerMenu
{
public:
    class PaymentSlot : public Slot
    {
    public:
        PaymentSlot(boost::shared_ptr<Container> container, int index, int x, int y);

        virtual bool mayPlace(not_null_ptr<ItemInstance> item);
        virtual int getMaxStackSize();
    };

    BeaconMenu(boost::shared_ptr<Container> playerContainer, boost::shared_ptr<Container> beaconContainer);

    virtual void addSlotListener(net_minecraft_world_inventory::ContainerListener* listener);
    virtual void setData(int id, int value);
    virtual void removed(boost::shared_ptr<Player> player);
    boost::shared_ptr<Container> getBeaconContainer();
    virtual bool stillValid(boost::shared_ptr<Player> player);
    virtual not_null_ptr<ItemInstance> quickMoveStack(boost::shared_ptr<Player> player, int index);

private:
    boost::shared_ptr<Container> mBeaconContainer;
    PaymentSlot* mPaymentSlot;
};

#endif
