#include "game/item/ShapelessRecipe.hpp"

#include <cstring>

#include "game/item/CraftingContainer.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"

ShapelessRecipe::ShapelessRecipe(ItemInstance* result, std::vector<ItemInstance*>* ingredients,
                                 Recipe::_eGroupType group)
    : Recipe(),
      mGroup(group),
      mResultItem(result),
      mIngredients(ingredients)
{
}

ShapelessRecipe::~ShapelessRecipe()
{
}

ItemInstance* ShapelessRecipe::getResultItem()
{
    return mResultItem;
}

ItemInstance* ShapelessRecipe::getIngredientItem(int column, int row)
{
    return ItemInstance::EMPTY.get();
}

NonNullList<not_null_ptr<ItemInstance> > ShapelessRecipe::getRemainingItems(
    boost::shared_ptr<CraftingContainer> craftingContainer)
{
    NonNullList<not_null_ptr<ItemInstance> > result = NonNullList<not_null_ptr<ItemInstance> >::withSize(
        craftingContainer->getContainerSize(), ItemInstance::EMPTY);
    for (unsigned int i = 0; i < result.size(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (item->getItem()->hasCraftingRemainingItem())
        {
            result.set(i, not_null_ptr<ItemInstance>(new ItemInstance(item->getItem()->getCraftingRemainingItem())));
        }
    }
    return result;
}

Recipe::_eGroupType ShapelessRecipe::getGroup()
{
    return mGroup;
}

bool ShapelessRecipe::matches(boost::shared_ptr<CraftingContainer> craftingContainer, Level* level)
{
    std::vector<ItemInstance*> remaining = *mIngredients;
    for (int row = 0; row < craftingContainer->getHeight(); ++row)
    {
        for (int column = 0; column < craftingContainer->getWidth(); ++column)
        {
            not_null_ptr<ItemInstance> item = craftingContainer->getItem(column, row);
            if (!item->isEmpty())
            {
                bool found = false;
                std::vector<ItemInstance*>::iterator end = remaining.end();
                for (std::vector<ItemInstance*>::iterator it = remaining.begin(); it != end; ++it)
                {
                    ItemInstance* ingredient = *it;
                    if (item->getItem() == ingredient->getItem()
                        && (ingredient->getAuxValue() == -1 || item->getAuxValue() == ingredient->getAuxValue()
                            || (ingredient->getAuxValue() == 0 && ingredient->isDamageableItem())))
                    {
                        found = true;
                        remaining.erase(it);
                        break;
                    }
                }
                if (!found)
                {
                    return false;
                }
            }
        }
    }
    return remaining.empty();
}

not_null_ptr<ItemInstance> ShapelessRecipe::assemble(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    return mResultItem->copy();
}

int ShapelessRecipe::size()
{
    return static_cast<int>(mIngredients->size());
}

bool ShapelessRecipe::recipeRequires(int itemId)
{
    std::vector<ItemInstance*>* ingredients = new std::vector<ItemInstance*>;
    *ingredients = *mIngredients;
    std::vector<ItemInstance*>::iterator last = mIngredients->end();
    for (std::vector<ItemInstance*>::iterator ingredient = mIngredients->begin(); ingredient != last; ++ingredient)
    {
        ingredients->erase(ingredient);
    }
    delete ingredients;
    return false;
}

void ShapelessRecipe::recipeRequires(INGREDIENTS_REQUIRED* ingredientsRequired)
{
    int count = 0;
    int gridSlot = 0;
    int requiresCraftingTable = mIngredients->size() > 4;
    int* craftingGrid = new int[9];
    int* itemIds = new int[9];
    int* itemCounts = new int[9];
    int* auxValues = new int[9];
    CompoundTag** itemTags = new CompoundTag*[9];
    std::memset(itemIds, 0, sizeof(int) * 9);
    std::memset(itemCounts, 0, sizeof(int) * 9);
    std::memset(auxValues, -1, sizeof(int) * 9);
    std::memset(itemTags, 0, sizeof(CompoundTag*) * 9);
    std::memset(craftingGrid, 0, sizeof(int) * 9);

    bool compactRecipe = mIngredients->size() <= 4;
    std::vector<ItemInstance*>::iterator last = mIngredients->end();
    for (std::vector<ItemInstance*>::iterator it = mIngredients->begin(); it != last; ++it)
    {
        ItemInstance* ingredient = *it;
        if (ingredient != NULL && !ingredient->isEmpty())
        {
            int aux = (*it)->getAuxValue();
            craftingGrid[gridSlot++] = Item::getId(ingredient->getItem()) | (aux << 24);
            if (compactRecipe && gridSlot == 2)
            {
                gridSlot = 3;
            }

            int j;
            for (j = 0; j < count; ++j)
            {
                if (itemIds[j] == Item::getId(ingredient->getItem()) && (aux == -1 || auxValues[j] == aux))
                {
                    break;
                }
            }
            if (j < count)
            {
                ++itemCounts[j];
            }
            else
            {
                itemIds[count] = Item::getId(ingredient->getItem());
                auxValues[count] = aux;
                itemTags[count] = ingredient->getTag();
                ++itemCounts[count++];
            }
        }
    }

    ingredientsRequired->itemIds = new int[count];
    ingredientsRequired->itemCounts = new int[count];
    ingredientsRequired->auxValues = new int[count];
    ingredientsRequired->itemTags = new CompoundTag*[count];
    ingredientsRequired->craftingGrid = new int[9];
    ingredientsRequired->available[0] = false;
    ingredientsRequired->recipe = this;
    ingredientsRequired->available[1] = false;
    ingredientsRequired->available[2] = false;
    ingredientsRequired->available[3] = false;
    ingredientsRequired->numIngredients = count;
    ingredientsRequired->requiresCraftingTable = requiresCraftingTable;
    if (count != 0)
    {
        std::memcpy(ingredientsRequired->itemIds, itemIds, sizeof(int) * count);
        std::memcpy(ingredientsRequired->itemCounts, itemCounts, sizeof(int) * count);
        std::memcpy(ingredientsRequired->auxValues, auxValues, sizeof(int) * count);
        std::memcpy(ingredientsRequired->itemTags, itemTags, sizeof(CompoundTag*) * count);
    }
    std::memcpy(ingredientsRequired->craftingGrid, craftingGrid, sizeof(int) * 9);

    delete[] itemIds;
    delete[] itemCounts;
    delete[] auxValues;
    delete[] itemTags;
    delete[] craftingGrid;
}

boost::shared_ptr<CraftingContainer> ShapelessRecipe::generateExpectedCraftSlots()
{
    boost::shared_ptr<CraftingContainer> container(new CraftingContainer(NULL, 3, 3));
    unsigned int slot = 0;
    for (std::vector<ItemInstance*>::const_iterator ingredient = mIngredients->begin();
         ingredient != mIngredients->end(); ++ingredient)
    {
        ItemInstance* item = *ingredient;
        if (item != NULL && !item->isEmpty())
        {
            container->setItem(slot, item->copy());
            ++slot;
        }
    }
    return container;
}
