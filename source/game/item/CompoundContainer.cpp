#include "game/item/CompoundContainer.hpp"

#include "game/core/L10N.hpp"
#include "game/core/LockCode.hpp"
#include "game/ui/ChestMenu.hpp"
#include "game/item/Inventory.hpp"

CompoundContainer::CompoundContainer(int id, boost::shared_ptr<LockableContainer> first,
                                     boost::shared_ptr<LockableContainer> second)
{
    mId = id;
    if (first == NULL)
    {
        first = second;
    }
    if (second == NULL)
    {
        second = first;
    }
    mFirst = first;
    mSecond = second;
    if (first->isLocked())
    {
        second->setLocked(first->getLockKeyCode());
    }
    else if (second->isLocked())
    {
        first->setLocked(second->getLockKeyCode());
    }
}

CompoundContainer::~CompoundContainer()
{
}

std::wstring CompoundContainer::getName()
{
    if (mFirst->hasCustomName())
    {
        return mFirst->getName();
    }
    if (mSecond->hasCustomName())
    {
        return mSecond->getName();
    }
    return std::wstring(L10N::GetString(mId));
}

std::wstring CompoundContainer::getCustomName()
{
    if (mFirst->hasCustomName())
    {
        return mFirst->getName();
    }
    if (mSecond->hasCustomName())
    {
        return mSecond->getName();
    }
    return std::wstring(L"");
}

bool CompoundContainer::hasCustomName()
{
    return mFirst->hasCustomName() || mSecond->hasCustomName();
}

std::wstring CompoundContainer::getDisplayName()
{
    if (hasCustomName())
    {
        return getCustomName();
    }
    return getName();
}

unsigned int CompoundContainer::getContainerSize()
{
    return mFirst->getContainerSize() + mSecond->getContainerSize();
}

bool CompoundContainer::isEmpty()
{
    return mFirst->isEmpty() && mSecond->isEmpty();
}

not_null_ptr<ItemInstance> CompoundContainer::getItem(unsigned int slot)
{
    if (slot >= mFirst->getContainerSize())
    {
        return mSecond->getItem(slot - mFirst->getContainerSize());
    }
    else
    {
        return mFirst->getItem(slot);
    }
}

not_null_ptr<ItemInstance> CompoundContainer::removeItem(unsigned int slot, int count)
{
    if (slot >= mFirst->getContainerSize())
    {
        return mSecond->removeItem(slot - mFirst->getContainerSize(), count);
    }
    else
    {
        return mFirst->removeItem(slot, count);
    }
}

not_null_ptr<ItemInstance> CompoundContainer::removeItemNoUpdate(int slot)
{
    if (slot >= mFirst->getContainerSize())
    {
        return mSecond->removeItemNoUpdate(slot - mFirst->getContainerSize());
    }
    else
    {
        return mFirst->removeItemNoUpdate(slot);
    }
}

void CompoundContainer::setItem(unsigned int slot, not_null_ptr<ItemInstance> item)
{
    if (slot >= mFirst->getContainerSize())
    {
        mSecond->setItem(slot - mFirst->getContainerSize(), item);
    }
    else
    {
        mFirst->setItem(slot, item);
    }
}

int CompoundContainer::getMaxStackSize()
{
    return mFirst->getMaxStackSize();
}

void CompoundContainer::setChanged()
{
    mFirst->setChanged();
    mSecond->setChanged();
}

bool CompoundContainer::stillValid(boost::shared_ptr<Player> player)
{
    return mFirst->stillValid(player) && mSecond->stillValid(player);
}

void CompoundContainer::startOpen(boost::shared_ptr<Player> player)
{
    mFirst->startOpen(player);
    mSecond->startOpen(player);
}

void CompoundContainer::stopOpen(boost::shared_ptr<Player> player)
{
    mFirst->stopOpen(player);
    mSecond->stopOpen(player);
}

bool CompoundContainer::canPlaceItem(int slot, not_null_ptr<ItemInstance> item)
{
    (void)slot;
    (void)item;
    return true;
}

int CompoundContainer::getContainerData(int id)
{
    (void)id;
    return 0;
}

void CompoundContainer::setContainerData(int id, int value)
{
    (void)id;
    (void)value;
}

int CompoundContainer::getContainerDataCount()
{
    return 0;
}

void CompoundContainer::clearContent()
{
    mFirst->clearContent();
    mSecond->clearContent();
}

bool CompoundContainer::contains(boost::shared_ptr<Container> container)
{
    return mFirst.get() == container.get() || mSecond.get() == container.get();
}

bool CompoundContainer::isLocked()
{
    return mFirst->isLocked() || mSecond->isLocked();
}

void CompoundContainer::setLocked(LockCode* lockCode)
{
    mFirst->setLocked(lockCode);
    mSecond->setLocked(lockCode);
}

LockCode* CompoundContainer::getLockKeyCode()
{
    return mFirst->getLockKeyCode();
}

void CompoundContainer::unlock()
{
    mFirst->unlock();
    mSecond->unlock();
}

int CompoundContainer::getTypeId(bool large)
{
    return mFirst->getTypeId(true);
}

AbstractContainerMenu* CompoundContainer::createMenu(boost::shared_ptr<Inventory> inventory,
                                                     boost::shared_ptr<Player> player)
{
    return new ChestMenu(inventory, shared_from_this(), player);
}
