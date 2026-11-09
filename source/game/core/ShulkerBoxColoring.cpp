#include "game/core/ShulkerBoxColoring.hpp"

#include "game/core/DyeColor.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/ShulkerBoxBlock.hpp"

ShulkerBoxColoring::~ShulkerBoxColoring()
{
}

bool ShulkerBoxColoring::matches(boost::shared_ptr<CraftingContainer> craftingContainer, Level* level)
{
    int shulkerBoxCount = 0;
    int dyeCount = 0;
    unsigned int slot;

    for (slot = 0; slot < craftingContainer->getContainerSize(); ++slot)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(slot);
        if (item->isEmpty())
        {
            continue;
        }

        if (dynamic_cast<ShulkerBoxBlock*>(Block::byItem(item->getItem())))
        {
            ++shulkerBoxCount;
        }
        else if (item->getItem() == Items::dye)
        {
            ++dyeCount;
        }
        else
        {
            return false;
        }

        if (dyeCount > 1 || shulkerBoxCount > 1)
        {
            return false;
        }
    }

    return shulkerBoxCount == 1 && dyeCount == 1;
}

not_null_ptr<ItemInstance> ShulkerBoxColoring::assemble(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    not_null_ptr<ItemInstance> shulkerBox = ItemInstance::EMPTY;
    not_null_ptr<ItemInstance> dye = ItemInstance::EMPTY;
    unsigned int slot;

    for (slot = 0; slot < craftingContainer->getContainerSize(); ++slot)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(slot);
        if (item->isEmpty())
        {
            continue;
        }
        if (dynamic_cast<ShulkerBoxBlock*>(Block::byItem(item->getItem())))
        {
            shulkerBox = item;
        }
        else if (item->getItem() == Items::dye)
        {
            dye = item;
        }
    }

    not_null_ptr<ItemInstance> result = ShulkerBoxBlock::getColoredItemInstance(
        DyeColor::byItemData(dye->getAuxValue()));
    if (shulkerBox->hasTag())
    {
        result->setTag(shulkerBox->getTag()->copy());
    }
    return result;
}

int ShulkerBoxColoring::size()
{
    return 10;
}

ItemInstance* ShulkerBoxColoring::getResultItem()
{
    return ItemInstance::EMPTY.get();
}

ItemInstance* ShulkerBoxColoring::getIngredientItem(int column, int row)
{
    return ItemInstance::EMPTY.get();
}

NonNullList<not_null_ptr<ItemInstance> > ShulkerBoxColoring::getRemainingItems(
    boost::shared_ptr<CraftingContainer> craftingContainer)
{
    NonNullList<not_null_ptr<ItemInstance> > result = NonNullList<not_null_ptr<ItemInstance> >::withSize(
        craftingContainer->getContainerSize(), ItemInstance::EMPTY);
    unsigned int slot;
    for (slot = 0; slot < result.size(); ++slot)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(slot);
        if (item->getItem()->hasCraftingRemainingItem())
        {
            result.set(slot, new ItemInstance(item->getItem()->getCraftingRemainingItem()));
        }
    }
    return result;
}

Recipe::_eGroupType ShulkerBoxColoring::getGroup()
{
    return Recipe::eGroupType_None;
}

bool ShulkerBoxColoring::recipeRequires(int itemId)
{
    return false;
}

void ShulkerBoxColoring::recipeRequires(INGREDIENTS_REQUIRED* ingredientsRequired)
{
}

boost::shared_ptr<CraftingContainer> ShulkerBoxColoring::generateExpectedCraftSlots()
{
    return boost::shared_ptr<CraftingContainer>();
}
