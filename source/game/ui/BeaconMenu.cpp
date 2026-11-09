#include "game/ui/BeaconMenu.hpp"

#include "game/item/Container.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"
#include "game/network/net_minecraft_world_inventory__ContainerListener.hpp"

BeaconMenu::BeaconMenu(boost::shared_ptr<Container> playerContainer, boost::shared_ptr<Container> beaconContainer)
{
    mBeaconContainer = beaconContainer;
    mPaymentSlot = new PaymentSlot(beaconContainer, 0, 136, 110);
    addSlot(mPaymentSlot);

    int xo = 36;
    int yo = 137;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 9; ++j)
        {
            addSlot(new Slot(playerContainer, j + i * 9 + 9, xo + j * 18, yo + i * 18));
        }
    }

    for (int k = 0; k < 9; ++k)
    {
        addSlot(new Slot(playerContainer, k, xo + k * 18, yo + 58));
    }
}

void BeaconMenu::addSlotListener(net_minecraft_world_inventory::ContainerListener* listener)
{
    AbstractContainerMenu::addSlotListener(listener);
    listener->setAllContainerData(this, mBeaconContainer);
}

void BeaconMenu::setData(int id, int value)
{
    mBeaconContainer->setContainerData(id, value);
}

void BeaconMenu::removed(boost::shared_ptr<Player> player)
{
    AbstractContainerMenu::removed(player);
    if (player.get() != NULL && !player->mLevel->mIsClientSide)
    {
        not_null_ptr<ItemInstance> item = mPaymentSlot->remove(mPaymentSlot->getMaxStackSize());
        if (!item->isEmpty())
            player->drop(item, false);
    }
}

boost::shared_ptr<Container> BeaconMenu::getBeaconContainer()
{
    return mBeaconContainer;
}

bool BeaconMenu::stillValid(boost::shared_ptr<Player> player)
{
    return mBeaconContainer->stillValid(player);
}

not_null_ptr<ItemInstance> BeaconMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
{
    not_null_ptr<ItemInstance> result = ItemInstance::EMPTY;
    Slot* slot = m_slots.at(index);
    if (slot != NULL && slot->hasItem())
    {
        not_null_ptr<ItemInstance> item = slot->getItem();
        result = item->copy();

        if (index == 0)
        {
            if (!moveItemStackTo(item, 1, 37, true, true, true))
                return ItemInstance::EMPTY;
            slot->onQuickCraft(item, result);
        }
        else if (!mPaymentSlot->hasItem() && mPaymentSlot->mayPlace(item) && item->getCount() == 1)
        {
            if (!moveItemStackTo(item, 0, 1, false, true, true))
                return ItemInstance::EMPTY;
        }
        else if (index >= 1 && index < 28)
        {
            if (!moveItemStackTo(item, 28, 37, false, true, true))
                return ItemInstance::EMPTY;
        }
        else if (index >= 28 && index < 37)
        {
            if (!moveItemStackTo(item, 1, 28, false, true, true))
                return ItemInstance::EMPTY;
        }
        else
        {
            if (!moveItemStackTo(item, 1, 37, false, true, true))
                return ItemInstance::EMPTY;
        }

        if (item->getCount() == 0)
            slot->set(ItemInstance::EMPTY);
        else
            slot->setChanged();

        if (item->getCount() == result->getCount())
            return ItemInstance::EMPTY;
        else
            slot->onTake(player, item);
    }
    return result;
}

BeaconMenu::PaymentSlot::PaymentSlot(boost::shared_ptr<Container> container, int index, int x, int y)
    : Slot(container, index, x, y)
{
}

bool BeaconMenu::PaymentSlot::mayPlace(not_null_ptr<ItemInstance> item)
{
    Item* type = item->getItem();
    return type == Items::emerald || type == Items::diamond || type == Items::goldIngot || type == Items::ironIngot;
}

int BeaconMenu::PaymentSlot::getMaxStackSize()
{
    return 1;
}
