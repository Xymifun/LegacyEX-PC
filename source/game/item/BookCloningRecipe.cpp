#include "game/item/BookCloningRecipe.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/WrittenBookItem.hpp"

static const std::wstring TAG_GENERATION = L"generation";

BookCloningRecipe::BookCloningRecipe() : Recipe()
{
}

BookCloningRecipe::~BookCloningRecipe()
{
}

bool BookCloningRecipe::matches(boost::shared_ptr<CraftingContainer> craftingContainer, Level* level)
{
    int writableBooks = 0;
    not_null_ptr<ItemInstance> writtenBook = ItemInstance::EMPTY;

    for (unsigned int i = 0; i < craftingContainer->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (item->isEmpty())
        {
            continue;
        }
        if (item->getItem() == Items::writtenBook)
        {
            if (!writtenBook->isEmpty())
            {
                return false;
            }
            writtenBook = item;
        }
        else if (item->getItem() == Items::writableBook)
        {
            ++writableBooks;
        }
        else
        {
            return false;
        }
    }

    return !writtenBook->isEmpty() && writtenBook->hasTag() && writableBooks > 0;
}

not_null_ptr<ItemInstance> BookCloningRecipe::assemble(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    int writableBooks = 0;
    not_null_ptr<ItemInstance> writtenBook = ItemInstance::EMPTY;

    for (unsigned int i = 0; i < craftingContainer->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (item->isEmpty())
        {
            continue;
        }
        if (item->getItem() == Items::writtenBook)
        {
            if (!writtenBook->isEmpty())
            {
                return ItemInstance::EMPTY;
            }
            writtenBook = item;
        }
        else if (item->getItem() == Items::writableBook)
        {
            ++writableBooks;
        }
        else
        {
            return ItemInstance::EMPTY;
        }
    }

    if (writtenBook->isEmpty() || !writtenBook->hasTag() || writableBooks < 1
        || WrittenBookItem::getGeneration(writtenBook) >= 2)
    {
        return ItemInstance::EMPTY;
    }

    not_null_ptr<ItemInstance> result(new ItemInstance(Items::writtenBook, writableBooks));
    result->setTag(writtenBook->getTag()->copy());
    result->getTag()->putInt(TAG_GENERATION, WrittenBookItem::getGeneration(writtenBook) + 1);
    if (writtenBook->hasCustomHoverName())
    {
        result->setHoverName(writtenBook->getHoverName());
        result->setCensorshipInfo(writtenBook->getOwnerUUID(), writtenBook->wasCreatedByRestrictedPlayer());
    }
    return result;
}

int BookCloningRecipe::size()
{
    return 9;
}

ItemInstance* BookCloningRecipe::getResultItem()
{
    return ItemInstance::EMPTY.get();
}

ItemInstance* BookCloningRecipe::getIngredientItem(int column, int row)
{
    return ItemInstance::EMPTY.get();
}

NonNullList<not_null_ptr<ItemInstance> > BookCloningRecipe::getRemainingItems(
    boost::shared_ptr<CraftingContainer> craftingContainer)
{
    NonNullList<not_null_ptr<ItemInstance> > result = NonNullList<not_null_ptr<ItemInstance> >::withSize(
        craftingContainer->getContainerSize(), ItemInstance::EMPTY);
    for (unsigned int i = 0; i < result.size(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (!item->isEmpty() && dynamic_cast<WrittenBookItem*>(item->getItem()) != NULL)
        {
            not_null_ptr<ItemInstance> copy = item->copy();
            copy->setCount(1);
            result[i] = copy;
            break;
        }
    }
    return result;
}

Recipe::_eGroupType BookCloningRecipe::getGroup()
{
    return Recipe::eGroupType_None;
}

bool BookCloningRecipe::recipeRequires(int itemId)
{
    return false;
}

void BookCloningRecipe::recipeRequires(INGREDIENTS_REQUIRED* ingredientsRequired)
{
}

boost::shared_ptr<CraftingContainer> BookCloningRecipe::generateExpectedCraftSlots()
{
    return boost::shared_ptr<CraftingContainer>();
}
