#include "game/item/ShapedRecipe.hpp"

#include <cstring>

#include "game/data/CompoundTag.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"

ShapedRecipe::ShapedRecipe(int width, int height, ItemInstance** ingredients, ItemInstance* result, int group)
    : mWidth(width),
      mHeight(height),
      mIngredients(ingredients)
{
    for (int i = 0; i < width * height; i++)
    {
        if (mIngredients[i] == NULL)
        {
            mIngredients[i] = ItemInstance::EMPTY.get();
        }
    }
    mResult = result;
    mGroup = static_cast<_eGroupType>(group);
    mKeepTag = false;
}

ItemInstance* ShapedRecipe::getResultItem()
{
    return mResult;
}

ItemInstance* ShapedRecipe::getIngredientItem(int column, int row)
{
    return mIngredients[column + row * mWidth];
}

NonNullList<not_null_ptr<ItemInstance> > ShapedRecipe::getRemainingItems(
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

Recipe::_eGroupType ShapedRecipe::getGroup()
{
    return mGroup;
}

bool ShapedRecipe::matches(boost::shared_ptr<CraftingContainer> craftingContainer, int column, int row, bool mirror)
{
    int gridColumn;
    int gridRow;
    for (gridColumn = 0; gridColumn < 3; ++gridColumn)
    {
        for (gridRow = 0; gridRow < 3; ++gridRow)
        {
            int recipeColumn = gridColumn - column;
            int recipeRow = gridRow - row;
            ItemInstance* ingredient = ItemInstance::EMPTY.get();
            if (recipeColumn >= 0 && recipeRow >= 0 && recipeColumn < mWidth && recipeRow < mHeight)
            {
                if (mirror)
                {
                    ingredient = mIngredients[mWidth - recipeColumn - 1 + recipeRow * mWidth];
                }
                else
                {
                    ingredient = mIngredients[recipeColumn + recipeRow * mWidth];
                }
            }

            not_null_ptr<ItemInstance> item = craftingContainer->getItem(gridColumn, gridRow);
            if (item->isEmpty() && ingredient->isEmpty())
            {
                continue;
            }
            if (item->isEmpty() != ingredient->isEmpty())
            {
                return false;
            }
            if (ingredient->getItem() != item->getItem())
            {
                return false;
            }
            if (ingredient->getAuxValue() != -1 && ingredient->getAuxValue() != item->getAuxValue()
                && (ingredient->getAuxValue() != 0 || !item->isDamageableItem()))
            {
                return false;
            }
        }
    }
    return true;
}

bool ShapedRecipe::matches(boost::shared_ptr<CraftingContainer> craftingContainer, Level* level)
{
    int column;
    int row;
    for (column = 0; column <= 3 - mWidth; ++column)
    {
        for (row = 0; row <= 3 - mHeight; ++row)
        {
            if (matches(craftingContainer, column, row, true))
            {
                return true;
            }
            if (matches(craftingContainer, column, row, false))
            {
                return true;
            }
        }
    }
    return false;
}

not_null_ptr<ItemInstance> ShapedRecipe::assemble(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    not_null_ptr<ItemInstance> result = getResultItem()->copy();
    if (mKeepTag && craftingContainer)
    {
        unsigned int i;
        for (i = 0; i < craftingContainer->getContainerSize(); ++i)
        {
            not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
            if (!item->isEmpty() && item->hasTag())
            {
                result->setTag(item->getTag()->copy());
            }
        }
    }
    return result;
}

int ShapedRecipe::size()
{
    return mWidth * mHeight;
}

bool ShapedRecipe::recipeRequires(int itemId)
{
    for (int column = 0; column < 3; column++)
    {
        for (int row = 0; row < 3; row++)
        {
            if (column < mWidth && row < mHeight)
            {
                ItemInstance* expected = mIngredients[column + row * mWidth];
                expected->isEmpty();
            }
        }
    }
    return false;
}

void ShapedRecipe::recipeRequires(INGREDIENTS_REQUIRED* ingredientsRequired)
{
    int requiresCraftingTable = mWidth > 2 || mHeight > 2;
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

    int count = 0;
    int column;
    int row;
    for (column = 0; column < 3; ++column)
    {
        for (row = 0; row < 3; ++row)
        {
            if (column < mWidth && row < mHeight)
            {
                ItemInstance* ingredient = mIngredients[column + row * mWidth];
                if (!ingredient->isEmpty())
                {
                    int aux = ingredient->getAuxValue();
                    craftingGrid[column + row * 3] = Item::VALIDgetId(ingredient->getItem()) | (aux << 24);

                    int j;
                    for (j = 0; j < count; ++j)
                    {
                        if (itemIds[j] == Item::VALIDgetId(ingredient->getItem()) && (aux == -1 || auxValues[j] == aux))
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
                        itemIds[count] = Item::VALIDgetId(ingredient->getItem());
                        auxValues[count] = aux;
                        itemTags[count] = ingredient->getTag();
                        ++itemCounts[count];
                        ++count;
                    }
                }
            }
        }
    }

    ingredientsRequired->itemIds = new int[count];
    ingredientsRequired->itemCounts = new int[count];
    ingredientsRequired->auxValues = new int[count];
    ingredientsRequired->itemTags = new CompoundTag*[count];
    int* grid = new int[9];
    int i;
    for (i = 0; i < 9; ++i)
    {
        grid[i] = craftingGrid[i];
    }
    ingredientsRequired->craftingGrid = grid;
    ingredientsRequired->numIngredients = count;
    ingredientsRequired->requiresCraftingTable = requiresCraftingTable;
    ingredientsRequired->available[0] = false;
    ingredientsRequired->available[1] = false;
    ingredientsRequired->available[2] = false;
    ingredientsRequired->available[3] = false;
    ingredientsRequired->recipe = this;
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

boost::shared_ptr<CraftingContainer> ShapedRecipe::generateExpectedCraftSlots()
{
    boost::shared_ptr<CraftingContainer> result(new CraftingContainer(NULL, 3, 3));
    for (int column = 0; column < 3; column++)
    {
        for (int row = 0; row < 3; row++)
        {
            if (column < mWidth && row < mHeight)
            {
                ItemInstance* ingredient = mIngredients[column + row * 3];
                if (!ingredient->isEmpty())
                {
                    result->setItem(column + row * 3, ingredient->copy());
                }
            }
        }
    }
    return result;
}
