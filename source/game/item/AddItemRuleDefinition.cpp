#include "game/item/AddItemRuleDefinition.hpp"

#include <boost/pointer_cast.hpp>

#include "game/item/AddEnchantmentRuleDefinition.hpp"
#include "game/item/Container.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"

AddItemRuleDefinition::AddItemRuleDefinition() : GameRuleDefinition()
{
    m4JData = 0;
    mData = 0;
    mItemId = 0;
    mCount = 1;
    mSlot = -1;
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_itemId, &mItemId);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_quantity, &mCount);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_auxValue, &mData);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_dataTag, &m4JData);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_slot, &mSlot);
}

not_null_ptr<ItemInstance> AddItemRuleDefinition::getItem()
{
    not_null_ptr<ItemInstance> itemInstance = ItemInstance::EMPTY;
    if (Item::byId(mItemId) != NULL)
    {
        int maxStackSize = Item::byId(mItemId)->getMaxStackSize();
        int count = mCount;
        if (maxStackSize < count)
        {
            count = maxStackSize;
        }
        itemInstance = not_null_ptr<ItemInstance>(new ItemInstance(Item::byId(mItemId), count, mData));
        itemInstance->set4JData(m4JData);
        std::vector<AddEnchantmentRuleDefinition*>::iterator it;
        for (it = mEnchantments.begin(); it != mEnchantments.end(); ++it)
        {
            (*it)->enchantItem(itemInstance);
        }
    }
    return itemInstance;
}

bool AddItemRuleDefinition::addItemToContainer(boost::shared_ptr<Container> container, not_null_ptr<ItemInstance> item,
                                               int slot, bool randomSlot)
{
    Random random;
    bool added = false;
    if (slot >= 0 && static_cast<unsigned int>(slot) < container->getContainerSize())
    {
        container->setItem(slot, item);
        added = true;
    }
    else if (boost::dynamic_pointer_cast<Inventory>(container))
    {
        added = boost::dynamic_pointer_cast<Inventory>(container)->add(item);
    }
    else if (randomSlot)
    {
        if (!container->isFull())
        {
            unsigned int index;
            do
            {
                index = static_cast<unsigned int>(random.nextInt(static_cast<int>(container->getContainerSize())));
            } while (!container->getItem(index)->isEmpty());
            container->setItem(index, item);
        }
        added = false;
    }
    else
    {
        unsigned int index;
        for (index = 0; index < container->getContainerSize(); ++index)
        {
            bool empty = container->getItem(index)->isEmpty();
            if (empty)
            {
                container->setItem(index, item);
                added = true;
                break;
            }
        }
    }
    return added;
}

bool AddItemRuleDefinition::addItemToContainer(boost::shared_ptr<Container> container, int slot)
{
    bool added = false;
    not_null_ptr<ItemInstance> item = getItem();
    if (!item->isEmpty())
    {
        int targetSlot = slot;
        if (mSlot >= 0 && static_cast<unsigned int>(mSlot) < container->getContainerSize())
        {
            targetSlot = mSlot;
        }
        added = addItemToContainer(container, item, targetSlot, false);
    }
    return added;
}

void AddItemRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children)
{
    std::vector<AddEnchantmentRuleDefinition*>::iterator it;
    for (it = mEnchantments.begin(); it != mEnchantments.end(); ++it)
    {
        children->push_back(*it);
    }
}

ConsoleGameRules::EGameRuleType AddItemRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_AddItem;
}

GameRuleDefinition* AddItemRuleDefinition::addChild(ConsoleGameRules::EGameRuleType type)
{
    GameRuleDefinition* rule = NULL;
    if (type == ConsoleGameRules::eGameRuleType_AddEnchantment)
    {
        rule = new AddEnchantmentRuleDefinition();
        mEnchantments.push_back(static_cast<AddEnchantmentRuleDefinition*>(rule));
    }
    return rule;
}
