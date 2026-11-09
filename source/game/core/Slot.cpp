#include "game/core/Slot.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/ArmorDyeRecipe.hpp"

#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/Recipes.hpp"

Slot::Slot(boost::shared_ptr<Container> container, int index, int x, int y)
    : mSlot(index),
      mContainer(container),
      mIndex(0),
      mX(x),
      mY(y)
{
}

void Slot::onQuickCraft(not_null_ptr<ItemInstance> oldItem, not_null_ptr<ItemInstance> newItem)
{
    int diff = newItem->getCount() - oldItem->getCount();
    if (diff > 0)
    {
        onQuickCraft(newItem, diff);
    }
}

void Slot::onQuickCraft(not_null_ptr<ItemInstance> item, int count)
{
    (void)item;
    (void)count;
}

void Slot::onSwapCraft(int count)
{
}

void Slot::checkTakeAchievements(not_null_ptr<ItemInstance> item)
{
}

not_null_ptr<ItemInstance> Slot::onTake(boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> item)
{
    setChanged();
    return item;
}

bool Slot::mayPlace(not_null_ptr<ItemInstance> item)
{
    return true;
}

not_null_ptr<ItemInstance> Slot::getItem()
{
    return mContainer->getItem(mSlot);
}

bool Slot::hasItem()
{
    return !getItem()->isEmpty();
}

void Slot::set(not_null_ptr<ItemInstance> item)
{
    mContainer->setItem(mSlot, item);
    setChanged();
}

void Slot::setChanged()
{
    mContainer->setChanged();
}

int Slot::getMaxStackSize()
{
    return mContainer->getMaxStackSize();
}

int Slot::getMaxStackSize(not_null_ptr<ItemInstance> item)
{
    return getMaxStackSize();
}

std::wstring Slot::getNoItemIcon()
{
    return std::wstring((const wchar_t*)NULL);
}

not_null_ptr<ItemInstance> Slot::remove(int count)
{
    return mContainer->removeItem(mSlot, count);
}

bool Slot::isAt(boost::shared_ptr<Container> container, int index)
{
    return container.get() == mContainer.get() && index == mSlot;
}

bool Slot::mayPickup(boost::shared_ptr<Player> player)
{
    return true;
}

bool Slot::isActive()
{
    return true;
}

bool Slot::mayCombine(not_null_ptr<ItemInstance> item)
{
    not_null_ptr<ItemInstance> current = getItem();
    if (!current->isEmpty() && !item->isEmpty())
    {
        if (!current->isEmpty() && !item->isEmpty() && Item::getId(current->getItem()) == Item::getId(item->getItem())
            && current->getCount() == 1 && item->getCount() == 1 && current->getItem()->canBeDepleted())
        {
            if (current->isDamaged() || item->isDamaged())
            {
                return !current->isEnchanted() && !item->isEnchanted();
            }
        }
        return false;
    }
    return false;
}

not_null_ptr<ItemInstance> Slot::combine(not_null_ptr<ItemInstance> item)
{
    not_null_ptr<ItemInstance> slotItem = getItem();
    boost::shared_ptr<CraftingContainer> craftingContainer(new CraftingContainer(NULL, 2, 2));
    craftingContainer->setItem(0, item);
    craftingContainer->setItem(1, slotItem);

    not_null_ptr<ItemInstance> result = ItemInstance::EMPTY;
    if (dynamic_cast<ArmorItem*>(slotItem->getItem()) != NULL)
        result = ArmorDyeRecipe::assembleDyedArmor(craftingContainer);
    else
        result = Recipes::getInstance()->getItemFor(craftingContainer, NULL, NULL, Recipes::eRecipeCategory_Crafting);

    craftingContainer->setItem(0, ItemInstance::EMPTY);
    craftingContainer->setItem(1, ItemInstance::EMPTY);
    return result;
}

void Slot::swap(Slot* other)
{
    not_null_ptr<ItemInstance> current = mContainer->getItem(mSlot);
    not_null_ptr<ItemInstance> incoming = other->mContainer->getItem(other->mSlot);

    if (!current->isEmpty() && current->getCount() > other->getMaxStackSize(current))
    {
        if (!incoming->isEmpty())
        {
            return;
        }
        incoming = current->split(current->getCount() - other->getMaxStackSize(current));
    }

    if (!incoming->isEmpty() && incoming->getCount() > getMaxStackSize(incoming))
    {
        if (!current->isEmpty())
        {
            return;
        }
        current = incoming->split(incoming->getCount() - getMaxStackSize(incoming));
    }

    other->mContainer->setItem(other->mSlot, current);
    mContainer->setItem(mSlot, incoming);
    setChanged();
}
