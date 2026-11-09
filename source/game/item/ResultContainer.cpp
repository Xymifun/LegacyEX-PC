#include "game/item/ResultContainer.hpp"

#include "game/item/ContainerHelper.hpp"
#include "game/item/ItemInstance.hpp"

ResultContainer::ResultContainer() : items(NonNullList<not_null_ptr<ItemInstance> >::withSize(1, ItemInstance::EMPTY))
{
}

ResultContainer::~ResultContainer()
{
    delete[] items.items.data;
}

unsigned int ResultContainer::getContainerSize()
{
    return 1;
}

bool ResultContainer::isEmpty()
{
    return false;
}

not_null_ptr<ItemInstance> ResultContainer::getItem(unsigned int slot)
{
    return items[0];
}

not_null_ptr<ItemInstance> ResultContainer::removeItem(unsigned int slot, int count)
{
    return ContainerHelper::takeItem(items, 0);
}

not_null_ptr<ItemInstance> ResultContainer::removeItemNoUpdate(int slot)
{
    return ContainerHelper::takeItem(items, 0);
}

void ResultContainer::setItem(unsigned int slot, not_null_ptr<ItemInstance> itemInstance)
{
    items[0] = itemInstance;
}

int ResultContainer::getMaxStackSize()
{
    return 64;
}

void ResultContainer::setChanged()
{
}

bool ResultContainer::stillValid(boost::shared_ptr<Player> player)
{
    return true;
}

void ResultContainer::startOpen(boost::shared_ptr<Player> player)
{
}

void ResultContainer::stopOpen(boost::shared_ptr<Player> player)
{
}

bool ResultContainer::canPlaceItem(int slot, not_null_ptr<ItemInstance> itemInstance)
{
    return true;
}

int ResultContainer::getContainerData(int id)
{
    return 0;
}

void ResultContainer::setContainerData(int id, int value)
{
}

int ResultContainer::getContainerDataCount()
{
    return 0;
}

void ResultContainer::clearContent()
{
    items.clear();
}

std::wstring ResultContainer::getName()
{
    return std::wstring(L"");
}

std::wstring ResultContainer::getCustomName()
{
    return std::wstring(L"");
}

bool ResultContainer::hasCustomName()
{
    return false;
}

std::wstring ResultContainer::getDisplayName()
{
    if (hasCustomName())
    {
        return getCustomName();
    }
    return getName();
}
