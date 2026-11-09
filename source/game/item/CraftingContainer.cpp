#include "game/item/CraftingContainer.hpp"

#include "game/item/ContainerHelper.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/AbstractContainerMenu.hpp"

CraftingContainer::CraftingContainer(AbstractContainerMenu* menu, unsigned int width, unsigned int height)
    : Container(),
      mItems(NonNullList<not_null_ptr<ItemInstance> >::withSize(width * height, ItemInstance::EMPTY))
{
    mMenu = menu;
    mWidth = width;
    mHeight = height;
}

CraftingContainer::~CraftingContainer()
{
    delete[] mItems.items.data;
}

std::wstring CraftingContainer::getName()
{
    return std::wstring(L"");
}

std::wstring CraftingContainer::getCustomName()
{
    return std::wstring(L"");
}

bool CraftingContainer::hasCustomName()
{
    return false;
}

std::wstring CraftingContainer::getDisplayName()
{
    if (hasCustomName())
    {
        return getName();
    }
    return getName();
}

unsigned int CraftingContainer::getContainerSize()
{
    return mItems.size();
}

bool CraftingContainer::isEmpty()
{
    for (unsigned int i = 0; i < mItems.size(); ++i)
    {
        not_null_ptr<ItemInstance> item = mItems[i];
        if (!item->isEmpty())
        {
            return false;
        }
    }
    return true;
}

not_null_ptr<ItemInstance> CraftingContainer::getItem(unsigned int slot)
{
    if (slot >= getContainerSize())
    {
        return ItemInstance::EMPTY;
    }
    return mItems[slot];
}

not_null_ptr<ItemInstance> CraftingContainer::getItem(unsigned int row, unsigned int column)
{
    if (row >= mWidth || column > mHeight)
        return ItemInstance::EMPTY;
    return getItem(row + column * mWidth);
}

not_null_ptr<ItemInstance> CraftingContainer::removeItem(unsigned int slot, int count)
{
    not_null_ptr<ItemInstance> result = ContainerHelper::removeItem(mItems, slot, count);
    if (!result->isEmpty())
    {
        mMenu->slotsChanged();
    }
    return result;
}

not_null_ptr<ItemInstance> CraftingContainer::removeItemNoUpdate(int slot)
{
    return ContainerHelper::takeItem(mItems, slot);
}

void CraftingContainer::setItem(unsigned int slot, not_null_ptr<ItemInstance> item)
{
    mItems[slot] = item;
    if (mMenu != NULL)
    {
        mMenu->slotsChanged();
    }
}

int CraftingContainer::getMaxStackSize()
{
    return 64;
}

void CraftingContainer::setChanged()
{
}

bool CraftingContainer::stillValid(boost::shared_ptr<Player> player)
{
    return true;
}

void CraftingContainer::startOpen(boost::shared_ptr<Player> player)
{
}

void CraftingContainer::stopOpen(boost::shared_ptr<Player> player)
{
}

bool CraftingContainer::canPlaceItem(int slot, not_null_ptr<ItemInstance> item)
{
    return true;
}

int CraftingContainer::getContainerData(int id)
{
    return 0;
}

void CraftingContainer::setContainerData(int id, int value)
{
}

int CraftingContainer::getContainerDataCount()
{
    return 0;
}

void CraftingContainer::clearContent()
{
    mItems.clear();
}

int CraftingContainer::getHeight()
{
    return mHeight;
}

int CraftingContainer::getWidth()
{
    return mWidth;
}
