#include "game/ui/LocalMenuContainer.hpp"

#include "game/core/LockCode.hpp"

LocalMenuContainer::LocalMenuContainer(int typeId, const std::wstring& name, int size)
    : SimpleContainer(name, size),
      LockableContainer(),
      mContainerData()
{
    mTypeId = typeId;
}

LocalMenuContainer::~LocalMenuContainer()
{
}

std::wstring LocalMenuContainer::getName()
{
    return SimpleContainer::getName();
}

std::wstring LocalMenuContainer::getCustomName()
{
    return SimpleContainer::getCustomName();
}

bool LocalMenuContainer::hasCustomName()
{
    return SimpleContainer::hasCustomName();
}

std::wstring LocalMenuContainer::getDisplayName()
{
    return SimpleContainer::getDisplayName();
}

unsigned int LocalMenuContainer::getContainerSize()
{
    return SimpleContainer::getContainerSize();
}

bool LocalMenuContainer::isEmpty()
{
    return SimpleContainer::isEmpty();
}

not_null_ptr<ItemInstance> LocalMenuContainer::getItem(unsigned int slot)
{
    return SimpleContainer::getItem(slot);
}

not_null_ptr<ItemInstance> LocalMenuContainer::removeItem(unsigned int slot, int count)
{
    return SimpleContainer::removeItem(slot, count);
}

not_null_ptr<ItemInstance> LocalMenuContainer::removeItemNoUpdate(int slot)
{
    return SimpleContainer::removeItemNoUpdate(slot);
}

void LocalMenuContainer::setItem(unsigned int slot, not_null_ptr<ItemInstance> item)
{
    SimpleContainer::setItem(slot, item);
}

int LocalMenuContainer::getMaxStackSize()
{
    return SimpleContainer::getMaxStackSize();
}

void LocalMenuContainer::setChanged()
{
    SimpleContainer::setChanged();
}

bool LocalMenuContainer::stillValid(boost::shared_ptr<Player> player)
{
    return SimpleContainer::stillValid(player);
}

void LocalMenuContainer::startOpen(boost::shared_ptr<Player> player)
{
    SimpleContainer::startOpen(player);
}

void LocalMenuContainer::stopOpen(boost::shared_ptr<Player> player)
{
    SimpleContainer::stopOpen(player);
}

bool LocalMenuContainer::canPlaceItem(int slot, not_null_ptr<ItemInstance> item)
{
    return SimpleContainer::canPlaceItem(slot, item);
}

int LocalMenuContainer::getContainerData(int id)
{
    boost::unordered_map<int, int>::iterator it = mContainerData.find(id);
    if (it != mContainerData.end())
    {
        return it->second;
    }
    return 0;
}

void LocalMenuContainer::setContainerData(int id, int value)
{
    mContainerData[id] = value;
}

int LocalMenuContainer::getContainerDataCount()
{
    return (int)mContainerData.size();
}

void LocalMenuContainer::clearContent()
{
    SimpleContainer::clearContent();
}

bool LocalMenuContainer::isLocked()
{
    return false;
}

void LocalMenuContainer::setLocked(LockCode* lockCode)
{
}

LockCode* LocalMenuContainer::getLockKeyCode()
{
    return LockCode::NO_LOCK;
}

void LocalMenuContainer::unlock()
{
}

int LocalMenuContainer::getTypeId(bool large)
{
    return mTypeId;
}

AbstractContainerMenu* LocalMenuContainer::createMenu(boost::shared_ptr<Inventory> inventory,
                                                      boost::shared_ptr<Player> player)
{
    return NULL;
}
