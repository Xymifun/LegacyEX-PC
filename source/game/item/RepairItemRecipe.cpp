#include "game/item/RepairItemRecipe.hpp"

#include <vector>

#include "game/item/CraftingContainer.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"

RepairItemRecipe::RepairItemRecipe() : Recipe()
{
}

RepairItemRecipe::~RepairItemRecipe()
{
}

bool RepairItemRecipe::matches(boost::shared_ptr<CraftingContainer> craftingContainer, Level* level)
{
    std::vector<not_null_ptr<ItemInstance> > items;
    unsigned int i;
    for (i = 0; i < craftingContainer->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (!item->isEmpty())
        {
            items.push_back(item);
            if (items.size() > 1)
            {
                not_null_ptr<ItemInstance> first = items[0];
                if (item->getItem() != first->getItem() || first->getCount() != 1 || item->getCount() != 1
                    || !first->getItem()->canBeDepleted())
                {
                    return false;
                }
            }
        }
    }
    return items.size() == 2;
}

not_null_ptr<ItemInstance> RepairItemRecipe::assemble(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    std::vector<not_null_ptr<ItemInstance> > items;
    unsigned int i;
    for (i = 0; i < craftingContainer->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (!item->isEmpty())
        {
            items.push_back(item);
            if (items.size() > 1)
            {
                not_null_ptr<ItemInstance> first = items[0];
                if (item->getItem() != first->getItem() || first->getCount() != 1 || item->getCount() != 1
                    || !first->getItem()->canBeDepleted())
                {
                    return ItemInstance::EMPTY;
                }
            }
        }
    }

    if (items.size() == 2)
    {
        not_null_ptr<ItemInstance> first = items[0];
        not_null_ptr<ItemInstance> second = items[1];
        if (first->getItem() == second->getItem() && first->getCount() == 1 && second->getCount() == 1
            && first->getItem()->canBeDepleted())
        {
            Item* item = first->getItem();
            int damage = item->getMaxDamage() - first->getDamageValue();
            damage += item->getMaxDamage() - second->getDamageValue();
            damage += item->getMaxDamage() * 5 / 100;
            int resultDamage = item->getMaxDamage() - damage;
            if (resultDamage < 0)
            {
                resultDamage = 0;
            }
            return not_null_ptr<ItemInstance>(new ItemInstance(first->getItem(), 1, resultDamage));
        }
    }
    return ItemInstance::EMPTY;
}

int RepairItemRecipe::size()
{
    return 4;
}

ItemInstance* RepairItemRecipe::getResultItem()
{
    return ItemInstance::EMPTY.get();
}

ItemInstance* RepairItemRecipe::getIngredientItem(int column, int row)
{
    return ItemInstance::EMPTY.get();
}

NonNullList<not_null_ptr<ItemInstance> > RepairItemRecipe::getRemainingItems(
    boost::shared_ptr<CraftingContainer> craftingContainer)
{
    NonNullList<not_null_ptr<ItemInstance> > result = NonNullList<not_null_ptr<ItemInstance> >::withSize(
        craftingContainer->getContainerSize(), ItemInstance::EMPTY);
    for (unsigned int i = 0; i < result.size(); i++)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (item->getItem()->hasCraftingRemainingItem())
        {
            result.set(i, not_null_ptr<ItemInstance>(new ItemInstance(item->getItem()->getCraftingRemainingItem())));
        }
    }
    return result;
}

Recipe::_eGroupType RepairItemRecipe::getGroup()
{
    return Recipe::eGroupType_None;
}

bool RepairItemRecipe::recipeRequires(int itemId)
{
    return false;
}

void RepairItemRecipe::recipeRequires(INGREDIENTS_REQUIRED* ingredientsRequired)
{
}

boost::shared_ptr<CraftingContainer> RepairItemRecipe::generateExpectedCraftSlots()
{
    return boost::shared_ptr<CraftingContainer>();
}
