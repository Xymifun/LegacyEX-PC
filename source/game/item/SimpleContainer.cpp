#include "game/item/SimpleContainer.hpp"

#include <algorithm>

#include "game/core/L10N.hpp"
#include "game/item/ContainerHelper.hpp"
#include "game/item/ItemInstance.hpp"

SimpleContainer::SimpleContainer(int id, const std::wstring& name, bool hasCustomName, int size)
{
    _init(id, name, hasCustomName, size);
}

SimpleContainer::SimpleContainer(const std::wstring& name, int size)
{
    _init(0, name, true, size);
}

SimpleContainer::~SimpleContainer()
{
    delete[] mItems.items.data;
}

void SimpleContainer::_init(int id, const std::wstring& name, bool hasCustomName, int size)
{
    mId = id;
    mName.assign(name, 0, 0xffffffff);
    mHasCustomName = hasCustomName;
    mSize = size;
    mItems = NonNullList<not_null_ptr<ItemInstance> >::withSize((unsigned int)size, ItemInstance::EMPTY);
}

std::wstring SimpleContainer::getName()
{
    return mName.empty() ? L10N::GetString(mId) : mName;
}

std::wstring SimpleContainer::getCustomName()
{
    if (hasCustomName())
    {
        return mName;
    }
    return std::wstring(L"");
}

bool SimpleContainer::hasCustomName()
{
    return mHasCustomName;
}

std::wstring SimpleContainer::getDisplayName()
{
    return getName();
}

unsigned int SimpleContainer::getContainerSize()
{
    return (unsigned int)mSize;
}

bool SimpleContainer::isEmpty()
{
    for (unsigned int i = 0; i < mItems.size(); ++i)
    {
        if (!mItems[i]->isEmpty())
        {
            return false;
        }
    }
    return true;
}

not_null_ptr<ItemInstance> SimpleContainer::getItem(unsigned int slot)
{
    if (slot >= mItems.size())
    {
        return ItemInstance::EMPTY;
    }
    return mItems[slot];
}

not_null_ptr<ItemInstance> SimpleContainer::removeItem(unsigned int slot, int count)
{
    not_null_ptr<ItemInstance> result = ContainerHelper::removeItem(mItems, slot, (unsigned int)count);
    if (!result->isEmpty())
    {
        setChanged();
    }
    return result;
}

not_null_ptr<ItemInstance> SimpleContainer::removeItemNoUpdate(int slot)
{
    not_null_ptr<ItemInstance> result = mItems[(unsigned int)slot];
    if (result->isEmpty())
    {
        return ItemInstance::EMPTY;
    }
    mItems.set((unsigned int)slot, ItemInstance::EMPTY);
    return result;
}

void SimpleContainer::setItem(unsigned int slot, not_null_ptr<ItemInstance> item)
{
    if (slot >= mItems.size())
    {
        return;
    }
    mItems[slot] = item;
    if (!item->isEmpty() && item->getCount() > getMaxStackSize())
    {
        item->setCount(getMaxStackSize());
    }
    setChanged();
}

int SimpleContainer::getMaxStackSize()
{
    return 64;
}

void SimpleContainer::setChanged()
{
    for (unsigned int i = 0; i < mListeners.size(); ++i)
    {
        mListeners.at(i)->containerChanged(this);
    }
}

bool SimpleContainer::stillValid(boost::shared_ptr<Player> player)
{
    return true;
}

bool SimpleContainer::canPlaceItem(int slot, not_null_ptr<ItemInstance> item)
{
    return true;
}

int SimpleContainer::getContainerData(int id)
{
    return 0;
}

void SimpleContainer::setContainerData(int id, int value)
{
}

int SimpleContainer::getContainerDataCount()
{
    return 0;
}

void SimpleContainer::clearContent()
{
    mItems.clear();
}

void SimpleContainer::addListener(net_minecraft_world::ContainerListener* listener)
{
    mListeners.push_back(listener);
}

void SimpleContainer::removeListener(net_minecraft_world::ContainerListener* listener)
{
    std::vector<net_minecraft_world::ContainerListener*>::iterator it;
    for (it = mListeners.begin(); it != mListeners.end() && *it != listener; ++it)
    {
    }
    if (it != mListeners.end())
    {
        mListeners.erase(it);
    }
}

not_null_ptr<ItemInstance> SimpleContainer::addItem(not_null_ptr<ItemInstance> item)
{
    not_null_ptr<ItemInstance> remainder = item->copy();
    for (int i = 0; i < mSize; ++i)
    {
        boost::shared_ptr<ItemInstance> current = getItem((unsigned int)i);
        if (current->isEmpty())
        {
            setItem((unsigned int)i, remainder);
            setChanged();
            return ItemInstance::EMPTY;
        }
        if (ItemInstance::isSame(current, remainder))
        {
            int maxSize = std::min(getMaxStackSize(), current->getMaxStackSize());
            int count = std::min(remainder->getCount(), maxSize - current->getCount());
            if (count > 0)
            {
                current->grow(count);
                remainder->shrink(count);
                if (remainder->isEmpty())
                {
                    setChanged();
                    return ItemInstance::EMPTY;
                }
            }
        }
    }
    if (remainder->getCount() != item->getCount())
    {
        setChanged();
    }
    return remainder;
}

void SimpleContainer::setCustomName(const std::wstring& name)
{
    mHasCustomName = true;
    mName.assign(name, 0, 0xffffffff);
}
